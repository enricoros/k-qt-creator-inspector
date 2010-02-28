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

#ifndef INSPECTORRUNNER_H
#define INSPECTORRUNNER_H

#include <debugger/debuggermanager.h>
#include <projectexplorer/runconfiguration.h>
#include <projectexplorer/applicationrunconfiguration.h>

namespace Inspector {
namespace Internal {


// This is a job description
class InspectorRunControl : public ProjectExplorer::RunControl
{
    Q_OBJECT

public:
    InspectorRunControl(Debugger::DebuggerManager *manager,
                        ProjectExplorer::LocalApplicationRunConfiguration *runConfiguration);
    InspectorRunControl(Debugger::DebuggerManager *manager,
                        const Debugger::DebuggerStartParametersPtr &startParameters);

    void setInspectorServerName(const QString &serverName);

    // ProjectExplorer::RunControl
    virtual void start();
    virtual void stop();
    virtual bool isRunning() const;
    virtual QString displayName() const;

signals:
    void stopRequested();

private slots:
    void debuggingFinished();
    void slotAddToOutputWindowInline(const QString &output);

private:
    void init();
    Debugger::DebuggerStartParametersPtr m_startParameters;
    Debugger::DebuggerManager *m_manager;
    QString m_name;
    bool m_running;
};

} // namespace Internal
} // namespace Inspector

#endif // INSPECTORRUNNER_H
