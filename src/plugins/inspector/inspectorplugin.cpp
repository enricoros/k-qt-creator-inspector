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

InspectorPlugin::InspectorPlugin()
  : m_instance(0)
  , m_aMemMonitor(0)
  , m_aShowPaint(0)
  , m_defaultActive(false)
{
}

InspectorPlugin::~InspectorPlugin()
{
    // objects registered with 'addAutoReleasedObject' will be removed
    // automatically
}

bool InspectorPlugin::showPaint() const
{
    return m_aShowPaint ? m_aShowPaint->isChecked() : false;
}

bool InspectorPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(error_message)

    // check arguments
    parseArguments(arguments);

    // create the Manager
    m_instance = new Inspector::InspectorInstance(this);
    m_instance->slotSetEnabled(m_defaultActive);
    addAutoReleasedObject(m_instance);

    // UI

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
    enableAction->setChecked(m_instance->enabled());
    connect(enableAction, SIGNAL(toggled(bool)), m_instance, SLOT(slotSetEnabled(bool)));
    command = actionManager->registerAction(enableAction, "Inspector.Enable", globalContext);
    inspContainer->addAction(command);

    QAction *infoAction = new QAction(tr("Information..."), this);
    connect(infoAction, SIGNAL(triggered()), m_instance, SLOT(slotShowInformation()));
    command = actionManager->registerAction(infoAction, "Inspector.Information", globalContext);
    inspContainer->addAction(command);

    QAction *workBenchAction = new QAction(tr("Workbench"), this);
    connect(workBenchAction, SIGNAL(triggered()), m_instance, SLOT(slotShowProbeMode()));
    command = actionManager->registerAction(workBenchAction, "Inspector.ShowWorkBench", globalContext);
    inspContainer->addAction(command);

    QAction *sep = new QAction(tr("Configuration"), this);
    sep->setSeparator(true);
    command = actionManager->registerAction(sep, QLatin1String("Inspector.Sep.One"), globalContext);
    inspContainer->addAction(command);

    m_aShowPaint = new QAction(tr("Show Painted Areas"), this);
    m_aShowPaint->setCheckable(true);
    command = actionManager->registerAction(m_aShowPaint, "Inspector.ShowPaintedAreas", globalContext);
    inspContainer->addAction(command);

    m_aMemMonitor = new QAction(tr("Allocation Analysis"), this);
    m_aMemMonitor->setCheckable(true);
    command = actionManager->registerAction(m_aMemMonitor, "Inspector.AnalyzeAllocations", globalContext);
    inspContainer->addAction(command);

    sep = new QAction(tr("Runtime Analysis"), this);
    sep->setSeparator(true);
    command = actionManager->registerAction(sep, QLatin1String("Inspector.Sep.Two"), globalContext);
    inspContainer->addAction(command);

#if 1
    QAction *temperatureAction = new QAction(tr("Painting Temperature"), this);
    connect(temperatureAction, SIGNAL(triggered()), m_instance, SLOT(slotPaintingTemperature()));
    command = actionManager->registerAction(temperatureAction, "Inspector.ShowTemperature", debuggerContext);
    inspContainer->addAction(command);
#endif

    Core::BaseMode * probeMode = new Core::BaseMode;
    probeMode->setName(tr("Probe"));
    probeMode->setIcon(QIcon(":/inspector/images/probe-icon-32.png"));
    probeMode->setPriority(Inspector::Internal::P_MODE_PROBE);
    probeMode->setWidget(m_instance->defaultWindow());
    probeMode->setUniqueModeName(Inspector::Internal::MODE_PROBE);
    probeMode->setContext(globalContext);
    addAutoReleasedObject(probeMode);

//    connect(core->modeManager(), SIGNAL(currentModeChanged(Core::IMode*)),
//            this, SLOT(modeChanged(Core::IMode*)), Qt::QueuedConnection);

    return true;
}

void InspectorPlugin::extensionsInitialized()
{
}

void InspectorPlugin::parseArguments(const QStringList &arguments)
{
    if (arguments.contains("-inspectoron", Qt::CaseInsensitive))
        m_defaultActive = true;
}

Q_EXPORT_PLUGIN(InspectorPlugin)
