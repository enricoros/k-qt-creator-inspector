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

#ifndef INSPECTORINSTANCE_H
#define INSPECTORINSTANCE_H

#include <QObject>
#include <QList>
#include <QVariantList>

namespace Inspector {
class CommServer;
namespace Internal {
class NotificationWidget;
class InspectorPlugin;
class InspectorFrame;
class ProbeController;
}

class Q_DECL_EXPORT InspectorInstance : public QObject
{
    Q_OBJECT

public:
    InspectorInstance(QObject *parent = 0);
    ~InspectorInstance();

    CommServer *commServer() const;
    Internal::ProbeController *probeController() const;

    int probeActivationFlags() const; //TEMP relocate

    // ### TEMP emits default server's debuggerCallFunction
    void commCallFunction(const QString & name, QVariantList args = QVariantList());

    // externally set information
    void setDebugging(bool on);
    bool debugging() const;
    void setEnabled(bool enabled);
    bool enabled() const;
    void setDebugPaint(bool checked);

public slots:
    void slotShowInformation();
    void slotShowProbeMode();

private slots:
    void slotNewWarnings(int count);

private:
    CommServer * m_commServer;
    Internal::NotificationWidget *m_notification;
    Internal::ProbeController *m_probeController;
    bool m_enabled;
    bool m_debugPaintFlag;
    bool m_sDebugging;
};

} // namespace Inspector

#endif // INSPECTORINSTANCE_H
