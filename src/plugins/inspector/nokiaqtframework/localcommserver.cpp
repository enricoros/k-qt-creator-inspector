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

#include "localcommserver.h"
#include "nokiaqtinspectionmodel.h"
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
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

using namespace Inspector::Internal;

LocalCommServer::LocalCommServer(NokiaQtInspectionModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_socket(0)
{
    // init model data
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 0, false);
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 1, QString());
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 2, false);
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 3, false);
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 4, QString());
  //m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 5, --spare--);
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 6, "messages");
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 7, "errors");
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 8, "comm");

    // create local server and listen for a connection
    m_localServer = new QLocalServer;
    int uniqueCode = QDateTime::currentDateTime().toTime_t() + QTime::currentTime().msec() + (qrand() % 1000);
    bool canListen = m_localServer->listen(QString("creator_insp_%1").arg(uniqueCode));
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 1, m_localServer->serverName());
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 2, canListen);
    if (!canListen)
        return;
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(slotIncomingConnection()));
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 0, true);
}

LocalCommServer::~LocalCommServer()
{
    delete m_localServer;
}

void LocalCommServer::slotIncomingConnection()
{
    while (m_localServer->hasPendingConnections()) {
        QLocalSocket * nextConnection = m_localServer->nextPendingConnection();
        if (m_socket) {
            QMessageBox::information(0, tr("Inspector-Probe Connection"), tr("A client is already connected and another is trying to... something wrong?"));
            continue;
        }

        // set the connection
        m_socket = nextConnection;
        m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 3, true);
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotReadConnection()));
        connect(m_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
        connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(slotConnError(QLocalSocket::LocalSocketError)));
    }
}

void LocalCommServer::slotReadConnection()
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
        quint32 channel, code1;
        QByteArray payload;
        bool decoded = Inspector::Internal::demarshallMessage(m_incomingData, &channel, &code1, &payload);
        if (!decoded)
            qWarning() << "CommServer::slotReadConnection: error decoding a message";
        else
            processIncomingData(channel, code1, &payload);

        // remove chunk from incoming data
        m_incomingData.remove(0, chunkSize);
    }
}

void LocalCommServer::slotDisconnected()
{
    m_socket->deleteLater();
    m_socket = 0;
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 3, false);
}

void LocalCommServer::slotConnError(QLocalSocket::LocalSocketError error)
{
    m_model->setItemValue(NokiaQtInspectionModel::CommServer_Row, 3, false);

    // Log Error
    addMessageToModel(7, tr("error %1: %2").arg(error).arg(m_localServer->errorString()));
}

bool LocalCommServer::processIncomingData(quint32 channel, quint32 code1, QByteArray *data)
{
    // Log Communication
    addMessageToModel(8, tr("%1:%2 (%3)").arg(channel).arg(code1).arg(data->size()));

    // Log Messages / Errors
    if (channel == Inspector::Internal::Channel_General) {
        if (code1 == 0x00) {
            // TODO: handle the just received 'probe startup'
        } else if (code1 == 0x01) {
            addMessageToModel(6, QString(*data));
        } else if (code1 == 0x02) {
            addMessageToModel(7, QString(*data));
        }
    }

    // tell all the listening modules about this data
    emit incomingData(channel, code1, data);

    return true;

    //qWarning() << "unhandled message" << channel << code1 << *data;
    //return false;
}

void LocalCommServer::addMessageToModel(int column, const QString &message)
{
    QStandardItem *parentItem = m_model->item(NokiaQtInspectionModel::CommServer_Row, column);
    if (parentItem)
        parentItem->appendRow(new QStandardItem(message));
}
