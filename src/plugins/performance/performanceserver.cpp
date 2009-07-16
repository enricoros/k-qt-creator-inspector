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
#include "performancemanager.h"
#include "performancewindow.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>

#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QDateTime>
#include <QTime>

// for demarshalling communication with The Probe
#include "../../../share/qtcreator/gdbmacros/perfunction.h"

using namespace Performance;
using namespace Performance::Internal;

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

#include <QImage>
bool PerformanceServer::processIncomingData(quint32 code1, quint32 code2, QByteArray * data)
{
    PerformanceWindow * window = PerformanceManager::instance()->defaultWindow();

    // 1. Service
    if (code1 == 0x01) {
        window->serviceText->appendPlainText(QString::number(code2) + " " + QString(*data));

        // 1.1 begin
        if (code2 == 0x01) {
            return true;
        }
    }

    // 2. Generic Comm
    if (code1 == 0x02) {

        // 2.1 text messages
        if (code2 == 0x01) {
            window->messagesText->appendHtml("<font color='#800'>" + *data + "</font>");
            return true;
        }

        // 2.2 warning messages
        if (code2 == 0x02) {
            window->errorsText->appendHtml("<font color='#800'>" + *data + "</font>");
            emit newWarnings(1);
            return true;
        }

        // 2.3 percent
        if (code2 == 0x03) {
            int percent = qBound(0, QString(*data).toInt(), 100);
            window->progressBar->setValue(percent);
            window->progressLabel->setVisible(percent < 100);
            window->progressBar->setVisible(percent < 100);
            return true;
        }

        // 2.4 qimages
        if (code2 == 0x04) {
            QDataStream dataReader(data, QIODevice::ReadOnly);
            QSize size;
            quint32 format;
            QByteArray contents;
            dataReader >> size;
            dataReader >> format;
            dataReader >> contents;
            QImage image((uchar *)contents.data(), size.width(), size.height(), (QImage::Format)format);

            QLabel * label = new QLabel();
            label->setWindowIcon(QIcon(":/performance/images/menu-icon.png"));
            label->setWindowTitle(tr("Image from The Probe"));
            label->setFixedSize(size);
            label->setPixmap(QPixmap::fromImage(image));
            label->show();
            return true;
        }
    }

    // 3. Event Loop Information
    if (code1 == 0x03) {
        // 3.1. Timing
        if (code2 == 0x01) {
            //qWarning("timing");
            return false;
        }
    }

    // warn
    qWarning() << "unhandled message" << code1 << code2 << *data;
    return false;
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
    m_incomingData += m_socket->readAll();

    // drop overgrowing internal buffer
    if (m_incomingData.size() > 10000000) {
        qWarning() << "PerformanceServer::slotReadConnection: dropping XXL message from the Probe";
        m_incomingData = QByteArray();
        return;
    }

    // process all incoming information
    while (!m_incomingData.isEmpty()) {

        // partial/incomplete chunks: save for later
        quint32 chunkSize = Performance::Internal::messageLength(m_incomingData);
        if (!chunkSize || m_incomingData.length() < (int)chunkSize)
            break;

        // decode chunk
        quint32 code1, code2;
        QByteArray payload;
        bool decoded = Performance::Internal::demarshallMessage(m_incomingData, &code1, &code2, &payload);
        if (!decoded)
            qWarning() << "PerformanceServer::slotReadConnection: error decoding a message";
        else
            processIncomingData(code1, code2, &payload);

        // remove chunk from incoming data
        m_incomingData.remove(0, chunkSize);
    }
}

void PerformanceServer::slotDisconnected()
{
    m_socket->deleteLater();
    m_socket = 0;
    m_sConnected = false;
}

void PerformanceServer::slotConnError(QLocalSocket::LocalSocketError error)
{
    qWarning() << "PerformanceServer::slotConnError: error" << error;
}
