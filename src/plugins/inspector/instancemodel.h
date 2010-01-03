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

#ifndef INSTANCEMODEL_H
#define INSTANCEMODEL_H

#include "abstracteasymodel.h"
#include <QVariantList>

namespace Inspector {

namespace Internal {
}

class Q_DECL_EXPORT InstanceModel : public Internal::AbstractEasyModel
{
    Q_OBJECT

public:
    InstanceModel(QObject *parent = 0);

    enum { InstanceStatus_Row = 0, ProbeStatus_Row = 1, CommServer_Row = 2 };

    // instance status
    bool debugPaint() const;
    bool instanceEnabled() const;

    // probe status (set by the Debugger plugin)
    void setDebugEnabled(bool);
    void setDebugStopped(bool);
    void setProbePresent(bool);
    void setProbeInjected(bool);
    void setProbeActive(bool);
    // startup params (read by the Debugger plugin)
    QString localServerName() const;
    int probeActivationFlags() const;

    // temp model-function? maybe better in CommServer? ###
    // this is here only for not exposing commserver to the Debugger plugin
    bool callProbeFunction(const QString &name, const QVariantList &args = QVariantList());

public slots:
    void setDebugPaint(bool);
    void setInstanceEnabled(bool);

signals:
    void debuggerCallFunction(const QString &name, const QVariantList &args);
};

} // namespace Inspector

#endif // INSTANCEMODEL_H
