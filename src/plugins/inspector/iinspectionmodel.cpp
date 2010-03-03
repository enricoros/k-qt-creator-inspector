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

#include "iinspectionmodel.h"

#include "../../../share/qtcreator/gdbmacros/perfunction.h"

using namespace Inspector::Internal;

/* == InspectionModel Usage ==
Row 'InspectionStatus_Row': Inspection Status
  0: targetName             string
  1: backendName          string
  2: monotonicId            int
*/

IInspectionModel::IInspectionModel(QObject *parent)
  : Internal::AbstractEasyModel(3, 0, parent)
{
    // init model
    setItemValue(InspectionStatus_Row, 0, QString("no target"));
    setItemValue(InspectionStatus_Row, 1, QString("no backend"));
    static int s_monotonicId = 0;
    setItemValue(InspectionStatus_Row, 2, (++s_monotonicId));
}

QString IInspectionModel::displayName() const
{
    return tr("%1 [%2]").arg(targetName()).arg(backendName());
}

QString IInspectionModel::targetName() const
{
    return itemValue(InspectionStatus_Row, 0).toString();
}

QString IInspectionModel::backendName() const
{
    return itemValue(InspectionStatus_Row, 1).toString();
}

int IInspectionModel::monotonicId() const
{
    return itemValue(InspectionStatus_Row, 2).toInt();
}

void IInspectionModel::setTargetName(const QString &name)
{
    setItemValue(InspectionStatus_Row, 0, name);
}

void IInspectionModel::setBackendName(const QString &name)
{
    setItemValue(InspectionStatus_Row, 1, name);
}
