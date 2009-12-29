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
#include "inspectorinstance.h"
#include "notificationwidget.h"
#include "inspectorframe.h"
#include "probecontroller.h"

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

Q_DECL_EXPORT Inspector::InspectorInstance * Inspector::defaultInstance()
{
    return InspectorPlugin::defaultInstance();
}

InspectorPlugin *Inspector::Internal::InspectorPlugin::s_pluginInstance = 0;

InspectorPlugin::InspectorPlugin()
  : ExtensionSystem::IPlugin()
  , m_window(0)
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
    m_window = 0;
}

Inspector::InspectorInstance * InspectorPlugin::defaultInstance()
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

    // create a single Instance - SINGLE debuggee is supposed here
    Inspector::InspectorInstance *instance = new Inspector::InspectorInstance;
    m_instances.append(instance);
    instance->setEnabled(m_pluginEnabled);
    connect(instance, SIGNAL(requestDisplay()), this, SLOT(slotDisplayInstance()));

    // UI

    // create the Window
    m_window = new InspectorFrame;
    m_window->setInstance(instance);

    // get core objects
    Core::ICore *core = Core::ICore::instance();
    Core::ActionManager *actionManager = core->actionManager();
    QList<int> globalContext = QList<int>()
        << Core::Constants::C_GLOBAL_ID;
    QList<int> debuggerContext = QList<int>()
        << core->uniqueIDManager()->uniqueIdentifier(Debugger::Constants::GDBRUNNING);

    // create the Menu and add it to the Debug menu
    Core::ActionContainer *inspContainer = actionManager->createMenu("Inspector.Container");
    QMenu *inspMenu = inspContainer->menu();
    inspMenu->setTitle(tr("&Inspector"));
    inspMenu->setIcon(QIcon(":/inspector/images/menu-icon.png"));
    Core::ActionContainer *debugContainer = actionManager->actionContainer(ProjectExplorer::Constants::M_DEBUG);
    debugContainer->addMenu(inspContainer);

    // create (and register to the system) the actions
    Core::Command *command;

    QAction *enableAction = new QAction(tr("Enable"), this);
    enableAction->setCheckable(true);
    enableAction->setChecked(m_pluginEnabled);
    connect(enableAction, SIGNAL(toggled(bool)), this, SLOT(slotSetEnabled(bool)));
    command = actionManager->registerAction(enableAction, "Inspector.Enable", globalContext);
    inspContainer->addAction(command);

    QAction *workBenchAction = new QAction(tr("Workbench"), this);
    connect(workBenchAction, SIGNAL(triggered()), this, SLOT(slotDisplayInstance()));
    command = actionManager->registerAction(workBenchAction, "Inspector.ShowWorkBench", globalContext);
    inspContainer->addAction(command);

    QAction *sep = new QAction(tr("Configuration"), this);
    sep->setSeparator(true);
    command = actionManager->registerAction(sep, QLatin1String("Inspector.Sep.One"), globalContext);
    inspContainer->addAction(command);

    QAction *debugPaintAction = new QAction(tr("Show Painted Areas"), this);
    debugPaintAction->setCheckable(true);
    command = actionManager->registerAction(debugPaintAction, "Inspector.ShowPaintedAreas", globalContext);
    inspContainer->addAction(command);
    connect(debugPaintAction, SIGNAL(toggled(bool)), this, SLOT(slotDebugPaintToggled(bool)));

    QAction *allocAction = new QAction(tr("Allocation Analysis"), this);
    allocAction->setCheckable(true);
    command = actionManager->registerAction(allocAction, "Inspector.AnalyzeAllocations", globalContext);
    inspContainer->addAction(command);
    //connect(allocAction, SIGNAL(toggled(bool)), this, SLOT(slotDebugAllocationToggled(bool)));

    sep = new QAction(tr("Runtime Analysis"), this);
    sep->setSeparator(true);
    command = actionManager->registerAction(sep, QLatin1String("Inspector.Sep.Two"), globalContext);
    inspContainer->addAction(command);

#if 1
    QAction *temperatureAction = new QAction(tr("Painting Temperature"), this);
    connect(temperatureAction, SIGNAL(triggered()), this, SLOT(slotTempPaintingTemperature()));
    command = actionManager->registerAction(temperatureAction, "Inspector.ShowTemperature", debuggerContext);
    inspContainer->addAction(command);
#endif

    // create the Mode, that registers the widget too
    Core::BaseMode * inspectorMode = new Core::BaseMode;
    inspectorMode->setName(tr("Probe"));
    inspectorMode->setIcon(QIcon(":/inspector/images/probe-icon-32.png"));
    inspectorMode->setPriority(Inspector::Internal::P_MODE_INSPECTOR);
    inspectorMode->setWidget(m_window);
    inspectorMode->setUniqueModeName(Inspector::Internal::MODE_INSPECTOR);
    inspectorMode->setContext(globalContext);
    addAutoReleasedObject(inspectorMode);

//    connect(core->modeManager(), SIGNAL(currentModeChanged(Core::IMode*)),
//            this, SLOT(modeChanged(Core::IMode*)), Qt::QueuedConnection);

    return true;
}

void InspectorPlugin::extensionsInitialized()
{
}

void InspectorPlugin::slotDisplayInstance()
{
    // switch window to calling InspectorInstance if present
    Inspector::InspectorInstance *instance = dynamic_cast<Inspector::InspectorInstance *>(sender());
    if (instance && m_instances.contains(instance))
        m_window->setInstance(instance);

    // switch to the Probe view
    Core::ICore::instance()->modeManager()->activateMode(Inspector::Internal::MODE_INSPECTOR);
}

void InspectorPlugin::slotDebugPaintToggled(bool checked)
{
    m_instances.first()->setDebugPaint(checked);
}

void InspectorPlugin::slotTempPaintingTemperature()
{
    defaultInstance()->probeController()->activatePTProbe();
}

void InspectorPlugin::slotSetEnabled(bool enabled)
{
    // enable/disable all Instances
    m_pluginEnabled = enabled;
    foreach (InspectorInstance * instance, m_instances)
        instance->setEnabled(enabled);
}

void InspectorPlugin::parseArguments(const QStringList &arguments)
{
    if (arguments.contains("-inspectoron", Qt::CaseInsensitive))
        m_pluginEnabled = true;
}

Q_EXPORT_PLUGIN(InspectorPlugin)
