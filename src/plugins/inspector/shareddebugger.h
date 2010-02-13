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

namespace Debugger {
class DebuggerManager;
}

namespace ProjectExplorer {
class RunConfiguration;
}

namespace Inspector {
namespace Internal {

/**
  \brief Controls Creator's Debugger Manager (single instanced by default)
*/
class SharedDebugger : public QObject
{
    Q_OBJECT

public:
    SharedDebugger(QObject *parent = 0);

    bool acquirable() const;
    bool acquire();
    void release();

    bool startPidAttach(quint64 pid);
    bool startRunConfiguration(ProjectExplorer::RunConfiguration *);

    void callProbeFunction(const QString &name, const QVariantList &args);

signals:
    void acquirableChanged(bool);

private slots:
    void slotDmStateChanged(int);

private:
    void setRunning(bool);
    Debugger::DebuggerManager *m_debuggerManager;
    bool m_acquired;
    bool m_running;
};

} // namespace Internal
} // namespace Inspector

#endif // SHAREDDEBUGGER_H
