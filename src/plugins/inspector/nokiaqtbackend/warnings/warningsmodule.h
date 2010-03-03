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

#include "ibackendmodule.h"
#include "ibackendtask.h"

namespace Inspector {
namespace Internal {

// constants
const int UID_MODULE_WARNINGS = 3;

class LocalCommServer;
class NokiaQtBackend;
class NotificationWidget;
class WarningsTask;

/**
  \brief Automatic/Manual warnings on certain events
*/
class WarningsModule : public IBackendModule
{
    Q_OBJECT

public:
    WarningsModule(NokiaQtBackend *, QObject *parent = 0);
    ~WarningsModule();

    // ::IBackendModule
    int uid() const { return UID_MODULE_WARNINGS; }
    QString name() const;
    ModuleMenuEntries menuEntries() const;

private slots:
    void slotBackendConnected(bool connected);
    void slotNotificationClicked();
    void slotTaskAddWarning();

private:
    NokiaQtBackend *m_nqBackend;
    NotificationWidget *m_notification;
};

/**
  \brief Filters add incoming communication for warnings
*/
class WarningsTask : public IBackendTask
{
    Q_OBJECT

public:
    WarningsTask(NokiaQtBackend *, QObject *parent = 0);

    // ::IBackendTask
    QString displayName() const;
    void activateTask();
    void deactivateTask();

signals:
    void addWarning();

private slots:
    void slotProcessIncomingData(quint32 channel, quint32 code1, QByteArray *data);

private:
    LocalCommServer *m_commServer;
};

} // namespace Internal
} // namespace Inspector

#endif // WARNINGSMODULE_H
