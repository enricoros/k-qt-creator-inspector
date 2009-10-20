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

#ifndef PERFORMANCESERVER_H
#define PERFORMANCESERVER_H

#include <QObject>
#include <QByteArray>
#include <QLocalSocket>
#include <QVariant>
class QLocalServer;

namespace Performance {
namespace Internal {
class PerformanceWindow;
}

class Q_DECL_EXPORT PerformanceServer : public QObject
{
    Q_OBJECT

public:
    PerformanceServer(QObject * parent = 0);
    ~PerformanceServer();

    bool enabled() const; //CHANGE ME
    QString serverName() const;

    bool processIncomingData(quint32 code1, quint32 code2, QByteArray * data);

    // externally set information
    void setDebugging(bool on);
    void setHelpersPresent(bool on);
    void setHelpersInjected(bool on);

signals:
    void newWarnings(int count);
    void debuggerCallFunction(const QString & name, QVariantList args = QVariantList());

private slots:
    void slotIncomingConnection();
    void slotReadConnection();
    void slotDisconnected();
    void slotConnError(QLocalSocket::LocalSocketError error);

private:
    friend class PerformanceManager;
    friend class Internal::PerformanceWindow;
    QLocalServer *m_localServer;
    QLocalSocket *m_socket;
    QByteArray m_incomingData;

    bool m_sEnabled;
    bool m_sDebugging;
    bool m_sHelpers;
    bool m_sInjected;
    bool m_sConnected;
};

} // namespace Performance

#endif