/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
** contact the sales department at http://www.qtsoftware.com/contact.
**
**************************************************************************/

#include "performanceplugin.h"
#include "performancemanager.h"
#include "performancenotification.h"
#include "performancewindow.h"

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

using namespace Performance::Internal;

PerformancePlugin::PerformancePlugin()
  : m_manager(0)
  , m_aMemMonitor(0)
  , m_aShowPaint(0)
  , m_defaultActive(false)
{
}

PerformancePlugin::~PerformancePlugin()
{
    // objects registered with 'addAutoReleasedObject' will be removed
    // automatically
}

bool PerformancePlugin::showPaint() const
{
    return m_aShowPaint ? m_aShowPaint->isChecked() : false;
}

bool PerformancePlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(error_message)

    // check arguments
    parseArguments(arguments);

    // create the Manager
    m_manager = new Performance::PerformanceManager(this);
    m_manager->slotSetEnabled(m_defaultActive);
    addAutoReleasedObject(m_manager);

    // UI

    // get core objects
    Core::ICore *core = Core::ICore::instance();
    Core::ActionManager *actionManager = core->actionManager();
    QList<int> globalContext = QList<int>()
        << Core::Constants::C_GLOBAL_ID;
    QList<int> debuggerContext = QList<int>()
        << core->uniqueIDManager()->uniqueIdentifier(Debugger::Constants::GDBRUNNING);

    // create the Performance Menu and add it to the Debug menu
    Core::ActionContainer *perfContainer = actionManager->createMenu("Performance.Container");
    QMenu *perfMenu = perfContainer->menu();
    perfMenu->setTitle(tr("&Performance"));
    perfMenu->setIcon(QIcon(":/performance/images/menu-icon.png"));
    Core::ActionContainer *debugContainer = actionManager->actionContainer(ProjectExplorer::Constants::M_DEBUG);
    debugContainer->addMenu(perfContainer);

    // create (and register to the system) the actions
    Core::Command *command;

    QAction *enableAction = new QAction(tr("Enable"), this);
    enableAction->setCheckable(true);
    enableAction->setChecked(m_manager->enabled());
    connect(enableAction, SIGNAL(toggled(bool)), m_manager, SLOT(slotSetEnabled(bool)));
    command = actionManager->registerAction(enableAction, "Performance.Enable", globalContext);
    perfContainer->addAction(command);

    QAction *infoAction = new QAction(tr("Information..."), this);
    connect(infoAction, SIGNAL(triggered()), m_manager, SLOT(slotShowInformation()));
    command = actionManager->registerAction(infoAction, "Performance.Information", globalContext);
    perfContainer->addAction(command);

    QAction *workBenchAction = new QAction(tr("Workbench"), this);
    connect(workBenchAction, SIGNAL(triggered()), m_manager, SLOT(slotShowProbeMode()));
    command = actionManager->registerAction(workBenchAction, "Performance.ShowWorkBench", globalContext);
    perfContainer->addAction(command);

    QAction *sep = new QAction(tr("Configuration"), this);
    sep->setSeparator(true);
    command = actionManager->registerAction(sep, QLatin1String("Performance.Sep.One"), globalContext);
    perfContainer->addAction(command);

    m_aShowPaint = new QAction(tr("Show Painted Areas"), this);
    m_aShowPaint->setCheckable(true);
    command = actionManager->registerAction(m_aShowPaint, "Performance.ShowPaintedAreas", globalContext);
    perfContainer->addAction(command);

    m_aMemMonitor = new QAction(tr("Allocation Analysis"), this);
    m_aMemMonitor->setCheckable(true);
    command = actionManager->registerAction(m_aMemMonitor, "Performance.AnalyzeAllocations", globalContext);
    perfContainer->addAction(command);

    sep = new QAction(tr("Runtime Analysis"), this);
    sep->setSeparator(true);
    command = actionManager->registerAction(sep, QLatin1String("Performance.Sep.Two"), globalContext);
    perfContainer->addAction(command);

#if 1
    QAction *temperatureAction = new QAction(tr("Painting Temperature"), this);
    connect(temperatureAction, SIGNAL(triggered()), m_manager, SLOT(slotPaintingTemperature()));
    command = actionManager->registerAction(temperatureAction, "Performance.ShowTemperature", debuggerContext);
    perfContainer->addAction(command);
#endif

    Core::BaseMode * probeMode = new Core::BaseMode;
    probeMode->setName(tr("Probe"));
    probeMode->setIcon(QIcon(":/performance/images/probe-icon-32.png"));
    probeMode->setPriority(Performance::Internal::P_MODE_PROBE);
    probeMode->setWidget(m_manager->defaultWindow());
    probeMode->setUniqueModeName(Performance::Internal::MODE_PROBE);
    probeMode->setContext(globalContext);
    addAutoReleasedObject(probeMode);

//    connect(core->modeManager(), SIGNAL(currentModeChanged(Core::IMode*)),
//            this, SLOT(modeChanged(Core::IMode*)), Qt::QueuedConnection);

    return true;
}

void PerformancePlugin::extensionsInitialized()
{
}

void PerformancePlugin::parseArguments(const QStringList &arguments)
{
    if (arguments.contains("-performanceon", Qt::CaseInsensitive))
        m_defaultActive = true;
}

Q_EXPORT_PLUGIN(PerformancePlugin)
