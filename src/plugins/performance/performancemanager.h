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

#ifndef PERFORMANCEMANAGER_H
#define PERFORMANCEMANAGER_H

#include <QObject>
#include <QList>
#include <QVariantList>

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

    // ### TEMP emits default server's debuggerCallFunction
    void defaultServerCallFunction(const QString & name, QVariantList args = QVariantList());

public slots:
    void slotSetEnabled(bool enabled);
    void slotShowInformation();
    void slotShowProbeMode();
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
