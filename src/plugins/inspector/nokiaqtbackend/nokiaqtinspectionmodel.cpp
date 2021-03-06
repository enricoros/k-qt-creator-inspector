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

#include "nokiaqtinspectionmodel.h"

#include "../../../../share/qtcreator/gdbmacros/perfunction.h"

using namespace Inspector::Internal;

/* == NokiaQtInspectionModel Usage ==
Inherits rows from IInspectionModel

Row 'ProbeStatus_Row': Probe Status
  0: target name            undefined
  1: debug enabled          int (-1 (unknown), 0 no, 1 yes)
  2: debug stopped          int (-1 (unknown), 0 no, 1 yes)
  3: target capabilities    undefined
  4: probe present          int (-1 (unknown), 0 no, 1 yes)
  5: probe injected         int (-1 (unknown), 0 no, 1 yes)
  6: probe capabilities     undefined
  7: probe active           int (-1 (unknown), 0 no, 1 yes)

Row 'CommServer_Row': Communication Server
  0: server enabled
  1: server local name
  2: server listening
  3: probe connected
  4: probe info
  5: ---
  6: communication messages parent (1 row per string)
  7: communication errors parent (1 row per string)
  8: communication log parent (1 row per string)
*/

NokiaQtInspectionModel::NokiaQtInspectionModel(QObject *parent)
  : IInspectionModel(parent)
{
    // init model
    setItemValue(ProbeStatus_Row, 0, QString());
    setItemValue(ProbeStatus_Row, 1, -1);
    setItemValue(ProbeStatus_Row, 2, -1);
    setItemValue(ProbeStatus_Row, 3, QString());
    setItemValue(ProbeStatus_Row, 4, -1);
    setItemValue(ProbeStatus_Row, 5, -1);
    setItemValue(ProbeStatus_Row, 6, QString());
    setItemValue(ProbeStatus_Row, 7, -1);

    setBackendName(tr("Qt by Nokia"));
}

void NokiaQtInspectionModel::setDebugEnabled(bool value)
{
    setItemValue(ProbeStatus_Row, 1, value);
    if (!value) {
        setItemValue(ProbeStatus_Row, 4, false);
        setItemValue(ProbeStatus_Row, 5, false);
        setItemValue(ProbeStatus_Row, 7, false);
    }
}

void NokiaQtInspectionModel::setDebugStopped(bool value)
{
    setItemValue(ProbeStatus_Row, 2, value);
}

void NokiaQtInspectionModel::setProbePresent(bool value)
{
    setItemValue(ProbeStatus_Row, 4, value);
}

void NokiaQtInspectionModel::setProbeInjected(bool value)
{
    setItemValue(ProbeStatus_Row, 5, value);
}

void NokiaQtInspectionModel::setProbeActive(bool value)
{
    setItemValue(ProbeStatus_Row, 7, value);
}

QString NokiaQtInspectionModel::localServerName() const
{
    return itemValue(CommServer_Row, 1).toString();
}
