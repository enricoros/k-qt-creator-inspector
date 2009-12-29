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

#include "combotreewidget.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QList>
#include <QModelIndex>
#include <QPainter>
#include <QStyledItemDelegate>

namespace Inspector {
namespace Internal {

class NodesDelegate : public QStyledItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (index.data(Qt::UserRole).isNull())
            painter->fillRect(option.rect, Qt::lightGray);
        QStyledItemDelegate::paint(painter, option, index);
    }
};

class NodesComboBox : public QComboBox
{
public:
    NodesComboBox(int level, QWidget *parent = 0)
      : QComboBox(parent)
      , m_level(level)
    {
        setItemDelegate(new NodesDelegate);
    }

    int level() const
    {
        return m_level;
    }

private:
    int m_level;
};

struct MenuNode {
    // description
    QString name;
    QVariant data;
    QIcon icon;

    // tree-like structure
    MenuNode * parent;
    QList<MenuNode *> children;

    MenuNode(const QString &name = QString(), const QVariant &data = QVariant(), const QIcon &icon = QIcon())
        : name(name), data(data), icon(icon), parent(0)
    {
    }

    ~MenuNode()
    {
        if (parent)
            parent->children.removeAll(this);
        qDeleteAll(children);
    }

    MenuNode *findChild(const QString &childName)
    {
        foreach (MenuNode *child, children)
            if (child->name == childName)
                return child;
        return 0;
    }

    MenuNode *getChild(const QString &childName)
    {
        MenuNode *childNode = findChild(childName);
        if (!childNode) {
            childNode = new MenuNode(childName);
            childNode->parent = this;
            children.append(childNode);
        }
        return childNode;
    }
};


ComboTreeWidget::ComboTreeWidget(QWidget *parent)
  : Utils::StyledBar(parent)
  , m_rootNode(new MenuNode)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addStretch(5);
    setLayout(layout);
}

ComboTreeWidget::~ComboTreeWidget()
{
    qDeleteAll(m_comboPath);
    delete m_rootNode;
}

void ComboTreeWidget::addItem(const QStringList &path, const QVariant &userData, const QIcon &icon)
{
    // update the tree
    MenuNode *node = m_rootNode;
    foreach (const QString &token, path) {
        node = node->getChild(token);
        if (token == path.last()) {
            node->data = userData;
            node->icon = icon;
        }
    }

    // update the visual path
    syncLeafPath();

    // notify about the changes
    emit treeChanged();
}

void ComboTreeWidget::removeItem(const QStringList &path)
{
    // find out the node sequence for the path
    QList<MenuNode *> stack;
    MenuNode *node = m_rootNode;
    foreach (const QString &token, path) {
        if ((node = node->findChild(token))) {
            stack.append(node);
        } else {
            qWarning("ComboTreeWidget::removePath: the given path is not present");
            break;
        }
    }

    // backtraverse the node sequence deleting empty branches
    while (!stack.isEmpty()) {
        MenuNode * node = stack.takeLast();
        if (node->children.isEmpty())
            delete node;
        else
            break;
        m_nodePath.removeAll(node);
    }

    // update the visual path
    syncLeafPath();

    // notify about the changes
    emit treeChanged();
}

void ComboTreeWidget::clear()
{
    // fast clear: recreate the private data
    m_nodePath.clear();
    qDeleteAll(m_comboPath);
    m_comboPath.clear();
    delete m_rootNode;
    m_rootNode = new MenuNode;

    // update the visual path
    syncLeafPath();

    // notify about the changes
    emit treeChanged();
}

void ComboTreeWidget::setCurrentPath(const QStringList &path)
{
    m_nodePath.clear();
    MenuNode *node = m_rootNode;
    foreach (const QString &token, path) {
        node = node->findChild(token);
        if (!node)
            break;
        m_nodePath.append(node);
    }
    syncLeafPath();
}

QStringList ComboTreeWidget::currentPath() const
{
    QStringList path;
    foreach (MenuNode *node, m_nodePath)
        path.append(node->name);
    return path;
}

QVariant ComboTreeWidget::currentData() const
{
    if (m_nodePath.isEmpty())
        return QVariant();
    return m_nodePath.last()->data;
}

void ComboTreeWidget::paintEvent(QPaintEvent *event)
{
    Utils::StyledBar::paintEvent(event);
}

void ComboTreeWidget::slotComboIndexChanged(int comboIndex)
{
    // change the right node in the path and pune exceeding ones
    NodesComboBox *combo = static_cast<NodesComboBox *>(sender());
    int pathLevel = combo->level();
    if (pathLevel >= m_nodePath.count())
        return;
    MenuNode *parentNode = m_nodePath[pathLevel]->parent;
    if (comboIndex < 0 || comboIndex >= parentNode->children.count())
        return;
    m_nodePath[pathLevel] = parentNode->children[comboIndex];
    while (m_nodePath.count() > (pathLevel + 1))
        m_nodePath.removeLast();

    // update leaf path, both visually and in the nodePath
    syncLeafPath();

    // notify about the change if a leaf was selected
    if (!m_nodePath.isEmpty() && m_nodePath.last()->data.isValid())
        emit pathSelected(currentPath(), currentData());
}

void ComboTreeWidget::syncLeafPath()
{
    int pathLevel = 0;
    MenuNode *parentNode = m_rootNode;
    for (; parentNode && !parentNode->children.isEmpty(); ++pathLevel) {

        // complete path
        if (m_nodePath.count() <= pathLevel)
            m_nodePath.append(parentNode->children.first());
        MenuNode *pathNode = m_nodePath.at(pathLevel);

        // get the combo for the level, or append one if missing
        NodesComboBox *combo = 0;
        if (m_comboPath.count() <= pathLevel) {
            combo = new NodesComboBox(pathLevel, this);
            connect(combo, SIGNAL(activated(int)), this, SLOT(slotComboIndexChanged(int)));
            static_cast<QHBoxLayout *>(layout())->insertWidget(layout()->count() - 1, combo, 1);
            m_comboPath.append(combo);
        } else {
            combo = m_comboPath.at(pathLevel);
            combo->clear();
        }

        // repopulate combo with children items (and select the active one)
        bool parentSet = false;
        foreach (MenuNode *child, parentNode->children) {
            combo->addItem(child->icon, child->name, child->data);
            if (child == pathNode) {
                combo->setCurrentIndex(combo->count() - 1);
                parentNode = child;
                parentSet = true;
            }
        }
        if (!parentSet)
            break;
    }

    // delete exceeding combo boxes
    while (m_comboPath.count() > pathLevel)
        delete m_comboPath.takeLast();

    // relayout combos (width may have changed)
    layout()->activate();
}

} // namespace Internal
} // namespace Inspector
