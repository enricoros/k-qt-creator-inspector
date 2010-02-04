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

#include "iinspectorframework.h"
#include "abstractpanel.h"
#include "instance.h"
#include "tasksmodel.h"

using namespace Inspector::Internal;

IInspectorFramework::IInspectorFramework(Instance *instance, QObject *parent)
  : QObject(parent)
  , m_instance(instance)
{
    connect(instance->tasksModel(), SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(slotModelItemChanged(QStandardItem*)));
}

IInspectorFramework::~IInspectorFramework()
{
    // delete all the modules (bypassing the 'destroyed' hook)
    QList<IFrameworkModule *> listCopy = m_modules;
    m_modules.clear();
    qDeleteAll(listCopy);
}

void IInspectorFramework::addModule(IFrameworkModule * module)
{
    if (!module) {
        qWarning("IInspectorFramework::addModule: skipping null module");
        return;
    }
    // check for duplicate Ids
    foreach (IFrameworkModule *mod, m_modules) {
        if (mod->uid() == module->uid()) {
            qWarning("IInspectorFramework::addModule: skipping module with duplicated Uid %d", module->uid());
            return;
        }
    }
    // register the IFrameworkModule
    connect(module, SIGNAL(requestActivation(QString)), this, SLOT(slotModuleActivationRequested(QString)));
    connect(module, SIGNAL(deactivated()), this, SLOT(slotModuleDeactivated()));
    connect(module, SIGNAL(destroyed()), this, SLOT(slotModuleDestroyed()));
    m_modules.append(module);
    emit modulesChanged();
}

void IInspectorFramework::removeModule(IFrameworkModule * module)
{
    if (!module) {
        qWarning("IInspectorFramework::removeModule: skipping null module");
        return;
    }
    // unregister the IFrameworkModule
    disconnect(module, 0, this, 0);
    m_modules.removeAll(module);
    m_activeModules.removeAll(module);
    emit modulesChanged();
}

ModuleMenuEntries IInspectorFramework::menuEntries() const
{
    ModuleMenuEntries entries;
    foreach (IFrameworkModule *module, m_modules)
        entries.append(module->menuEntries());
    return entries;
}

QStringList IInspectorFramework::moduleNames() const
{
    QStringList names;
    foreach (IFrameworkModule *module, m_modules)
        names.append(module->name());
    return names;
}

AbstractPanel *IInspectorFramework::createPanel(int moduleUid, int panelId) const
{
    IFrameworkModule * module = moduleForUid(moduleUid);
    if (!module) {
        qWarning("IInspectorFramework::createPanel: unknown module Uid %d", moduleUid);
        return 0;
    }
    return module->createPanel(panelId);
}

IFrameworkModule *IInspectorFramework::moduleForUid(int moduleUid) const
{
    foreach (IFrameworkModule *module, m_modules)
        if (module->uid() == moduleUid)
            return module;
    return 0;
}

void IInspectorFramework::slotModuleActivationRequested(const QString &text)
{
    IFrameworkModule * module = static_cast<IFrameworkModule *>(sender());

    // update the model
    QString name = text.isEmpty() ? module->name() : text;
    if (!m_instance->tasksModel()->addTask(module->uid(), name, "provide description here")) {
        qWarning("IInspectorFramework::slotModuleActivationRequested: can't add module %d", module->uid());
        return;
    }
    if (!m_instance->tasksModel()->startTask(module->uid())) {
        qWarning("IInspectorFramework::slotModuleActivationRequested: can't start module %d", module->uid());
        return;
    }

    // activate module
    module->controlActivate();

    // CHANGE THIS? superseed by the model?
    if (!m_activeModules.contains(module))
        m_activeModules.append(module);
}

void IInspectorFramework::slotModuleDeactivated()
{
    IFrameworkModule * module = static_cast<IFrameworkModule *>(sender());

    // update the model
    if (!m_instance->tasksModel()->stopTask(module->uid())) {
        qWarning("IInspectorFramework::slotModuleDeactivated: can't stop module %d", module->uid());
        return;
    }

    // CHANGE THIS? superseed by the model?
    m_activeModules.removeAll(module);
}

void IInspectorFramework::slotModuleDestroyed()
{
    IFrameworkModule * module = static_cast<IFrameworkModule *>(sender());
    // CHANGE THIS? superseed by the model?
    if (m_modules.contains(module)) {
        m_instance->tasksModel()->stopTask(module->uid());
        removeModule(module);
    }
}

void IInspectorFramework::slotModelItemChanged(QStandardItem *item)
{
    // get the TaskItem
    TaskItem *taskItem = dynamic_cast<TaskItem *>(item);
    if (!taskItem)
        return;

    // check if the RequestStop flag is set
    if (taskItem->requestStop()) {
        // FIXME THIS: equivalence TaskId <-> ModuleId implied here
        IFrameworkModule * module = moduleForUid(taskItem->tid());
        if (module)
            module->controlDeactivate();
        return;
    }
}