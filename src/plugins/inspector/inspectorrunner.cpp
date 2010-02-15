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

#include "inspectorrunner.h"

#include <coreplugin/icore.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/debugginghelper.h>
#include <projectexplorer/target.h>

#include <QtCore/QFileInfo>

using namespace Inspector::Internal;

// NOTE: SYNC this with Debugger::Internal::DebuggerRunner - HARDCODED

//
// InspectorRunControl
//
InspectorRunControl::InspectorRunControl(Debugger::DebuggerManager *manager,
                                         ProjectExplorer::LocalApplicationRunConfiguration *runConfiguration)
  : RunControl(runConfiguration)
  , m_startParameters(new Debugger::DebuggerStartParameters())
  , m_manager(manager)
  , m_running(false)
{
    init();
    if (!runConfiguration)
        return;

    m_startParameters->startMode = Debugger::StartInternal;
    m_startParameters->executable = runConfiguration->executable();
    m_startParameters->environment = runConfiguration->environment().toStringList();
    m_startParameters->workingDir = runConfiguration->workingDirectory();
    m_startParameters->processArgs = runConfiguration->commandLineArguments();

    switch (m_startParameters->toolChainType) {
    case ProjectExplorer::ToolChain::UNKNOWN:
    case ProjectExplorer::ToolChain::INVALID:
        m_startParameters->toolChainType = runConfiguration->toolChainType();
        break;
    default:
        break;
    }
    if (runConfiguration->target()->project()) {
        m_startParameters->buildDir =
            runConfiguration->target()->activeBuildConfiguration()->buildDirectory();
    }
    m_startParameters->useTerminal =
        runConfiguration->runMode() == ProjectExplorer::LocalApplicationRunConfiguration::Console;
    m_startParameters->dumperLibrary =
        runConfiguration->dumperLibrary();
    m_startParameters->dumperLibraryLocations =
        runConfiguration->dumperLibraryLocations();

    QString qmakePath = ProjectExplorer::DebuggingHelperLibrary::findSystemQt(
            runConfiguration->environment());
    if (!qmakePath.isEmpty()) {
        QProcess proc;
        QStringList args;
        args.append(QLatin1String("-query"));
        args.append(QLatin1String("QT_INSTALL_HEADERS"));
        proc.start(qmakePath, args);
        proc.waitForFinished();
        QByteArray ba = proc.readAllStandardOutput().trimmed();
        QFileInfo fi(QString::fromLocal8Bit(ba) + "/..");
        m_startParameters->qtInstallPath = fi.absoluteFilePath();
    }
}

InspectorRunControl::InspectorRunControl(Debugger::DebuggerManager *manager,
                                         const Debugger::DebuggerStartParametersPtr &startParameters)
  : RunControl(0)
  , m_startParameters(startParameters)
  , m_manager(manager)
  , m_running(false)
{
    init();

    if (m_startParameters->environment.empty())
        m_startParameters->environment = ProjectExplorer::Environment::Environment().toStringList();
    m_startParameters->useTerminal = false;
}

void InspectorRunControl::init()
{
    connect(m_manager, SIGNAL(debuggingFinished()),
            this, SLOT(debuggingFinished()),
            Qt::QueuedConnection);
    connect(m_manager, SIGNAL(applicationOutputAvailable(QString)),
            this, SLOT(slotAddToOutputWindowInline(QString)),
            Qt::QueuedConnection);
    connect(m_manager, SIGNAL(inferiorPidChanged(qint64)),
            this, SLOT(bringApplicationToForeground(qint64)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(stopRequested()),
            m_manager, SLOT(exitDebugger()));
}

void InspectorRunControl::start()
{
    m_running = true;
    QString errorMessage;
    QString settingsCategory;
    QString settingsPage;
    if (m_manager->checkDebugConfiguration(m_startParameters->toolChainType, &errorMessage,
                                           &settingsCategory, &settingsPage)) {
        m_manager->startNewDebugger(m_startParameters);
        m_manager->continueExec();
    } else {
        error(this, errorMessage);
        emit finished();
        Core::ICore::instance()->showWarningWithOptions(tr("Inspector"), errorMessage,
                                                        QString(),
                                                        settingsCategory, settingsPage);
    }
}

void InspectorRunControl::slotAddToOutputWindowInline(const QString &data)
{
    emit addToOutputWindowInline(this, data);
}

void InspectorRunControl::stop()
{
    m_running = false;
    emit stopRequested();
}

void InspectorRunControl::debuggingFinished()
{
    m_running = false;
    emit finished();
}

bool InspectorRunControl::isRunning() const
{
    return m_running;
}
