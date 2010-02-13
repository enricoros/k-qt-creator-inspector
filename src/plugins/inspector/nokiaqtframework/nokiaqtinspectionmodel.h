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

#ifndef NOKIAQTINSPECTIONMODEL_H
#define NOKIAQTINSPECTIONMODEL_H

#include "iinspectionmodel.h"

namespace Inspector {
namespace Internal {

class NokiaQtInspectionModel : public IInspectionModel
{
    Q_OBJECT

public:
    NokiaQtInspectionModel(QObject *parent = 0);

    enum {
        ProbeStatus_Row = 1,
        CommServer_Row = 2      // this is managed by the LocalCommServer
    };

    void setDebugEnabled(bool);
    void setDebugStopped(bool);
    void setProbePresent(bool);
    void setProbeInjected(bool);
    void setProbeActive(bool);

    QString localServerName() const;
    int probeActivationFlags() const;
    bool debugPaint() const;

public slots:
    void setDebugPaint(bool);
};

} // namespace Internal
} // namespace Inspector

#endif // NOKIAQTINSPECTIONMODEL_H
