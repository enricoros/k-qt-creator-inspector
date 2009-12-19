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

#ifndef COMBOTREEWIDGET_H
#define COMBOTREEWIDGET_H

#include <utils/styledbar.h>
#include <QIcon>
#include <QVariant>

namespace Inspector {
namespace Internal {

/**
    \brief Navigates a tree with horizontal combo-boxes
*/
class ComboTreeWidget : public Utils::StyledBar
{
    Q_OBJECT

public:
    ComboTreeWidget(QWidget *parent = 0);

    void addPath(const QStringList &path, const QVariant &userData, const QIcon &icon = QIcon());
    void removePath(const QStringList &path);
    void clear();

    QStringList currentPath() const;
    QVariant currentData() const;
    void setPath(const QStringList &path);

signals:
    void treeChanged();
    void pathSelected(const QStringList &path, const QVariant &userData);

private slots:
    void slotComboIndexChanged(int index);
};

} // namespace Internal
} // namespace Inspector

#endif // COMBOTREEWIDGET_H
