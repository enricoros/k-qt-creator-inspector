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
#include "performanceinformation.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>

#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QDateTime>
#include <QTime>

using namespace Performance;

PerformanceServer::PerformanceServer(QObject * parent)
    : QObject(parent)
    , m_socket(0)
    , m_sEnabled(false)
    , m_sDebugging(false)
    , m_sHelpers(false)
    , m_sInjected(false)
    , m_sConnected(false)
{
    m_localServer = new QLocalServer;
    int uniqueCode = QDateTime::currentDateTime().toTime_t() + QTime::currentTime().msec() + (qrand() % 1000);
    if (!m_localServer->listen(QString("creator_perf_%1").arg(uniqueCode))) {
        // TODO: make this a state, disable the plugin, remove the messagebox
        QMessageBox::information(0, tr("Performance Plugin Connection"), tr("The Performance Plugin server can't be started\nerror: %1").arg(m_localServer->errorString()));
        return;
    }
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(slotIncomingConnection()));
    m_sEnabled = true;
}

PerformanceServer::~PerformanceServer()
{
    delete m_localServer;
}

bool PerformanceServer::enabled() const
{
    return m_sEnabled;
}

QString PerformanceServer::serverName() const
{
    return m_localServer->serverName();
}

void PerformanceServer::setDebugging(bool on)
{
    m_sDebugging = on;
}

void PerformanceServer::setHelpersPresent(bool on)
{
    m_sHelpers = on;
}

void PerformanceServer::setHelpersInjected(bool on)
{
    m_sInjected = on;
}

void PerformanceServer::slotIncomingConnection()
{
    while (m_localServer->hasPendingConnections()) {
        QLocalSocket * nextConnection = m_localServer->nextPendingConnection();
        if (m_socket) {
            QMessageBox::information(0, tr("Performance Plugin Connection"), tr("A client is already connected and another is trying to... something wrong?"));
            continue;
        }

        // set the connection
        m_socket = nextConnection;
        m_sConnected = true;
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotReadConnection()));
        connect(m_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
        connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(slotConnError(QLocalSocket::LocalSocketError)));
    }
}

void PerformanceServer::slotReadConnection()
{
    QByteArray data = m_socket->readAll();

    // TODO demux data here

    emit newString(data);

    emit newWarnings(1);
}

void PerformanceServer::slotDisconnected()
{
    m_socket->deleteLater();
    m_socket = 0;
    m_sConnected = false;
}

#include <QDebug>
void PerformanceServer::slotConnError(QLocalSocket::LocalSocketError error)
{
    qWarning() << "Performance Plugin Debug: error" << error;
}
