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

#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>

#include <QLocalServer>
#include <QLocalSocket>

using namespace Performance::Internal;

PerformanceServer::PerformanceServer(PerformancePane * view, QObject * parent)
    : QObject(parent)
    , m_view(view)
    , m_mini(0)
{
    m_localServer = new QLocalServer(this);
    m_localServer->listen("performance1");
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
}

PerformanceServer::~PerformanceServer()
{
    delete m_localServer;
    delete m_mini;
}

void PerformanceServer::slotMiniClicked()
{
    m_view->popup(true);
}

void PerformanceServer::slotNewConnection()
{
    while (m_localServer->hasPendingConnections()) {
        QLocalSocket * sock = m_localServer->nextPendingConnection();
        connect(sock, SIGNAL(readyRead()), this, SLOT(slotNewData()));
    }
}

void PerformanceServer::slotNewData()
{
    QLocalSocket * sock = static_cast<QLocalSocket *>(sender());
    QByteArray data = sock->readAll();

    m_view->addString(data);

    if (!m_mini) {
        m_mini = new PerformanceMiniWidget;
        connect(m_mini, SIGNAL(clicked()), this, SLOT(slotMiniClicked()));
        Core::ICore::instance()->modeManager()->addWidget(m_mini);
    }
    m_mini->addWarning();
}
