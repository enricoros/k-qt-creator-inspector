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

#include "abstracteasymodel.h"
#include <QTableView>
#include <QTreeView>

using namespace Inspector::Internal;

AbstractEasyModel::AbstractEasyModel(QObject *parent)
  : QStandardItemModel(parent)
  , m_debugView(0)
{
}

AbstractEasyModel::AbstractEasyModel(int rows, int columns, QObject *parent)
  : QStandardItemModel(rows, columns, parent)
  , m_debugView(0)
{
}

AbstractEasyModel::~AbstractEasyModel()
{
    delete m_debugView;
}

void AbstractEasyModel::setItemValue(int row, int column, const QVariant &value, int role)
{
    QStandardItem *item = new QStandardItem;
    item->setEditable(true);
    item->setData(value, role);
    setItem(row, column, item);
}

QVariant AbstractEasyModel::itemValue(int row, int column, int role) const
{
    QStandardItem *it = item(row, column);
    return it ? it->data(role) : QVariant();
}

int AbstractEasyModel::incrementIntValue(int row, int column)
{
    int value = itemValue(row, column).toInt() + 1;
    setItemValue(row, column, value);
    return value;
}

void AbstractEasyModel::openDebugWidget(const QModelIndex &root)
{
    if (!m_debugView) {
#if 1
        m_debugView = new QTreeView;
#else
        m_debugView = new QTableView;
#endif
        m_debugView->setModel(this);
        connect(m_debugView, SIGNAL(doubleClicked(QModelIndex)), m_debugView, SLOT(setRootIndex(QModelIndex)));
    }
    m_debugView->show();
    if (root.isValid())
        m_debugView->setRootIndex(root);
}
