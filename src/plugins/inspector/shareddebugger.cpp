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
#include "inspectorrunner.h"

#include <debugger/debuggermanager.h>
#include <projectexplorer/applicationrunconfiguration.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>

using namespace Inspector::Internal;

SharedDebugger::SharedDebugger(QObject *parent)
  : QObject(parent)
  , m_debuggerManager(0)
  , m_acquired(false)
  , m_running(false)
{
    m_debuggerManager = Debugger::DebuggerManager::instance();
    connect(m_debuggerManager, SIGNAL(stateChanged(int)), this, SLOT(slotDmStateChanged(int)));

    // sync inital state
    slotDmStateChanged(m_debuggerManager->state());
}

bool SharedDebugger::acquirable() const
{
    return !m_running && !m_acquired;
}

bool SharedDebugger::acquire()
{
    if (!acquirable())
        return false;
    m_acquired = true;
    emit acquirableChanged(true);
    return true;
}

void SharedDebugger::release()
{
    if (!m_acquired)
        return;
    m_acquired = false;

    // shut down debugger when released
    if (m_running)
        m_debuggerManager->exitDebugger();
}

bool SharedDebugger::startPidAttach(quint64 pid)
{
    const Debugger::DebuggerStartParametersPtr sp(new Debugger::DebuggerStartParameters);
    sp->attachPID = pid;
    sp->startMode = Debugger::AttachExternal;
    ProjectExplorer::RunControl *runControl = new InspectorRunControl(m_debuggerManager, sp);
    ProjectExplorer::ProjectExplorerPlugin::instance()->startRunControl(runControl, ProjectExplorer::Constants::DEBUGMODE);
    return true;
}

bool SharedDebugger::startRunConfiguration(ProjectExplorer::RunConfiguration *runConfiguration)
{
    ProjectExplorer::LocalApplicationRunConfiguration *localAppRc =
            qobject_cast<ProjectExplorer::LocalApplicationRunConfiguration *>(runConfiguration);
    QTC_ASSERT(localAppRc, return false);
    ProjectExplorer::RunControl *runControl = new InspectorRunControl(m_debuggerManager, localAppRc);
    ProjectExplorer::ProjectExplorerPlugin::instance()->startRunControl(runControl, ProjectExplorer::Constants::DEBUGMODE);
    return true;
}

void SharedDebugger::callProbeFunction(const QString &name, const QVariantList &args)
{
    m_debuggerManager->callFunction(name, args);
}

void SharedDebugger::slotDmStateChanged(int state)
{
    switch ((Debugger::DebuggerState)state) {
    case Debugger::DebuggerNotReady:
        setRunning(false);
        break;
    case Debugger::EngineStarting:
    case Debugger::AdapterStarting:
    case Debugger::AdapterStarted:
    case Debugger::AdapterStartFailed:
    case Debugger::InferiorUnrunnable:
    case Debugger::InferiorStarting:
    case Debugger::InferiorStartFailed:
    case Debugger::InferiorRunningRequested:
    case Debugger::InferiorRunningRequested_Kill:
    case Debugger::InferiorRunning:
    case Debugger::InferiorStopping:
    case Debugger::InferiorStopping_Kill:
    case Debugger::InferiorStopped:
    case Debugger::InferiorStopFailed:
    case Debugger::InferiorShuttingDown:
    case Debugger::InferiorShutDown:
    case Debugger::InferiorShutdownFailed:
    case Debugger::EngineShuttingDown:
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
