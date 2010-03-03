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

#include "warningsmodule.h"
#include "notificationwidget.h"

#include "../localcommserver.h"
#include "../nokiaqtbackend.h"

#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>

using namespace Inspector::Internal;

//
// WarningsModule
//
WarningsModule::WarningsModule(NokiaQtBackend *backend, QObject *parent)
  : IBackendModule(backend, parent)
  , m_nqBackend(backend)
{
    // create the NotificationWidget and add it to CORE (do it now, to stay on top later)
    m_notification = new NotificationWidget;
    connect(m_notification, SIGNAL(clicked()), this, SLOT(slotNotificationClicked()));
    m_notification->hide();
    Core::ICore::instance()->modeManager()->addWidget(m_notification);

    // auto-create the WarningTask when the Backend gets connected
    connect(backend, SIGNAL(targetConnected(bool)),
            this, SLOT(slotBackendConnected(bool)));
}

WarningsModule::~WarningsModule()
{
    delete m_notification;
}

QString WarningsModule::name() const
{
    return tr("Auto-Warnings (0.2)");
}

ModuleMenuEntries WarningsModule::menuEntries() const
{
    ModuleMenuEntries entries;
    entries.append(ModuleMenuEntry(QStringList() << tr("Automatic") << tr("Warnings"), UID_MODULE_WARNINGS, 0, QIcon(":/inspector/warnings/menu-warning.png")));
    return entries;
}

void WarningsModule::slotBackendConnected(bool connected)
{
    if (connected) {
        WarningsTask *wt = new WarningsTask(m_nqBackend);
        connect(wt, SIGNAL(addWarning()), this, SLOT(slotTaskAddWarning()));
    }
}

void WarningsModule::slotNotificationClicked()
{
    // hide the Notification first
    m_notification->clearWarnings();
    m_notification->hide();

    // show our panel
    emit requestPanelDisplay(0);
}

void WarningsModule::slotTaskAddWarning()
{
    m_notification->addWarning();
    m_notification->show();
}

//
// WarningsTask
//
WarningsTask::WarningsTask(NokiaQtBackend *backend, QObject *parent)
  : IBackendTask(backend, parent)
  , m_commServer(backend->commServer())
{
    emit requestActivation();
}

QString WarningsTask::displayName() const
{
    return tr("Auto-Warnings");
}

void WarningsTask::activateTask()
{
    connect(m_commServer, SIGNAL(incomingData(quint32,quint32,QByteArray*)),
            this, SLOT(slotProcessIncomingData(quint32,quint32,QByteArray*)));
}

void WarningsTask::deactivateTask()
{
    disconnect(m_commServer, 0, this, 0);
    emit finished();
}

void WarningsTask::slotProcessIncomingData(quint32 channel, quint32 code1, QByteArray *data)
{
    Q_UNUSED(data);
    if (channel == 0x03 && code1 == 0x01)
        emit addWarning();
}
