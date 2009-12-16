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

#include "performancemanager.h"
#include "infoview.h"
#include "performancenotification.h"
#include "performanceplugin.h"
#include "performanceserver.h"
#include "performancewindow.h"
#include "testcontrol.h"
#include "paint-test/painttest.h"

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

    // create the Server
    PerformanceServer * server = new PerformanceServer;
    connect(server, SIGNAL(newWarnings(int)), this, SLOT(slotNewWarnings(int)));
    m_servers << server;

    // create the Notification
    m_notification = new PerformanceNotification;
    connect(m_notification, SIGNAL(clicked()), this, SLOT(slotShowProbeMode()));
    m_notification->hide();
    // add it to CORE (add it now, even if not visible, to stay on top later)
    Core::ICore::instance()->modeManager()->addWidget(m_notification);

    // create the Test Control & Tests
    m_testControl = new TestControl(this);
    m_testControl->addTest(new PaintTest);

    // create the Window
    m_window = new PerformanceWindow(m_testControl);
    m_window->showDefaultView();
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

void PerformanceManager::defaultServerCallFunction(const QString &name, QVariantList args)
{
    if (!m_servers.isEmpty())
        emit m_servers.first()->debuggerCallFunction(name, args);
}

void PerformanceManager::slotSetEnabled(bool enabled)
{
    m_enabled = enabled;
}

void PerformanceManager::slotShowInformation()
{
    PerformanceServer * server = defaultServer();
    Internal::InfoView info;
    info.setFieldState(info.debLabel, server->m_sDebugging ? 1 : -1);
    info.setFieldState(info.enaButton, server->m_sEnabled ? 1 : -1);
    info.setFieldState(info.hlpLabel, server->m_sHelpers ? 1 : server->m_sDebugging ? -1 : 0);
    info.setFieldState(info.injLabel, server->m_sInjected ? 1 : server->m_sDebugging ? -1 : 0);
    info.setFieldState(info.conLabel, server->m_sConnected ? 1 : server->m_sDebugging ? -1 : 0);
    info.setFieldState(info.workLabel, (server->m_sDebugging && server->m_sEnabled && server->m_sInjected && server->m_sConnected) ? 1 : 0);
    info.exec();
}

void PerformanceManager::slotShowProbeMode()
{
    // hide the Notification first
    m_notification->clearWarnings();
    m_notification->hide();

    // switch to the Probe view
    Core::ICore::instance()->modeManager()->activateMode(Performance::Internal::MODE_PROBE);
}

void PerformanceManager::slotNewWarnings(int count)
{
    for (int i = 0; i < count; i++)
        m_notification->addWarning();
    m_notification->show();
}
