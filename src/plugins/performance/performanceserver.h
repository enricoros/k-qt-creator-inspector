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
class QLocalServer;

namespace Performance {
namespace Internal {
class PerformancePane;
class PerformanceMiniWidget;

class PerformanceServer
  : public QObject
{
    Q_OBJECT

public:
    PerformanceServer(PerformancePane * view, QObject * parent = 0);
    ~PerformanceServer();

private slots:
    void slotMiniClicked();
    void slotNewConnection();
    void slotNewData();

private:
    QLocalServer * m_localServer;
    PerformancePane * m_view;
    PerformanceMiniWidget * m_mini;
};

} // namespace Internal
} // namespace Performance

#endif
