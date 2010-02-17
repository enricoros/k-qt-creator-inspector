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

#ifndef SHAREDDEBUGGER_H
#define SHAREDDEBUGGER_H

#include <QtCore/QObject>
#include <QtCore/QVariantList>

namespace Inspector {
namespace Internal {

class ProbeInjectingDebugger;

/**
  \brief Thin wrapper over ProbeInjectingDebugger that allows up to one instance

  Note: the whole SharedDebugger/ProbeInjectingDebugger stuff is done
  because creator allows only one instance of the debugger so we have
  to model some sort of (single) resource locking.
*/
class SharedDebugger : public QObject
{
    Q_OBJECT

public:
    SharedDebugger(QObject *parent = 0);

    bool acquirable() const;
    ProbeInjectingDebugger *acquireProbeInjectingDebugger();
    void releaseProbeInjectingDebugger();

signals:
    void acquirableChanged(bool);

private slots:
    void slotAcquiredDestroyed();
    void slotManagerStateChanged(int);

private:
    bool m_dmRunning;
    ProbeInjectingDebugger *m_acquired;
};

} // namespace Internal
} // namespace Inspector

#endif // SHAREDDEBUGGER_H
