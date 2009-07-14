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
namespace Internal {
    class PerformancePane;
    class PerformanceMiniWidget;
    class PerformanceInformation;
}

class Q_DECL_EXPORT PerformanceServer
  : public QObject
{
    Q_OBJECT

public:
    PerformanceServer(Internal::PerformancePane * view, QObject * parent = 0);
    ~PerformanceServer();

    // TODO 1 server per Debuggee

    bool enabled() const;
    QString serverName() const;

    Internal::PerformanceInformation * createInformationWidget() const;

    // externally set information
    void setDebugging(bool on);
    void setHelpersPresent(bool on);
    void setHelpersInjected(bool on);

private slots:
    void slotMiniClicked();
    void slotIncomingConnection();
    void slotReadConnection();
    void slotDisconnected();
    void slotConnError(QLocalSocket::LocalSocketError error);

private:
    QLocalServer * m_localServer;
    QLocalSocket * m_socket;
    Internal::PerformancePane * m_view;
    Internal::PerformanceMiniWidget * m_mini;

    bool m_sEnabled;
    bool m_sRunning;
    bool m_sHelpers;
    bool m_sInjected;
    bool m_sConnected;
};

} // namespace Performance

#endif
