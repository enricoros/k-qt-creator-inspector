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
#include "inspectorframe.h"
#include "inspectorplugin.h"
#include "notificationwidget.h"
#include "probecontroller.h"
#include "paint-probe/paintprobe.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>

#include "../../../share/qtcreator/gdbmacros/perfunction.h"

using namespace Inspector;

InspectorInstance::InspectorInstance(QObject *parent)
  : QObject(parent)
  , m_enabled(false)
  , m_debugPaintFlag(false)
  , m_sDebugging(false)
{
    // create the Server
    m_commServer = new CommServer;
    connect(m_commServer, SIGNAL(newWarnings(int)), this, SLOT(slotNewWarnings(int)));

    // create the Notification
    m_notification = new Internal::NotificationWidget;
    connect(m_notification, SIGNAL(clicked()), this, SLOT(slotShowProbeMode()));
    m_notification->hide();
    // add it to CORE (add it now, even if not visible, to stay on top later)
    Core::ICore::instance()->modeManager()->addWidget(m_notification);

    // create the Test Control & Tests
    m_probeController = new Internal::ProbeController(this);
    m_probeController->addProbe(new Internal::PaintProbe);
}

InspectorInstance::~InspectorInstance()
{
    delete m_probeController;
    delete m_notification;
    delete m_commServer;
}

CommServer *InspectorInstance::commServer() const
{
    return m_commServer;
}

Internal::ProbeController *InspectorInstance::probeController() const
{
    return m_probeController;
}

int InspectorInstance::probeActivationFlags() const
{
    // flags are in perfunction.h
    int flags = Inspector::Internal::AF_None;
    if (m_debugPaintFlag)
        flags |= Inspector::Internal::AF_PaintDebug;
    return flags;
}

void InspectorInstance::commCallFunction(const QString &name, QVariantList args)
{
    emit m_commServer->debuggerCallFunction(name, args);
}

void InspectorInstance::setDebugging(bool on)
{
    m_sDebugging = on;
}

bool InspectorInstance::debugging() const
{
    return m_sDebugging;
}

void InspectorInstance::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool InspectorInstance::enabled() const
{
    return m_enabled;
}

void InspectorInstance::setDebugPaint(bool checked)
{
    m_debugPaintFlag = checked;
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
