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

#include "commserver.h"
#include "instance.h"
#include "instancemodel.h"
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <QDebug>
#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QDateTime>
#include <QTime>

// for demarshalling communication with The Probe
#include "../../../share/qtcreator/gdbmacros/perfunction.h"

using namespace Inspector;
using namespace Inspector::Internal;

/* == InstanceModel Usage ==
Row 'CommServer_Row': Communication Server
  0: server enabled
  1: server local name
  2: server listening
  3: probe connected
  4: probe info
*/

CommServer::CommServer(InstanceModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_socket(0)
{
    // init model data
    m_model->setValue(InstanceModel::CommServer_Row, 0, false);
    m_model->setValue(InstanceModel::CommServer_Row, 1, QString());
    m_model->setValue(InstanceModel::CommServer_Row, 2, false);
    m_model->setValue(InstanceModel::CommServer_Row, 3, false);
    m_model->setValue(InstanceModel::CommServer_Row, 4, QString());

    // create local server and listen for a connection
    m_localServer = new QLocalServer;
    int uniqueCode = QDateTime::currentDateTime().toTime_t() + QTime::currentTime().msec() + (qrand() % 1000);
    bool canListen = m_localServer->listen(QString("creator_insp_%1").arg(uniqueCode));
    m_model->setValue(InstanceModel::CommServer_Row, 1, m_localServer->serverName());
    m_model->setValue(InstanceModel::CommServer_Row, 2, canListen);
    if (!canListen)
        return;
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(slotIncomingConnection()));
    m_model->setValue(InstanceModel::CommServer_Row, 0, true);
}

CommServer::~CommServer()
{
    delete m_localServer;
}

void CommServer::slotIncomingConnection()
{
    while (m_localServer->hasPendingConnections()) {
        QLocalSocket * nextConnection = m_localServer->nextPendingConnection();
        if (m_socket) {
            QMessageBox::information(0, tr("Inspector-Probe Connection"), tr("A client is already connected and another is trying to... something wrong?"));
            continue;
        }

        // set the connection
        m_socket = nextConnection;
        m_model->setValue(InstanceModel::CommServer_Row, 3, true);
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotReadConnection()));
        connect(m_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
        connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(slotConnError(QLocalSocket::LocalSocketError)));
    }
}

void CommServer::slotReadConnection()
{
    m_incomingData += m_socket->readAll();

    // drop overgrowing internal buffer
    if (m_incomingData.size() > 10000000) {
        qWarning() << "CommServer::slotReadConnection: dropping XXL message from the Probe";
        m_incomingData = QByteArray();
        return;
    }

    // process all incoming information
    while (!m_incomingData.isEmpty()) {

        // partial/incomplete chunks: save for later
        quint32 chunkSize = Inspector::Internal::messageLength(m_incomingData);
        if (!chunkSize || m_incomingData.length() < (int)chunkSize)
            break;

        // decode chunk
        quint32 code1, code2;
        QByteArray payload;
        bool decoded = Inspector::Internal::demarshallMessage(m_incomingData, &code1, &code2, &payload);
        if (!decoded)
            qWarning() << "CommServer::slotReadConnection: error decoding a message";
        else
            processIncomingData(code1, code2, &payload);

        // remove chunk from incoming data
        m_incomingData.remove(0, chunkSize);
    }
}

void CommServer::slotDisconnected()
{
    m_socket->deleteLater();
    m_socket = 0;
    m_model->setValue(InstanceModel::CommServer_Row, 3, false);
}

void CommServer::slotConnError(QLocalSocket::LocalSocketError error)
{
    m_model->setValue(InstanceModel::CommServer_Row, 3, false);
    qWarning() << "CommServer::slotConnError: error" << error;
}

bool CommServer::processIncomingData(quint32 code1, quint32 code2, QByteArray * data)
{
    // 1. Service
    if (code1 == 0x01) {
        ///window->serviceText->appendPlainText(QString::number(code2) + " " + QString(*data));

        // 1.1 begin
        if (code2 == 0x01) {
            return true;
        }
    }

    // 2. Generic Comm
    if (code1 == 0x02) {

        // 2.1 text messages
        if (code2 == 0x01) {
            ///window->messagesText->appendHtml("<font color='#800'>" + *data + "</font>");
            return true;
        }

        // 2.2 warning messages
        if (code2 == 0x02) {
            ///window->errorsText->appendHtml("<font color='#800'>" + *data + "</font>");
            emit newWarnings(1);
            return true;
        }

        // 2.3 percent
        if (code2 == 0x03) {
            int percent = qBound(0, QString(*data).toInt(), 100);
            qWarning("progress percent: %d", percent);
            // ### TODO
            ///window->progressBar->setValue(percent);
            ///window->progressLabel->setVisible(percent < 100);
            ///window->progressBar->setVisible(percent < 100);
            return true;
        }

        // 2.4 qimages
        if (code2 == 0x04) {
            qWarning("qimage received");
            QDataStream dataReader(data, QIODevice::ReadOnly);
            QSize size;
            quint32 format;
            QByteArray contents;
            dataReader >> size;
            dataReader >> format;
            dataReader >> contents;
            QImage image((uchar *)contents.data(), size.width(), size.height(), (QImage::Format)format);

            QLabel * label = new QLabel();
            label->setWindowIcon(QIcon(":/inspector/images/menu-icon.png"));
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
