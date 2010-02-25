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

#ifndef MODULEMENUWIDGET_H
#define MODULEMENUWIDGET_H

#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtGui/QTreeWidget>
#include <QtGui/QIcon>

namespace Inspector {
namespace Internal {

class ModuleMenuWidget : public QTreeWidget
{
    Q_OBJECT

public:
    ModuleMenuWidget(QWidget *parent = 0);

    void addItem(const QStringList &path, quint32 id, const QIcon &icon = QIcon());
    void setCurrentItem(quint32 id);

signals:
    void panelSelected(quint32 id);

protected:
    QSize sizeHint() const;

private slots:
    void slotItemActivated(QTreeWidgetItem *);

private:
    QIcon iconOverlay(QStyle::StandardPixmap name, const QIcon &overlayIcon);
    void recursiveInsertion(QTreeWidgetItem *parent, QStringList remainingPath, quint32 id, const QIcon &icon);
};

} // namespace Internal
} // namespace Inspector

#endif // MODULEMENUWIDGET_H
