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

#include "instancemodel.h"
#include <QTableView>
#include <QTreeView>

#include "../../../share/qtcreator/gdbmacros/perfunction.h"

using namespace Inspector;

/* == InstanceModel Usage ==
Row 1: Instance Status
  0: enabled

Row 2: Probe Status
  0: target name            undefined
  1: debug enabled          int (-1 (unknown), 0 no, 1 yes)
  2: debug stopped          int (-1 (unknown), 0 no, 1 yes)
  3: target capabilities    undefined
  4: probe present          int (-1 (unknown), 0 no, 1 yes)
  5: probe injected         int (-1 (unknown), 0 no, 1 yes)
  6: probe capabilities     undefined
  7: probe active           int (-1 (unknown), 0 no, 1 yes)
*/

InstanceModel::InstanceModel(QObject *parent)
  : QStandardItemModel(8, 1, parent)
  , m_debugView(0)
{
    // init model
    setValue(InstanceStatus_Row, 0, true);
    setValue(ProbeStatus_Row, 0, QString());
    setValue(ProbeStatus_Row, 1, -1);
    setValue(ProbeStatus_Row, 2, -1);
    setValue(ProbeStatus_Row, 3, QString());
    setValue(ProbeStatus_Row, 4, -1);
    setValue(ProbeStatus_Row, 5, -1);
    setValue(ProbeStatus_Row, 6, QString());
    setValue(ProbeStatus_Row, 7, -1);
    openDebugWidget();
}

InstanceModel::~InstanceModel()
{
    delete m_debugView;
}

void InstanceModel::setValue(int row, int column, const QVariant &value, int role)
{
    QStandardItem *item = new QStandardItem;
    item->setEditable(true);
    item->setData(value, role);
    setItem(row, column, item);
}

QVariant InstanceModel::value(int row, int column, int role) const
{
    QStandardItem *it = item(row, column);
    return it ? it->data(role) : QVariant();
}

void InstanceModel::setInstanceEnabled(bool value)
{
    setValue(InstanceStatus_Row, 0, value);
}

bool InstanceModel::instanceEnabled() const
{
    return value(InstanceStatus_Row, 0).toBool();
}

void InstanceModel::setDebugEnabled(bool value)
{
    setValue(ProbeStatus_Row, 1, value);
}

void InstanceModel::setDebugStopped(bool value)
{
    setValue(ProbeStatus_Row, 2, value);
}

void InstanceModel::setProbePresent(bool value)
{
    setValue(ProbeStatus_Row, 4, value);
}

void InstanceModel::setProbeInjected(bool value)
{
    setValue(ProbeStatus_Row, 5, value);
}

void InstanceModel::setProbeActive(bool value)
{
    setValue(ProbeStatus_Row, 7, value);
}

QString InstanceModel::localServerName() const
{
    return value(InstanceModel::CommServer_Row, 1).toString();
}

int InstanceModel::probeActivationFlags() const
{
    // flags are in perfunction.h
    int flags = Inspector::Internal::AF_None;
    qWarning("InstanceModel::probeActivationFlags: TODO");
    //if (m_debugPaintFlag)
        flags |= Inspector::Internal::AF_PaintDebug;
    return flags;
}

bool InstanceModel::callProbeFunction(const QString &name, QVariantList args)
{
    // REFACTOR THIS!! ###
    // this is the ONE AND ONLY link to control the debugger, for now
    emit debuggerCallFunction(name, args);
    return true;
}

void InstanceModel::setDebugPaint(bool enabled)
{
    //setValue(Pro...);
    qWarning("InstanceModel::setDebugPaint: TODO");
}

void InstanceModel::openDebugWidget()
{
    if (!m_debugView) {
        m_debugView = new QTableView;
        m_debugView->setModel(this);
QTreeView *tv = new QTreeView;
tv->setModel(this);
tv->show();
    }
    m_debugView->show();
}
