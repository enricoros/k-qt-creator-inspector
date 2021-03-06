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

#ifndef INSPECTIONTARGET_H
#define INSPECTIONTARGET_H

#include <QtCore/QtGlobal>
#include <QtCore/QString>

namespace ProjectExplorer {
class LocalApplicationRunConfiguration;
class RunControl;
}

namespace Inspector {
namespace Internal {


class InspectionTarget
{
public:
    InspectionTarget();

    enum TargetType {
        Undefined,
        StartLocalRunConfiguration,
        AttachToPid,
        HijackRunControl,
        HijackDebuggerRunControl
    };
    TargetType type;
    ProjectExplorer::LocalApplicationRunConfiguration *runConfiguration;
    quint64 pid;
    ProjectExplorer::RunControl *runControl;

    QString displayName;
};

} // namespace Internal
} // namespace Inspector

#endif // INSPECTIONTARGET_H
