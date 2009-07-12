/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Started on 10 Jul 2009 by root.                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "performanceserver.h"
#include "performancepane.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>

#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>

using namespace Performance::Internal;

PerformanceServer::PerformanceServer(PerformancePane * view, QObject * parent)
    : QObject(parent)
    , m_socket(0)
    , m_view(view)
    , m_mini(0)
{
    m_localServer = new QLocalServer;
    m_localServer->listen("performance-" + QString::number(qrand()));
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(slotIncomingConnection()));
}

PerformanceServer::~PerformanceServer()
{
    delete m_localServer;
    delete m_mini;
}

QString PerformanceServer::serverName() const
{
    return m_localServer->serverName();
}

void PerformanceServer::slotMiniClicked()
{
    // switch to full screen performance view
    Core::ICore::instance()->modeManager()->activateMode(Core::Constants::MODE_OUTPUT);
    m_view->popup(true);
}

void PerformanceServer::slotIncomingConnection()
{
    while (m_localServer->hasPendingConnections()) {
        if (m_socket) {
            QMessageBox::information(0, tr("Performance Plugin Connection"), tr("A client is already connected and another is trying to... something wrong?"));
            continue;
        }
        m_socket = m_localServer->nextPendingConnection();
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotReadConnection()));
    }
}

void PerformanceServer::slotReadConnection()
{
    QByteArray data = m_socket->readAll();

    // TODO demux data here

    m_view->addString(data);

    // show mini-widget and add a warning sign
    if (!m_mini) {
        m_mini = new PerformanceMiniWidget;
        connect(m_mini, SIGNAL(clicked()), this, SLOT(slotMiniClicked()));
        Core::ICore::instance()->modeManager()->addWidget(m_mini);
    }
    m_mini->addWarning();
}
