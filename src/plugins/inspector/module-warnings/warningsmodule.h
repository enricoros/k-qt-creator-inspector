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

#ifndef WARNINGSMODULE_H
#define WARNINGSMODULE_H

#include "iframeworkmodule.h"

namespace Inspector {
namespace Internal {

class NotificationWidget;

class WarningsModule : public IFrameworkModule
{
    Q_OBJECT

public:
    WarningsModule(Instance *, QObject *parent = 0);
    ~WarningsModule();

    // ::IFrameworkModule
    enum { Uid = 0x03 };
    int uid() const { return Uid; }
    QString name() const;
    ModuleMenuEntries menuEntries() const;
    AbstractPanel *createPanel(int panelId);

private slots:
    void slotDelayedActivation();
    void slotProcessIncomingData(quint32 channel, quint32 code1, QByteArray *data);
    void slotNotificationClicked();

    // ::IFrameworkModule
    void slotActivate();
    void slotDeactivate();

private:
    Internal::NotificationWidget *m_notification;
};

} // namespace Internal
} // namespace Inspector

#endif // WARNINGSMODULE_H
