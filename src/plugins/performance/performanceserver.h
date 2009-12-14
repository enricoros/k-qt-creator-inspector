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
