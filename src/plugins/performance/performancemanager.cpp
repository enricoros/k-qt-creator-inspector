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
#include "performancenotification.h"
#include "performanceplugin.h"
#include "performanceserver.h"
#include "performancewindow.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>

#include "../../../share/qtcreator/gdbmacros/perfunction.h"

using namespace Performance;
using namespace Performance::Internal;

PerformanceManager *PerformanceManager::s_instance = 0;

PerformanceManager::PerformanceManager(Internal::PerformancePlugin *plugin, QObject *parent)
  : QObject(parent)
  , m_plugin(plugin)
  , m_enabled(false)
{
    // save the instance (there is only 1 manager)
    s_instance = this;

    // create the Window
    m_window = new PerformanceWindow;

    // create the Server
    PerformanceServer * server = new PerformanceServer;
    connect(server, SIGNAL(newWarnings(int)), this, SLOT(slotNewWarnings(int)));
    m_servers << server;

    // create the Notification
    m_notification = new PerformanceNotification;
    connect(m_notification, SIGNAL(clicked()), this, SLOT(slotShowRuntimeMode()));
    m_notification->hide();
    // add it to CORE (add it now, even if not visible, to stay on top later)
    Core::ICore::instance()->modeManager()->addWidget(m_notification);
}

PerformanceManager::~PerformanceManager()
{
    // m_window is deleted by the plugin system (added by PerformancePlugin)
    delete m_notification;
    qDeleteAll(m_servers);
    s_instance = 0;
}

PerformanceManager *PerformanceManager::instance()
{
    return s_instance;
}

bool PerformanceManager::enabled() const
{
    return m_enabled;
}

Internal::PerformanceWindow * PerformanceManager::defaultWindow() const
{
    return m_window;
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

void PerformanceManager::slotSetEnabled(bool enabled)
{
    m_enabled = enabled;
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

void PerformanceManager::slotShowRuntimeMode()
{
    // hide the Notification first
    m_notification->clearWarnings();
    m_notification->hide();

    // switch to the Runtime view
    Core::ICore::instance()->modeManager()->activateMode(Performance::Internal::MODE_RUNTIME);
}

void PerformanceManager::slotPaintingTemperature()
{
    QVariantList args;
    // passes << headDrops << tailDrops << innerPasses << chunkWidth << chunkHeight << consoleDebug
    args << 5 << 1 << 2 << 4 << 20 << 20 << true;
    //args << 4 << 0 << 2 << 1 << 1 << 1 << true;
    emit defaultServer()->debuggerCallFunction("qWindowTemperature", args);
}

void PerformanceManager::slotNewWarnings(int count)
{
    for (int i = 0; i < count; i++)
        m_notification->addWarning();
    m_notification->show();
}
