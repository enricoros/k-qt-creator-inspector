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

#ifndef STATUSBARWIDGET_H
#define STATUSBARWIDGET_H

#include <QWidget>
#include <QList>
#include <QModelIndex>
#include <QToolButton>
class QAbstractButton;
class QHBoxLayout;
class QPixmap;

namespace Inspector {
namespace Internal {

class TasksModel;

class KillTaskButton : public QToolButton
{
    Q_OBJECT

public:
    KillTaskButton(quint32 tid, QWidget *parent = 0);

    quint32 tid() const;

private:
    quint32 m_tid;
};

class StatusBarWidget : public QWidget
{
    Q_OBJECT

public:
    StatusBarWidget(QWidget *parent = 0);

    void setTasksModel(TasksModel *model);

protected:
    // ::QWidget
    void paintEvent(QPaintEvent *);

private slots:
    void slotTasksChanged();
    void slotKillTask();

private:
    QPixmap *m_shadowTile;
    QList<KillTaskButton *> m_buttons;
    QHBoxLayout *m_layout;
    TasksModel *m_model;
};

} // namespace Internal
} // namespace Inspector

#endif // STATUSBARWIDGET_H
