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

#include "probeinjectingdebugger.h"
#include "inspectorrunner.h"

#include <debugger/debuggermanager.h>
#include <projectexplorer/applicationrunconfiguration.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>

#include <QtCore/QTimer>

using namespace Inspector::Internal;

#define PRECONDITION QTC_ASSERT(m_inspectorRunControl, return)

//
// InspectingDebugger
//
ProbeInjectingDebugger::ProbeInjectingDebugger(QObject *parent)
  : QObject(parent)
  , m_debuggerManager(0)
  , m_inspectorRunControl(0)
  , m_runDelayTimer(0)
{
    m_debuggerManager = Debugger::DebuggerManager::instance();
}

ProbeInjectingDebugger::~ProbeInjectingDebugger()
{
    QTC_ASSERT(!m_inspectorRunControl, /**/);
}

bool ProbeInjectingDebugger::setInspectionTarget(const InspectionTarget &target, const QString &localServerName)
{
    QTC_ASSERT(!m_inspectorRunControl, return false);
    QTC_ASSERT(m_debuggerManager->state() == Debugger::DebuggerNotReady, return false);

    m_inspectorRunControl = 0;
    switch (target.type) {
    case InspectionTarget::StartLocalRunConfiguration:
        m_inspectorRunControl = new InspectorRunControl(m_debuggerManager, target.runConfiguration);
        break;

    case InspectionTarget::AttachToPid: {
        const Debugger::DebuggerStartParametersPtr sp(new Debugger::DebuggerStartParameters);
        sp->attachPID = target.pid;
        sp->startMode = Debugger::AttachExternal;
        m_inspectorRunControl = new InspectorRunControl(m_debuggerManager, sp);
        } break;

    default:
    case InspectionTarget::HijackRunControl:
    case InspectionTarget::HijackDebuggerRunControl:
        qWarning("SharedDebugger::startTarget: Hijack* not implemented");
        return false;
    }

    m_target = target;

    QTC_ASSERT(!localServerName.isNull(), return false);
    m_inspectorRunControl->setInspectorParams(localServerName, 0);

    initInspection();
    return true;
}

void ProbeInjectingDebugger::closeInspection()
{
    if (m_inspectorRunControl) {
        m_debuggerManager->exitDebugger();
        uninitInspection();
    }
}

bool ProbeInjectingDebugger::inspecting() const
{
    return m_inspectorRunControl;
}

void ProbeInjectingDebugger::callProbeFunction(const QString &name, const QVariantList &args)
{
    PRECONDITION;
    m_debuggerManager->callFunction(name, args);
}

void ProbeInjectingDebugger::slotRunControlStarted()
{
    emit inspectionStarted();
}

void ProbeInjectingDebugger::slotRunControlFinished()
{
    uninitInspection();
}

void ProbeInjectingDebugger::slotRunControlDestroyed()
{
    QTC_ASSERT(!m_inspectorRunControl, /**/);
}

// from here on, avoid typing Debugger:: ..
using namespace Debugger;

void ProbeInjectingDebugger::slotDebuggerStateChanged(int nextState)
{
    PRECONDITION;
    m_prevState = m_state;
    m_state = nextState;
    qWarning("changed to %s", DebuggerManager::stateName(m_state));

    // look for inferior presence
    if (m_state == InferiorStarting) {
        m_sHaveInferior = true;
        return;
    } else if (m_state == InferiorShuttingDown) {
        m_sHaveInferior = false;
        return;
    }
    if (!m_sHaveInferior)
        return;

    /* In case of 'Attach to PID', the debugger loads the inferior (InferiorStarting)
       and immediately stops it, injecting the code too.

       Here we detect the case and start running it back.
    */
    if (m_target.type == InspectionTarget::AttachToPid && !m_sQuirkDone) {
        if (m_state == InferiorStopped && m_prevState == InferiorStarting) {
            m_sQuirkDone = true;
            // TODO: find a more predictable way to start the inferior
            QTimer::singleShot(500, this, SLOT(slotDebuggerStartInferior()));
            return;
        }
    }

    /* In case of a normal 'Run', after InferiorStarting there is a RunRequest
       immediately followed by a stop with some commands being done (...Requested
        > ..Running > ..Stopping > ..Stopped) but the debugging helpers activation
        code is not called.

        This may be because it's not a stop request, but just a command execution
        (break at main) that performs a light stop before running the command.

        Here we detect the case, issue a real stop and then a real start.
    */
    if (m_target.type == InspectionTarget::StartLocalRunConfiguration && !m_sQuirkDone) {
        if (m_state == InferiorRunning) {
            if (!m_runDelayTimer) {
                m_runDelayTimer = new QTimer(this);
                m_runDelayTimer->setSingleShot(true);
                connect(m_runDelayTimer, SIGNAL(timeout()), this, SLOT(slotDebuggerRestartInferior()));
            }
            m_runDelayTimer->start(500);
        }
        if (m_sManuallyStopped && m_state == InferiorStopped) {
            m_sQuirkDone = true;
            QTimer::singleShot(500, this, SLOT(slotDebuggerStartInferior()));
        }
    }
}

void ProbeInjectingDebugger::slotDebuggerStartInferior()
{
    m_debuggerManager->continueExec();
}

void ProbeInjectingDebugger::slotDebuggerRestartInferior()
{
    if (m_debuggerManager->state() != InferiorRunning) {
        qWarning("ProbeInjectingDebugger::slotDebuggerRestartInferior: I wont't stop while in state '%s' because the program will close",
                 DebuggerManager::stateName(m_debuggerManager->state()));
        return;
    }
    m_sManuallyStopped = true;
    m_debuggerManager->interruptDebuggingRequest();
}

void ProbeInjectingDebugger::initInspection()
{
    connect(m_inspectorRunControl, SIGNAL(started()), this, SLOT(slotRunControlStarted()));
    connect(m_inspectorRunControl, SIGNAL(finished()), this, SLOT(slotRunControlFinished()));
    connect(m_inspectorRunControl, SIGNAL(destroyed()), this, SLOT(slotRunControlDestroyed()));

    connect(m_debuggerManager, SIGNAL(stateChanged(int)), this, SLOT(slotDebuggerStateChanged(int)));

    m_state = 0;
    m_prevState = 0;
    m_sQuirkDone = false;
    m_sHaveInferior = false;
    m_sManuallyStopped = false;

    ProjectExplorer::ProjectExplorerPlugin::instance()->
            startRunControl(m_inspectorRunControl, ProjectExplorer::Constants::DEBUGMODE);
}

void ProbeInjectingDebugger::uninitInspection()
{
    PRECONDITION;
    disconnect(m_inspectorRunControl, 0, this, 0);
    disconnect(m_debuggerManager, 0, this, 0);
    m_inspectorRunControl = 0;
    emit inspectionEnded();
}
