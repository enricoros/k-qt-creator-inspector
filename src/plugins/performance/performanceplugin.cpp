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
#include "performanceinformation.h"
#include "performancemanager.h"
#include "performancepane.h"

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
{
}

PerformancePlugin::~PerformancePlugin()
{
    // objects registered with 'addAutoReleasedObject' will be removed
    // automatically
}

bool PerformancePlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments)
    Q_UNUSED(error_message)

    // create the Manager
    m_manager = new Performance::PerformanceManager;
    addAutoReleasedObject(m_manager);
    addAutoReleasedObject(m_manager->pane());

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

    QAction *infoAction = new QAction(tr("Information..."), this);
    connect(infoAction, SIGNAL(triggered()), m_manager, SLOT(slotShowInformation()));
    command = actionManager->registerAction(infoAction, "Performance.Information", globalContext);
    perfContainer->addAction(command);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    command = actionManager->registerAction(sep, QLatin1String("Performance.Sep.One"), globalContext);
    perfContainer->addAction(command);

    QAction *workBenchAction = new QAction(tr("Workbench"), this);
    connect(workBenchAction, SIGNAL(triggered()), m_manager, SLOT(slotShowWorkbench()));
    command = actionManager->registerAction(workBenchAction, "Performance.ShowWorkBench", globalContext);
    perfContainer->addAction(command);

    m_aShowPaint = new QAction(tr("Show Painted Areas"), this);
    m_aShowPaint->setCheckable(true);
    connect(m_aShowPaint, SIGNAL(toggled(bool)), SLOT(slotShowPaint(bool)));
    command = actionManager->registerAction(m_aShowPaint, "Performance.ShowPaintedAreas", debuggerContext);
    perfContainer->addAction(command);

    m_aMemMonitor = new QAction(tr("Allocation Analysis"), this);
    m_aMemMonitor->setCheckable(true);
    connect(m_aMemMonitor, SIGNAL(toggled(bool)), SLOT(slotShowPaint(bool)));
    command = actionManager->registerAction(m_aMemMonitor, "Performance.AnalyzeAllocations", debuggerContext);
    perfContainer->addAction(command);

    return true;
}

void PerformancePlugin::extensionsInitialized()
{
}

void PerformancePlugin::slotShowPaint(bool /*show*/)
{
    QMessageBox::information(0, tr("Performance!"), tr("Performance!! Beautiful day today, isn't it?"));
}

Q_EXPORT_PLUGIN(PerformancePlugin)
