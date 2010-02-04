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

#include "inspectorplugin.h"
#include "inspectorcontainer.h"
#include "instance.h"
#include "nokiaqtframework/nokiaqtframework.h"
#include "nvidiacudaframework/nvidiacudaframework.h"
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/basemode.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <debugger/debuggerconstants.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <QtCore/QDebug>
#include <QtCore/QtPlugin>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

using namespace Inspector::Internal;

/*Q_DECL_EXPORT Inspector::Internal::Instance * Inspector::defaultInstance()
{
    return InspectorPlugin::defaultInstance();
}*/

InspectorPlugin *Inspector::Internal::InspectorPlugin::s_pluginInstance = 0;

InspectorPlugin::InspectorPlugin()
  : ExtensionSystem::IPlugin()
  , m_container(0)
  , m_pluginEnabled(false)
{
    // reference the plugin instance (for static accessors)
    s_pluginInstance = this;
}

InspectorPlugin::~InspectorPlugin()
{
    // goodbye plugin
    s_pluginInstance = 0;

    // delete instances
    qDeleteAll(m_instances);
    m_instances.clear();

    // objects registered with 'addAutoReleasedObject' will be removed automatically, like:
    // m_window is deleted by the plugin system
    m_container = 0;
}

void InspectorPlugin::addInstance(Instance * instance)
{
    if (m_instances.contains(instance)) {
        qWarning("InspectorPlugin::addInstance: instance already present");
        return;
    }

    instance->instanceModel()->setInstanceEnabled(m_pluginEnabled);
    m_instances.append(instance);
    m_container->addInstance(instance);
}

Instance * InspectorPlugin::defaultInstance()
{
    if (!s_pluginInstance || s_pluginInstance->m_instances.isEmpty())
        return 0;
    return s_pluginInstance->m_instances.first();
}

bool InspectorPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(error_message)

    // check arguments
    parseArguments(arguments);

    // get core objects
    Core::ICore *core = Core::ICore::instance();
    QList<int> ourContext;
    ourContext << Core::Constants::C_GLOBAL_ID;
    ourContext << core->uniqueIDManager()->uniqueIdentifier(Debugger::Constants::GDBRUNNING);

    addAutoReleasedObject(new NokiaQtFrameworkFactory());
    addAutoReleasedObject(new NvidiaCudaFrameworkFactory());

    m_container = new InspectorContainer;
    connect(m_container, SIGNAL(requestWindowDisplay()), this, SLOT(slotDisplayWindow()));

    // create the Mode, that registers the widget too
    Core::BaseMode * inspectorMode = new Core::BaseMode;
    inspectorMode->setDisplayName(tr("Probe"));
    inspectorMode->setId(QLatin1String(Inspector::Internal::MODE_INSPECTOR));
    inspectorMode->setIcon(QIcon(":/inspector/images/inspector-icon-32.png"));
    inspectorMode->setPriority(Inspector::Internal::P_MODE_INSPECTOR);
    inspectorMode->setWidget(m_container);
    inspectorMode->setContext(ourContext);
    addAutoReleasedObject(inspectorMode);

    // create the Menu and add it to the Debug menu
    Core::ActionManager *actionManager = core->actionManager();
    Core::ActionContainer *inspContainer = actionManager->createMenu("Inspector.Container");
    QMenu *inspMenu = inspContainer->menu();
    inspMenu->setTitle(tr("&Inspector"));
    inspMenu->setIcon(QIcon(":/inspector/images/menu-display.png"));
    Core::ActionContainer *debugContainer = actionManager->actionContainer(ProjectExplorer::Constants::M_DEBUG);
    debugContainer->addMenu(inspContainer);

    QAction *enableAction = new QAction(tr("Enable"), this);
    enableAction->setCheckable(true);
    enableAction->setChecked(m_pluginEnabled);
    connect(enableAction, SIGNAL(toggled(bool)), this, SLOT(slotSetPluginEnabled(bool)));
    Core::Command *command = actionManager->registerAction(enableAction, "Inspector.Enable", ourContext);
    inspContainer->addAction(command);

    QAction *workBenchAction = new QAction(tr("Current Instance"), this);
    connect(workBenchAction, SIGNAL(triggered()), this, SLOT(slotDisplayWindow()));
    command = actionManager->registerAction(workBenchAction, "Inspector.ShowInstance", ourContext);
    inspContainer->addAction(command);

//    connect(core->modeManager(), SIGNAL(currentModeChanged(Core::IMode*)),
//            this, SLOT(modeChanged(Core::IMode*)), Qt::QueuedConnection);

    // create a single Instance - SINGLE debuggee is supposed here
    Instance *instance = new Instance;
    addInstance(instance);

    return true;
}

void InspectorPlugin::extensionsInitialized()
{
}

void InspectorPlugin::slotSetPluginEnabled(bool enabled)
{
    // enable/disable all Instances
    m_pluginEnabled = enabled;
    foreach (Instance * instance, m_instances)
        instance->instanceModel()->setInstanceEnabled(enabled);
}

void InspectorPlugin::slotDisplayWindow()
{
    // switch creator to the Inspector Mode
    Core::ICore::instance()->modeManager()->activateMode(Inspector::Internal::MODE_INSPECTOR);
}

void InspectorPlugin::parseArguments(const QStringList &arguments)
{
    if (arguments.contains("-inspectoron", Qt::CaseInsensitive))
        m_pluginEnabled = true;
}

Q_EXPORT_PLUGIN(InspectorPlugin)
