/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009-2010 Enrico Ros
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include "ibackend.h"
#include "ibackendtask.h"
#include "abstractpanel.h"
#include "iinspectionmodel.h"
#include "tasksmodel.h"

#include <utils/qtcassert.h>

using namespace Inspector::Internal;

IBackend::IBackend(IInspectionModel *inspectionModel, QObject *parent)
  : QObject(parent)
  , m_inspectionModel(inspectionModel)
  , m_taskActivationEnabled(true)
{
    m_tasksModel = new TasksModel(this);
    connect(m_tasksModel, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(slotTaskModelItemChanged(QStandardItem*)));
}

IBackend::~IBackend()
{
    // delete add the BackendTasks
    while (!m_tasks.isEmpty()) {
        IBackendTask *task = m_tasks.first();
        unregisterTask(task);
        delete task;
    }

    // delete all the BackendModules (bypassing the 'destroyed' hook)
    while (!m_modules.isEmpty()) {
        IBackendModule *module = m_modules.first();
        takeModule(module);
        delete module;
    }

    // delete the models
    delete m_tasksModel;
    delete m_inspectionModel;
}

IInspectionModel *IBackend::inspectionModel() const
{
    return m_inspectionModel;
}

TasksModel *IBackend::tasksModel() const
{
    return m_tasksModel;
}

QStringList IBackend::loadedModules() const
{
    QStringList names;
    foreach (IBackendModule *module, m_modules)
        names.append(module->name());
    return names;
}

ModuleMenuEntries IBackend::menuItems() const
{
    ModuleMenuEntries entries;
    foreach (IBackendModule *module, m_modules)
        entries.append(module->menuEntries());
    return entries;
}

AbstractPanel *IBackend::createPanel(int moduleUid, int panelId) const
{
    IBackendModule * module = moduleForUid(moduleUid);
    if (!module) {
        qWarning("IBackend::createPanel: unknown module Uid %d", moduleUid);
        return 0;
    }
    return module->createPanel(panelId);
}


// Modules

void IBackend::addModule(IBackendModule *module)
{
    QTC_ASSERT(module, return);

    // check for duplicate Ids
    foreach (IBackendModule *mod, m_modules) {
        if (mod->uid() == module->uid()) {
            qWarning("IBackend::addModule: skipping module with duplicated Uid %d", module->uid());
            return;
        }
    }

    // register the IBackendModule
    connect(module, SIGNAL(requestPanelDisplay(int)),
            this, SLOT(slotModulePanelDisplayRequested(int)));
    connect(module, SIGNAL(destroyed()),
            this, SLOT(slotModuleDestroyed()));
    m_modules.append(module);
    emit moduleListChanged();
}

void IBackend::takeModule(IBackendModule *module)
{
    QTC_ASSERT(module, return);

    // unregister the IBackendModule
    disconnect(module, 0, this, 0);
    m_modules.removeAll(module);
    emit moduleListChanged();
}

IBackendModule *IBackend::moduleForUid(int moduleUid) const
{
    foreach (IBackendModule *module, m_modules)
        if (module->uid() == moduleUid)
            return module;
    return 0;
}

void IBackend::slotModulePanelDisplayRequested(int panelId)
{
    int moduleUid = static_cast<IBackendModule *>(sender())->uid();
    emit requestPanelDisplay(moduleUid, panelId);
}

void IBackend::slotModuleDestroyed()
{
    QTC_ASSERT(false, /*shouldn't delete the module without removing it first*/);
    takeModule(static_cast<IBackendModule *>(sender()));
}


// Tasks

void IBackend::registerTask(IBackendTask *task)
{
    if (m_tasks.contains(task))
        return;

    m_tasks.append(task);
    connect(task, SIGNAL(requestActivation()),
            this, SLOT(slotTaskActivationRequested()),
            Qt::QueuedConnection);
    connect(task, SIGNAL(finished()),
            this, SLOT(slotTaskDeletionRequested()),
            Qt::QueuedConnection);
    connect(task, SIGNAL(setProgress(int)),
            this, SLOT(slotTaskSetProgress(int)),
            Qt::DirectConnection);
}

void IBackend::unregisterTask(IBackendTask *task)
{
    QTC_ASSERT(task, return);

    disconnect(task, 0, this, 0);
    m_tasks.removeAll(task);
}

void IBackend::setTaskActivationEnabled(bool enabled)
{
    m_taskActivationEnabled = enabled;

    // if disabled, quit active tasks
    if (!m_taskActivationEnabled) {
        foreach (IBackendTask *task, m_tasks)
            task->controlDeactivate();
    }
}

void IBackend::slotTaskActivationRequested()
{
    IBackendTask *task = qobject_cast<IBackendTask *>(sender());
    QTC_ASSERT(task, return);

    if (!m_taskActivationEnabled) {
        task->controlRefuse();
        return;
    }

    quint32 taskId = task->taskUid();
    QString taskName = tr("%1 (%2)").arg(task->displayName()).arg(taskId);
    QColor taskColor = QColor::fromHsv(60 * (qrand() % 6), 255, 200);

    // create entry in the model
    if (!m_tasksModel->addTask(taskId, taskName, taskColor)) {
        qWarning("IBackend::slotTaskActivationRequested: can't create the task %d", taskId);
        return;
    }

    // mark as started
    if (!m_tasksModel->startTask(taskId)) {
        qWarning("IBackend::slotTaskActivationRequested: can't start the task %d", taskId);
        return;
    }

    // activate Task
    task->controlActivate();
}

void IBackend::slotTaskDeletionRequested()
{
    IBackendTask *task = qobject_cast<IBackendTask *>(sender());
    QTC_ASSERT(task, return);

    // TasksModel: finish the task
    if (!m_tasksModel->stopTask(task->taskUid()))
        qWarning("IBackend::slotTaskDeletionRequested: can't stop task %d", task->taskUid());

    // delete the task (deregistration will happen right away)
    task->deleteLater();
}

void IBackend::slotTaskSetProgress(int percent)
{
    if (IBackendTask *task = qobject_cast<IBackendTask *>(sender()))
        m_tasksModel->setTaskProgress(task->taskUid(), percent);
}

void IBackend::slotTaskModelItemChanged(QStandardItem *item)
{
    // get the TaskItem
    TaskItem *taskItem = dynamic_cast<TaskItem *>(item);
    if (!taskItem)
        return;

    // monitor for RequestStop
    if (taskItem->requestStop()) {
        quint32 tId = taskItem->tid();
        foreach (IBackendTask *task, m_tasks)
            if (task->taskUid() == tId)
                task->controlDeactivate();
    }
}
