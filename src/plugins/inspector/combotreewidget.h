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
#include <QStringList>
#include <QVariant>
class QComboBox;

namespace Inspector {
namespace Internal {

class MenuNode;
class NodesComboBox;

/**
    \brief Navigates a tree with horizontal combo-boxes

    Ex. Structure  | Path
    ---------------+----------------------------------
    * leaf1        | /leaf1
    * branch1      |
      * leaf 1     | /branch1/leaf 1
      * leaf 2     | /branch1/leaf 2
    * branch2      |
      * branch1    |
        * branch1  |
          * leaf 1 | /branch2/branch1/branch1/leaf 1
    * leaf2        | /leaf2
    * branch3      |
      * branch1    |
        * leaf 1   | /branch3/branch1/leaf 1
      * branch2    |
        * leaf 1   | /branch3/branch2/leaf 1
      * leaf 1     | /branch3/leaf 1

    At any time one leaf (path) is selected. Note that a leaf can not have
    children (otherwise the user interaction needs to be re-though)
*/
class ComboTreeWidget : public Utils::StyledBar
{
    Q_OBJECT

public:
    ComboTreeWidget(QWidget *parent = 0);
    ~ComboTreeWidget();

    // contents setup
    void addItem(const QStringList &path, const QVariant &userData, const QIcon &icon = QIcon());
    void removeItem(const QStringList &path);
    void clear();

    // current path
    void setCurrentPath(const QStringList &path);
    QStringList currentPath() const;
    QVariant currentData() const;

signals:
    void treeChanged();
    void pathSelected(const QStringList &path, const QVariant &userData);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void slotComboIndexChanged(int index);

private:
    void syncLeafPath();

    MenuNode * m_rootNode;
    QList<MenuNode *> m_nodePath;
    QList<NodesComboBox *> m_comboPath;
};

} // namespace Internal
} // namespace Inspector

#endif // COMBOTREEWIDGET_H
