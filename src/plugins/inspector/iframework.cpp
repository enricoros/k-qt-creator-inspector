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

#include "iframework.h"
#include "iframeworktask.h"
#include "abstractpanel.h"
#include "iinspectionmodel.h"
#include "tasksmodel.h"

#include <utils/qtcassert.h>

using namespace Inspector::Internal;

IFramework::IFramework(IInspectionModel *inspectionModel, QObject *parent)
  : QObject(parent)
  , m_inspectionModel(inspectionModel)
  , m_taskActivationEnabled(true)
{
    m_tasksModel = new TasksModel(this);
    connect(m_tasksModel, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(slotTaskModelItemChanged(QStandardItem*)));
}

IFramework::~IFramework()
{
    // delete add the FrameworkTasks
    while (!m_tasks.isEmpty()) {
        IFrameworkTask *task = m_tasks.first();
        unregisterTask(task);
        delete task;
    }

    // delete all the FrameworkModules (bypassing the 'destroyed' hook)
    while (!m_modules.isEmpty()) {
        IFrameworkModule *module = m_modules.first();
        takeModule(module);
        delete module;
    }

    // delete the models
    delete m_tasksModel;
    delete m_inspectionModel;
}

IInspectionModel *IFramework::inspectionModel() const
{
    return m_inspectionModel;
}

TasksModel *IFramework::tasksModel() const
{
    return m_tasksModel;
}

QStringList IFramework::loadedModules() const
{
    QStringList names;
    foreach (IFrameworkModule *module, m_modules)
        names.append(module->name());
    return names;
}

ModuleMenuEntries IFramework::menuItems() const
{
    ModuleMenuEntries entries;
    foreach (IFrameworkModule *module, m_modules)
        entries.append(module->menuEntries());
    return entries;
}

AbstractPanel *IFramework::createPanel(int moduleUid, int panelId) const
{
    IFrameworkModule * module = moduleForUid(moduleUid);
    if (!module) {
        qWarning("IFramework::createPanel: unknown module Uid %d", moduleUid);
        return 0;
    }
    return module->createPanel(panelId);
}


// Modules

void IFramework::addModule(IFrameworkModule *module)
{
    QTC_ASSERT(module, return);

    // check for duplicate Ids
    foreach (IFrameworkModule *mod, m_modules) {
        if (mod->uid() == module->uid()) {
            qWarning("IFramework::addModule: skipping module with duplicated Uid %d", module->uid());
            return;
        }
    }

    // register the IFrameworkModule
    connect(module, SIGNAL(requestPanelDisplay(int)),
            this, SLOT(slotModulePanelDisplayRequested(int)));
    connect(module, SIGNAL(destroyed()),
            this, SLOT(slotModuleDestroyed()));
    m_modules.append(module);
    emit moduleListChanged();
}

void IFramework::takeModule(IFrameworkModule *module)
{
    QTC_ASSERT(module, return);

    // unregister the IFrameworkModule
    disconnect(module, 0, this, 0);
    m_modules.removeAll(module);
    emit moduleListChanged();
}

IFrameworkModule *IFramework::moduleForUid(int moduleUid) const
{
    foreach (IFrameworkModule *module, m_modules)
        if (module->uid() == moduleUid)
            return module;
    return 0;
}

void IFramework::slotModulePanelDisplayRequested(int panelId)
{
    int moduleUid = static_cast<IFrameworkModule *>(sender())->uid();
    emit requestPanelDisplay(moduleUid, panelId);
}

void IFramework::slotModuleDestroyed()
{
    QTC_ASSERT(false, /*shouldn't delete the module without removing it first*/);
    takeModule(static_cast<IFrameworkModule *>(sender()));
}


// Tasks

void IFramework::registerTask(IFrameworkTask *task)
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

void IFramework::unregisterTask(IFrameworkTask *task)
{
    QTC_ASSERT(task, return);

    disconnect(task, 0, this, 0);
    m_tasks.removeAll(task);
}

void IFramework::setTaskActivationEnabled(bool enabled)
{
    m_taskActivationEnabled = enabled;

    // if disabled, quit active tasks
    if (!m_taskActivationEnabled) {
        foreach (IFrameworkTask *task, m_tasks)
            task->controlDeactivate();
    }
}

void IFramework::slotTaskActivationRequested()
{
    IFrameworkTask *task = qobject_cast<IFrameworkTask *>(sender());
    QTC_ASSERT(task, return);

    if (!m_taskActivationEnabled) {
        task->controlRefuse();
        return;
    }

    // TasksModel: create entry and start it
    quint32 tId = task->taskUid();
    QString taskName = tr("%1 (%2)").arg(task->displayName()).arg(tId);
    if (!m_tasksModel->addTask(tId, taskName)) {
        qWarning("IFramework::slotTaskActivationRequested: can't create the task %d", tId);
        return;
    }
    if (!m_tasksModel->startTask(tId)) {
        qWarning("IFramework::slotTaskActivationRequested: can't start the task %d", tId);
        return;
    }

    // activate Task
    task->controlActivate();
}

void IFramework::slotTaskDeletionRequested()
{
    IFrameworkTask *task = qobject_cast<IFrameworkTask *>(sender());
    QTC_ASSERT(task, return);

    // TasksModel: finish the task
    if (!m_tasksModel->stopTask(task->taskUid()))
        qWarning("IFramework::slotTaskDeletionRequested: can't stop task %d", task->taskUid());

    // delete the task (deregistration will happen right away)
    task->deleteLater();
}

void IFramework::slotTaskSetProgress(int percent)
{
    if (IFrameworkTask *task = qobject_cast<IFrameworkTask *>(sender()))
        m_tasksModel->setTaskProgress(task->taskUid(), percent);
}

void IFramework::slotTaskModelItemChanged(QStandardItem *item)
{
    // get the TaskItem
    TaskItem *taskItem = dynamic_cast<TaskItem *>(item);
    if (!taskItem)
        return;

    // monitor for RequestStop
    if (taskItem->requestStop()) {
        quint32 tId = taskItem->tid();
        foreach (IFrameworkTask *task, m_tasks)
            if (task->taskUid() == tId)
                task->controlDeactivate();
    }
}
