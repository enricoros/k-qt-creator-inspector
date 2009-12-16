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

#include "inspectorinstance.h"
#include "commserver.h"
#include "infodialog.h"
#include "inspectorplugin.h"
#include "inspectorframe.h"
#include "notificationwidget.h"
#include "probecontroller.h"
#include "paint-probe/paintprobe.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>

#include "../../../share/qtcreator/gdbmacros/perfunction.h"

using namespace Inspector;
using namespace Inspector::Internal;

InspectorInstance *InspectorInstance::s_instance = 0;

InspectorInstance::InspectorInstance(Internal::InspectorPlugin *plugin, QObject *parent)
  : QObject(parent)
  , m_plugin(plugin)
  , m_enabled(false)
{
    // save the instance (there is only 1 manager)
    s_instance = this;

    // create the Server
    CommServer * server = new CommServer;
    connect(server, SIGNAL(newWarnings(int)), this, SLOT(slotNewWarnings(int)));
    m_servers << server;

    // create the Notification
    m_notification = new NotificationWidget;
    connect(m_notification, SIGNAL(clicked()), this, SLOT(slotShowProbeMode()));
    m_notification->hide();
    // add it to CORE (add it now, even if not visible, to stay on top later)
    Core::ICore::instance()->modeManager()->addWidget(m_notification);

    // create the Test Control & Tests
    m_probeController = new ProbeController(this);
    m_probeController->addTest(new PaintProbe);

    // create the Window
    m_window = new InspectorFrame(m_probeController);
    m_window->showDefaultView();
}

InspectorInstance::~InspectorInstance()
{
    // m_window is deleted by the plugin system (added by InspectorPlugin)
    delete m_notification;
    qDeleteAll(m_servers);
    s_instance = 0;
}

InspectorInstance *InspectorInstance::instance()
{
    return s_instance;
}

bool InspectorInstance::enabled() const
{
    return m_enabled;
}

Internal::InspectorFrame * InspectorInstance::defaultWindow() const
{
    return m_window;
}

CommServer * InspectorInstance::defaultComm() const
{
    return m_servers.isEmpty() ? 0 : m_servers.first();
}

int InspectorInstance::activationFlags() const
{
    // flags are in perfunction.h
    int flags = Inspector::Internal::AF_None;
    if (m_plugin->showPaint())
        flags |= Inspector::Internal::AF_PaintDebug;
    return flags;
}

void InspectorInstance::defaultServerCallFunction(const QString &name, QVariantList args)
{
    if (!m_servers.isEmpty())
        emit m_servers.first()->debuggerCallFunction(name, args);
}

void InspectorInstance::slotSetEnabled(bool enabled)
{
    m_enabled = enabled;
}

void InspectorInstance::slotShowInformation()
{
    CommServer * server = defaultComm();
    Internal::InfoDialog info;
    info.setFieldState(info.debLabel, server->m_sDebugging ? 1 : -1);
    info.setFieldState(info.enaButton, server->m_sEnabled ? 1 : -1);
    info.setFieldState(info.hlpLabel, server->m_sHelpers ? 1 : server->m_sDebugging ? -1 : 0);
    info.setFieldState(info.injLabel, server->m_sInjected ? 1 : server->m_sDebugging ? -1 : 0);
    info.setFieldState(info.conLabel, server->m_sConnected ? 1 : server->m_sDebugging ? -1 : 0);
    info.setFieldState(info.workLabel, (server->m_sDebugging && server->m_sEnabled && server->m_sInjected && server->m_sConnected) ? 1 : 0);
    info.exec();
}

void InspectorInstance::slotShowProbeMode()
{
    // hide the Notification first
    m_notification->clearWarnings();
    m_notification->hide();

    // switch to the Probe view
    Core::ICore::instance()->modeManager()->activateMode(Inspector::Internal::MODE_PROBE);
}

void InspectorInstance::slotNewWarnings(int count)
{
    for (int i = 0; i < count; i++)
        m_notification->addWarning();
    m_notification->show();
}
