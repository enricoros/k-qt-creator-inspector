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

using namespace Inspector::Internal;

#define PRECONDITION QTC_ASSERT(m_inspectorRunControl, return)

//
// InspectingDebugger
//
ProbeInjectingDebugger::ProbeInjectingDebugger(QObject *parent)
  : QObject(parent)
  , m_debuggerManager(0)
  , m_inspectorRunControl(0)
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

void ProbeInjectingDebugger::slotDmStateChanged(int state)
{
    PRECONDITION;
    switch (state) {

    }
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

void ProbeInjectingDebugger::initInspection()
{
    connect(m_inspectorRunControl, SIGNAL(started()), this, SLOT(slotRunControlStarted()));
    connect(m_inspectorRunControl, SIGNAL(finished()), this, SLOT(slotRunControlFinished()));
    connect(m_inspectorRunControl, SIGNAL(destroyed()), this, SLOT(slotRunControlDestroyed()));

    connect(m_debuggerManager, SIGNAL(stateChanged(int)), this, SLOT(slotDmStateChanged(int)));

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
