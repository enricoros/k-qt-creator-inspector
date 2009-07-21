/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Started on 14 Jul 2009 by root.                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PERFORMANCEMANAGER_H
#define PERFORMANCEMANAGER_H

#include <QObject>
#include <QList>

namespace Performance {
class PerformanceServer;
namespace Internal {
class PerformanceNotification;
class PerformancePlugin;
class PerformanceWindow;
}

class Q_DECL_EXPORT PerformanceManager : public QObject
{
    Q_OBJECT

public:
    PerformanceManager(Internal::PerformancePlugin *plugin, QObject *parent = 0);
    ~PerformanceManager();

    static PerformanceManager * instance();

    bool enabled() const;

    Internal::PerformanceWindow *defaultWindow() const;
    PerformanceServer *defaultServer() const;
    int activationFlags() const; //TEMP relocate

public slots:
    void slotSetEnabled(bool enabled);
    void slotShowInformation();
    void slotShowRuntimeMode();
    void slotPaintingTemperature();

private slots:
    void slotNewWarnings(int count);

private:
    static PerformanceManager *s_instance;
    QList<PerformanceServer*> m_servers;
    Internal::PerformancePlugin *m_plugin;
    Internal::PerformanceWindow *m_window;
    Internal::PerformanceNotification *m_notification;
    bool m_enabled;
};

} // namespace Performance

#endif // PERFORMANCEMANAGER_H
