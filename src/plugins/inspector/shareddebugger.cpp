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

#include "shareddebugger.h"
#include "probeinjectingdebugger.h"

#include <debugger/debuggermanager.h>

using namespace Inspector::Internal;

SharedDebugger::SharedDebugger(QObject *parent)
  : QObject(parent)
  , m_dmRunning(false)
  , m_acquired(0)
{
    Debugger::DebuggerManager *manager = Debugger::DebuggerManager::instance();
    connect(manager, SIGNAL(stateChanged(int)), this, SLOT(slotManagerStateChanged(int)));
    slotManagerStateChanged(manager->state());
}

bool SharedDebugger::acquirable() const
{
    return !m_dmRunning && !m_acquired;
}

ProbeInjectingDebugger *SharedDebugger::acquireProbeInjectingDebugger()
{
    if (!acquirable())
        return 0;
    m_acquired = new ProbeInjectingDebugger(this);
    connect(m_acquired, SIGNAL(destroyed()), this, SLOT(slotAcquiredDestroyed()));
    emit acquirableChanged(acquirable());
    return m_acquired;
}

void SharedDebugger::releaseProbeInjectingDebugger()
{
    if (!m_acquired) {
        qWarning("SharedDebugger::releaseProbeInjectingDebugger: nothing to release!");
        return;
    }
    disconnect(m_acquired, 0, this, 0);
    m_acquired->closeInspection();
    m_acquired->deleteLater();
    m_acquired = 0;
    emit acquirableChanged(acquirable());
}

void SharedDebugger::slotAcquiredDestroyed()
{
    qWarning("SharedDebugger::slotAcquiredDestroyed: don't delete the ProbeInjectingDebugger directly!");
    m_acquired = 0;
    emit acquirableChanged(acquirable());
}

void SharedDebugger::slotManagerStateChanged(int state)
{
    bool running = state != Debugger::DebuggerNotReady;
    if (running != m_dmRunning) {
        m_dmRunning = running;
        emit acquirableChanged(acquirable());
    }
}
