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

#include "modulemenuwidget.h"
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QVBoxLayout>

using namespace Inspector::Internal;

ModuleMenuWidget::ModuleMenuWidget(QWidget *parent)
  : QTreeWidget(parent)
{
    // customize look & feel
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setFrameStyle(QFrame::NoFrame);
    setHeaderHidden(true);
    setIconSize(QSize(16, 16));
    setIndentation(indentation() * 4/5);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

    // connect to user interaction
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(slotItemActivated(QTreeWidgetItem*)));
}

static QTreeWidgetItem *recursiveSearchId(QTreeWidgetItem *parent, quint32 id)
{
    int children = parent->childCount();
    for (int i = 0; i < children; i++) {
        QTreeWidgetItem *item = parent->child(i);
        quint32 itemId = item->data(0, Qt::UserRole).toUInt();
        if (itemId == id)
            return item;
        if (QTreeWidgetItem *found = recursiveSearchId(item, id))
            return found;
    }
    return 0;
}

void ModuleMenuWidget::addItem(const QStringList &path, quint32 id, const QIcon &icon)
{
    recursiveInsertion(invisibleRootItem(), path, id, icon);
}

void ModuleMenuWidget::setCurrentItem(quint32 id)
{
    if (QTreeWidgetItem *item = recursiveSearchId(invisibleRootItem(), id)) {
        clearSelection();
        item->setExpanded(true);
        item->setSelected(true);
        slotItemActivated(item);
    }
}

QSize ModuleMenuWidget::sizeHint() const
{
    return QSize(sizeHintForColumn(0) + frameWidth() * 2 + 20, 100);
}

void ModuleMenuWidget::slotItemActivated(QTreeWidgetItem *item)
{
    if (!item)
        return;
    if (quint32 id = item->data(0, Qt::UserRole).toUInt())
        emit panelSelected(id);
}

QIcon ModuleMenuWidget::iconOverlay(QStyle::StandardPixmap name, const QIcon &overlayIcon)
{
    const QSize size = iconSize();
    const QPixmap overlayPixmap = overlayIcon.pixmap(size);
    QPixmap iconPixmap = QApplication::style()->standardIcon(name).pixmap(size);
    QPainter painter(&iconPixmap);
    painter.drawPixmap((iconPixmap.width() - overlayPixmap.width()) / 2,
                       (iconPixmap.height() - overlayPixmap.height()) / 2,
                       overlayPixmap);
    painter.end();
    return QIcon(iconPixmap);
}

void ModuleMenuWidget::recursiveInsertion(QTreeWidgetItem *parent, QStringList remainingPath, quint32 id, const QIcon &icon)
{
    QString name = remainingPath.takeFirst();

    // create leaf item
    if (remainingPath.isEmpty()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList() << name);
        item->setData(0, Qt::UserRole, id);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setIcon(0, iconOverlay(QStyle::SP_FileIcon, icon));
        item->setSizeHint(0, iconSize());
        item->setExpanded(true);
        return;
    }

    // look for an existing element with this name
    int children = parent->childCount();
    for (int i = 0; i < children; i++) {
        QTreeWidgetItem *existingItem = parent->child(i);
        if (existingItem->text(0) == name) {
            recursiveInsertion(existingItem, remainingPath, id, icon);
            return;
        }
    }

    // create the new Folder item
    QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList() << name);
    item->setFlags(Qt::ItemIsEnabled);
    item->setIcon(0, iconOverlay(QStyle::SP_DirIcon, QIcon(":/inspector/images/folder_overlay.png")));
    item->setSizeHint(0, iconSize());
    item->setExpanded(true);
    recursiveInsertion(item, remainingPath, id, icon);
}
