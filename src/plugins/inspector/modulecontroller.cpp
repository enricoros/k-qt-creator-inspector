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

#include "modulecontroller.h"
#include "abstractmodule.h"
#include "abstractview.h"
#include "tasksmodel.h"

using namespace Inspector::Internal;

ModuleController::ModuleController(TasksModel *tasksModel, QObject *parent)
  : QObject(parent)
  , m_tasksModel(tasksModel)
{
    connect(m_tasksModel, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(slotModelItemChanged(QStandardItem*)));
}

ModuleController::~ModuleController()
{
    // delete all the modules (bypassing the 'destroyed' hook)
    QList<AbstractModule *> listCopy = m_modules;
    m_modules.clear();
    qDeleteAll(listCopy);
}

void ModuleController::addModule(AbstractModule * module)
{
    if (!module) {
        qWarning("ModuleController::addModule: skipping null module");
        return;
    }
    // check for duplicate Ids
    foreach (AbstractModule *mod, m_modules) {
        if (mod->uid() == module->uid()) {
            qWarning("ModuleController::addModule: skipping module with duplicated Uid %d", module->uid());
            return;
        }
    }
    // register the AbstractModule
    connect(module, SIGNAL(requestActivation(QString)), this, SLOT(slotModuleActivationRequested(QString)));
    connect(module, SIGNAL(deactivated()), this, SLOT(slotModuleDeactivated()));
    connect(module, SIGNAL(destroyed()), this, SLOT(slotModuleDestroyed()));
    m_modules.append(module);
    emit modulesChanged();
}

void ModuleController::removeModule(AbstractModule * module)
{
    if (!module) {
        qWarning("ModuleController::removeModule: skipping null module");
        return;
    }
    // unregister the AbstractModule
    disconnect(module, 0, this, 0);
    m_modules.removeAll(module);
    m_activeModules.removeAll(module);
    emit modulesChanged();
}

ModuleMenuEntries ModuleController::menuEntries() const
{
    ModuleMenuEntries entries;
    foreach (AbstractModule *module, m_modules)
        entries.append(module->menuEntries());
    return entries;
}

QStringList ModuleController::moduleNames() const
{
    QStringList names;
    foreach (AbstractModule *module, m_modules)
        names.append(module->name());
    return names;
}

AbstractView *ModuleController::createView(int moduleUid, int viewId) const
{
    AbstractModule * module = moduleForUid(moduleUid);
    if (!module) {
        qWarning("ModuleController::createView: unknown module Uid %d", moduleUid);
        return 0;
    }
    return module->createView(viewId);
}

AbstractModule *ModuleController::moduleForUid(int moduleUid) const
{
    foreach (AbstractModule *module, m_modules)
        if (module->uid() == moduleUid)
            return module;
    return 0;
}

void ModuleController::slotModuleActivationRequested(const QString &text)
{
    AbstractModule * module = static_cast<AbstractModule *>(sender());

    // update the model
    QString name = text.isEmpty() ? module->name() : text;
    if (!m_tasksModel->addTask(module->uid(), name, "provide description here")) {
        qWarning("ModuleController::slotModuleActivationRequested: can't add module %d", module->uid());
        return;
    }
    if (!m_tasksModel->startTask(module->uid())) {
        qWarning("ModuleController::slotModuleActivationRequested: can't start module %d", module->uid());
        return;
    }

    // activate module
    module->controlActivate();

    // CHANGE THIS? superseed by the model?
    if (!m_activeModules.contains(module))
        m_activeModules.append(module);
}

void ModuleController::slotModuleDeactivated()
{
    AbstractModule * module = static_cast<AbstractModule *>(sender());

    // update the model
    if (!m_tasksModel->stopTask(module->uid())) {
        qWarning("ModuleController::slotModuleDeactivated: can't stop module %d", module->uid());
        return;
    }

    // CHANGE THIS? superseed by the model?
    m_activeModules.removeAll(module);
}

void ModuleController::slotModuleDestroyed()
{
    AbstractModule * module = static_cast<AbstractModule *>(sender());
    // CHANGE THIS? superseed by the model?
    if (m_modules.contains(module)) {
        m_tasksModel->stopTask(module->uid());
        removeModule(module);
    }
}

void ModuleController::slotModelItemChanged(QStandardItem *item)
{
    // get the TaskItem
    TaskItem *taskItem = dynamic_cast<TaskItem *>(item);
    if (!taskItem)
        return;

    // check if the RequestStop flag is set
    if (taskItem->requestStop()) {
        // FIXME THIS: equivalence TaskId <-> ModuleId implied here
        AbstractModule * module = moduleForUid(taskItem->tid());
        if (module)
            module->controlDeactivate();
        return;
    }
}
