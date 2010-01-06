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

#ifndef COMMSERVER_H
#define COMMSERVER_H

#include <QObject>
#include <QByteArray>
#include <QLocalSocket>
#include <QVariant>
class QLocalServer;

namespace Inspector {

class InstanceModel;

namespace Internal {

class CommServer : public QObject
{
    Q_OBJECT

public:
    CommServer(InstanceModel *, QObject *parent = 0);
    ~CommServer();

signals:
    void incomingData(quint32 channel, quint32 code1, QByteArray *data);

private slots:
    void slotIncomingConnection();
    void slotReadConnection();
    void slotDisconnected();
    void slotConnError(QLocalSocket::LocalSocketError error);

private:
    bool processIncomingData(quint32 channel, quint32 code1, QByteArray *data);
    void addMessageToModel(int column, const QString &message);

    InstanceModel *m_model;

    QLocalServer *m_localServer;
    QLocalSocket *m_socket;
    QByteArray m_incomingData;

};

} // namespace Internal
} // namespace Inspector

#endif
