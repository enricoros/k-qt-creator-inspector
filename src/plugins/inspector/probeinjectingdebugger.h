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

#ifndef PROBEINJECTINGDEBUGGER_H
#define PROBEINJECTINGDEBUGGER_H

#include <QtCore/QObject>
#include <QtCore/QVariantList>
#include "inspectiontarget.h"

namespace Debugger {
class DebuggerManager;
}

namespace Inspector {
namespace Internal {

class InspectorRunControl;
class SharedDebugger;

/**
  \brief Debugger based probe injection and control (using Creator's DebuggerManager)

  This class uses DebuggerManager directly (since it's the only exposed
  symbol in the debug module) and implements some logic to load the
  'Probe' just after the degger begins its operation.

  DebuggerManager could be changed to allow loading of libraries (symbols)
  and the calling of functions.
*/
class ProbeInjectingDebugger : public QObject
{
    Q_OBJECT

public:
    ~ProbeInjectingDebugger();

    bool setInspectionTarget(const InspectionTarget &, const QString &localServerName);
    void closeInspection();
    bool inspecting() const;

    void callProbeFunction(const QString &name, const QVariantList &args);

signals:
    void inspectionStarted();
    void inspectionEnded();

private slots:
    void slotDmStateChanged(int);
    void slotRunControlStarted();
    void slotRunControlFinished();
    void slotRunControlDestroyed();

private:
    friend class SharedDebugger;
    ProbeInjectingDebugger(QObject *parent = 0);
    void initInspection();
    void uninitInspection();
    Debugger::DebuggerManager *m_debuggerManager;
    InspectorRunControl *m_inspectorRunControl;
};

} // namespace Internal
} // namespace Inspector

#endif // PROBEINJECTINGDEBUGGER_H
