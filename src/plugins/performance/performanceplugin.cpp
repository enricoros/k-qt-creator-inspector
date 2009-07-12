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
#include "performancepane.h"
#include "performanceserver.h"

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
    : m_server(0)
    , m_pane(0)
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

    // get the primary access point to the workbench.
    Core::ICore *core = Core::ICore::instance();
    Core::ActionManager *actionManager = core->actionManager();
    Core::Command *command;

    // Create a unique context id for our own view, that will be used for the menu entry later
    QList<int> contexts = QList<int>()
//        << Core::Constants::C_GLOBAL_ID // for debug
        << core->uniqueIDManager()->uniqueIdentifier(Debugger::Constants::GDBRUNNING /*C_GDBDEBUGGER*/);

    // create the Performance Menu and add it to the Debug menu
    Core::ActionContainer *perfContainer = actionManager->createMenu("Performance.Container");
    QMenu *perfMenu = perfContainer->menu();
    perfMenu->setTitle(tr("&Performance"));
    perfMenu->setEnabled(true);
    Core::ActionContainer *debugContainer = actionManager->actionContainer(ProjectExplorer::Constants::M_DEBUG);
    debugContainer->addMenu(perfContainer);

    // create (and register to the system) the actions
    m_aPerfMonitor = new QAction(tr("Measure Performance"), this);
    connect(m_aPerfMonitor, SIGNAL(triggered()), SLOT(slotPerformance()));
    command = actionManager->registerAction(m_aPerfMonitor, "Performance.PerformanceMonitor", contexts);
    perfContainer->addAction(command);

    m_aLagMonitor = new QAction(tr("Transfer Function(s)"), this);
    connect(m_aLagMonitor, SIGNAL(triggered()), SLOT(slotLag()));
    command = actionManager->registerAction(m_aLagMonitor, "Performance.LagMonitor", contexts);
    perfContainer->addAction(command);

    m_aShowPaint = new QAction(tr("Show Painted Areas"), this);
    m_aShowPaint->setCheckable(true);
    connect(m_aShowPaint, SIGNAL(triggered()), SLOT(slotPerformance()));
    command = actionManager->registerAction(m_aShowPaint, "Performance.ShowPaintedAreas", contexts);
    perfContainer->addAction(command);

    m_aMemMonitor = new QAction(tr("Memory Manager"), this);
    m_aMemMonitor->setEnabled(false);
    command = actionManager->registerAction(m_aMemMonitor, "Performance.MemoryMonitor", contexts);
    perfContainer->addAction(command);

    // Create the Pane
    m_pane = new PerformancePane;
    addAutoReleasedObject(m_pane);

    // Create the Server
    m_server = new PerformanceServer(m_pane);
    addAutoReleasedObject(m_server);

    return true;
}

/*! Notification that all extensions that this plugin depends on have been
    initialized. The dependencies are defined in the plugins .qwp file.

    Normally this method is used for things that rely on other plugins to have
    added objects to the plugin manager, that implement interfaces that we're
    interested in. These objects can now be requested through the
    PluginManagerInterface.

    The PerformancePlugin doesn't need things from other plugins, so it does
    nothing here.
*/
#include <extensionsystem/pluginmanager.h>
#include <debugger/debuggerplugin.h>

void PerformancePlugin::extensionsInitialized()
{
    // TODO: request debugger objects here (for injection management)
    Debugger::Internal::DebuggerPlugin * plugin = ExtensionSystem::PluginManager::instance()->getObject<Debugger::Internal::DebuggerPlugin>();
    qWarning("dbg: %x", plugin);
//    ExtensionSystem::PluginManager::instance()->getObject<CppTools::CppModelManagerInterface>();
//    CppTools::CppModelManagerInterface *modelManager
//            = ExtensionSystem::PluginManager::instance()->getObject<CppTools::CppModelManagerInterface>();

}

void PerformancePlugin::slotPerformance()
{
    QMessageBox::information(0, tr("Performance!"), tr("Performance!! Beautiful day today, isn't it?"));
}

void PerformancePlugin::slotLag()
{
    QMessageBox::information(0, tr("Lag!"), tr("Performance!! Beautiful day today, isn't it?"));
}

Q_EXPORT_PLUGIN(PerformancePlugin)
