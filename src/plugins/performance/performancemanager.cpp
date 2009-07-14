/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Started on 14 Jul 2009 by root.                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "performancemanager.h"
#include "performanceinformation.h"
#include "performancepane.h"
#include "performanceplugin.h"
#include "performanceserver.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>

#include "../../../share/qtcreator/gdbmacros/perfunction.h"

using namespace Performance;
using namespace Performance::Internal;

PerformanceManager::PerformanceManager(Internal::PerformancePlugin *plugin, QObject *parent)
  : QObject(parent)
  , m_plugin(plugin)
{
    // create the Pane
    m_pane = new PerformancePane;

    // create the Server
    PerformanceServer * server = new PerformanceServer;
    connect(server, SIGNAL(newWarnings(int)), this, SLOT(slotNewWarnings(int)));
    connect(server, SIGNAL(newString(const QString &)), this, SLOT(slotNewString(const QString &)));
    m_servers << server;

    // create the MiniWidget
    m_mini = new PerformanceMiniWidget;
    connect(m_mini, SIGNAL(clicked()), this, SLOT(slotShowWorkbench()));
    m_mini->hide();
    // add it to CORE (add it now, even if not visible, to stay on top later)
    Core::ICore::instance()->modeManager()->addWidget(m_mini);
}

PerformanceManager::~PerformanceManager()
{
    delete m_mini;
    delete m_pane;
    qDeleteAll(m_servers);
}

Internal::PerformancePane * PerformanceManager::pane() const
{
    return m_pane;
}

PerformanceServer * PerformanceManager::defaultServer() const
{
    return m_servers.isEmpty() ? 0 : m_servers.first();
}

int PerformanceManager::activationFlags() const
{
    // flags are in perfunction.h
    int flags = Performance::Internal::AF_None;
    if (m_plugin->showPaint())
        flags |= Performance::Internal::AF_PaintDebug;
    return flags;
}

void PerformanceManager::slotShowInformation()
{
    PerformanceServer * server = defaultServer();
    Internal::PerformanceInformation info;
    info.setFieldState(info.debLabel, server->m_sDebugging ? 1 : -1);
    info.setFieldState(info.enaButton, server->m_sEnabled ? 1 : -1);
    info.setFieldState(info.hlpLabel, server->m_sHelpers ? 1 : server->m_sDebugging ? -1 : 0);
    info.setFieldState(info.injLabel, server->m_sInjected ? 1 : server->m_sDebugging ? -1 : 0);
    info.setFieldState(info.conLabel, server->m_sConnected ? 1 : server->m_sDebugging ? -1 : 0);
    info.setFieldState(info.workLabel, (server->m_sDebugging && server->m_sEnabled && server->m_sInjected && server->m_sConnected) ? 1 : 0);
    info.exec();
}

void PerformanceManager::slotShowWorkbench()
{
    // hide the MiniWidget first
    m_mini->hide();

    // CORE: switch to full screen performance view
    Core::ICore::instance()->modeManager()->activateMode(Core::Constants::MODE_OUTPUT);
    m_pane->popup(true);
}

void PerformanceManager::slotNewString(const QString & string)
{
    m_pane->addString(string);
}

void PerformanceManager::slotNewWarnings(int count)
{
    for (int i = 0; i < count; i++)
        m_mini->addWarning();
    m_mini->show();
}
