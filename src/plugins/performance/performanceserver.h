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
#include <QLocalSocket>
class QLocalServer;

namespace Performance {

class Q_DECL_EXPORT PerformanceServer : public QObject
{
    Q_OBJECT

public:
    PerformanceServer(QObject * parent = 0);
    ~PerformanceServer();

    bool enabled() const; //CHANGE ME
    QString serverName() const;

    // externally set information
    void setDebugging(bool on);
    void setHelpersPresent(bool on);
    void setHelpersInjected(bool on);

signals:
    void newString(const QString & string);
    void newWarnings(int count);

private slots:
    void slotIncomingConnection();
    void slotReadConnection();
    void slotDisconnected();
    void slotConnError(QLocalSocket::LocalSocketError error);

private:
    friend class PerformanceManager;
    QLocalServer * m_localServer;
    QLocalSocket * m_socket;

    bool m_sEnabled;
    bool m_sDebugging;
    bool m_sHelpers;
    bool m_sInjected;
    bool m_sConnected;
};

} // namespace Performance

#endif
