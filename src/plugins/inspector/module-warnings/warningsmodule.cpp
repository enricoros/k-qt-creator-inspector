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
#include "commserver.h"
#include "instance.h"
#include "notificationwidget.h"
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <QTimer>

using namespace Inspector::Internal;

WarningsModule::WarningsModule(Inspector::Instance *instance, QObject *parent)
  : AbstractModule(instance, parent)
{
    // create the NotificationWidget and add it to CORE (do it now, to stay on top later)
    m_notification = new Internal::NotificationWidget;
    connect(m_notification, SIGNAL(clicked()), this, SLOT(slotNotificationClicked()));
    m_notification->hide();
    Core::ICore::instance()->modeManager()->addWidget(m_notification);

    // activate right now
    QTimer::singleShot(0, this, SLOT(slotDelayedActivation()));
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
    entries.append(ModuleMenuEntry(QStringList() << "Warnings", Uid, 0, QIcon(":/inspector/images/menu-warning.png")));
    return entries;
}

AbstractPanel *WarningsModule::createPanel(int panelId)
{
    if (panelId == 0)
        return 0;
    return AbstractModule::createPanel(panelId);
}

void WarningsModule::slotDelayedActivation()
{
    emit requestActivation(tr("Automatic Warnings"));
}

void WarningsModule::slotProcessIncomingData(quint32 channel, quint32 code1, QByteArray *data)
{
    Q_UNUSED(data);
    if (channel == 0x03 && code1 == 0x01) {
        m_notification->addWarning();
        m_notification->show();
    }
}

void WarningsModule::slotNotificationClicked()
{
    // hide the Notification first
    m_notification->clearWarnings();
    m_notification->hide();

    // show our panel
    parentInstance()->makeVisible(Uid, 0);
}

void WarningsModule::slotActivate()
{
    // read the data coming from the CommServer
    connect(parentInstance()->commServer(), SIGNAL(incomingData(quint32,quint32,QByteArray*)),
            this, SLOT(slotProcessIncomingData(quint32,quint32,QByteArray*)));
}

void WarningsModule::slotDeactivate()
{
    // disconnect from the CommServer
    disconnect(parentInstance()->commServer(), 0, this, 0);

    // notify that we have been deactivated
    emit deactivated();
}