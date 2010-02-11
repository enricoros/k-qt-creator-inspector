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
#include <debugger/debuggermanager.h>

using namespace Inspector::Internal;
using namespace Debugger;

SharedDebugger::SharedDebugger(QObject *parent)
  : QObject(parent)
  , m_debuggerManager(0)
  , m_instance(0)
  , m_running(false)
{
    m_debuggerManager = DebuggerManager::instance();
    connect(m_debuggerManager, SIGNAL(stateChanged(int)), this, SLOT(slotDmStateChanged(int)));

    // sync inital state
    slotDmStateChanged(m_debuggerManager->state());
}

bool SharedDebugger::acquirable() const
{
    return !m_running && !m_instance;
}

void SharedDebugger::callProbeFunction(const QString &name, const QVariantList &args)
{
    m_debuggerManager->callFunction(name, args);
}

void SharedDebugger::slotDmStateChanged(int state)
{
    switch ((DebuggerState)state) {
    case DebuggerNotReady:
        setRunning(false);
        break;
    case EngineStarting:
    case AdapterStarting:
    case AdapterStarted:
    case AdapterStartFailed:
    case InferiorUnrunnable:
    case InferiorStarting:
    case InferiorStartFailed:
    case InferiorRunningRequested:
    case InferiorRunningRequested_Kill:
    case InferiorRunning:
    case InferiorStopping:
    case InferiorStopping_Kill:
    case InferiorStopped:
    case InferiorStopFailed:
    case InferiorShuttingDown:
    case InferiorShutDown:
    case InferiorShutdownFailed:
    case EngineShuttingDown:
        setRunning(true);
        break;
    }
}

void SharedDebugger::setRunning(bool running)
{
    if (running != m_running) {
        bool prev = acquirable();
        m_running = running;
        bool current = acquirable();
        if (prev != current)
            emit acquirableChanged(current);
    }
}

void SharedDebugger::setInstance(Instance *instance)
{
    if (m_instance != instance) {
        bool prev = acquirable();
        m_instance = instance;
        bool current = acquirable();
        if (prev != current)
            emit acquirableChanged(current);
    }

    // shut down debugger when released
    if (!instance && m_running)
        m_debuggerManager->exitDebugger();
}
