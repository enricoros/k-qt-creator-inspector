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

#include "instance.h"
#include "commserver.h"
#include "inspectorplugin.h"
#include "notificationwidget.h"
#include "modulecontroller.h"
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>

using namespace Inspector;

#include "instancemodel.h"

Instance::Instance(QObject *parent)
  : QObject(parent)
  , m_model(new InstanceModel)
  , m_enabled(false)
  , m_debugPaintFlag(false)
  , m_sDebugging(false)
{
    // create the CommServer
    m_commServer = new Internal::CommServer(m_model);
    connect(m_commServer, SIGNAL(newWarnings(int)), this, SLOT(slotNewWarnings(int)));

    // create the NotificationWidget
    m_notification = new Internal::NotificationWidget;
    connect(m_notification, SIGNAL(clicked()), this, SLOT(slotNotificationTriggered()));
    m_notification->hide();
    // add it to CORE (add it now, even if not visible, to stay on top later)
    Core::ICore::instance()->modeManager()->addWidget(m_notification);

    // create the Test Control & Tests
    m_moduleController = new Internal::ModuleController(this);
}

Instance::~Instance()
{
    delete m_moduleController;
    delete m_notification;
    delete m_commServer;
    delete m_model;
}

InstanceModel *Instance::model() const
{
    return m_model;
}

Internal::CommServer *Instance::commServer() const
{
    return m_commServer;
}

Internal::ModuleController *Instance::moduleController() const
{
    return m_moduleController;
}

void Instance::slotNotificationTriggered()
{
    // hide the Notification first
    m_notification->clearWarnings();
    m_notification->hide();

    // switch view to this instance
    emit requestDisplay();
}

void Instance::slotNewWarnings(int count)
{
    for (int i = 0; i < count; i++)
        m_notification->addWarning();
    m_notification->show();
}
