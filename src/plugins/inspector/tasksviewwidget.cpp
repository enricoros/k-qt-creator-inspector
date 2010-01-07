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

#include "tasksviewwidget.h"
#include "tasksmodel.h"
#include "tasksscene.h"
#include <QPalette>

using namespace Inspector::Internal;

// the scene drawing the tasks

TasksViewWidget::TasksViewWidget(QWidget *parent)
  : QGraphicsView(parent)
  , m_tasksModel(0)
  , m_scene(new TasksScene)
{
    // customize widget
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setFrameStyle(QFrame::NoFrame);
    setScene(m_scene);
}

void TasksViewWidget::setTasksModel(TasksModel *model)
{
    // clear previous model data
    if (m_tasksModel) {
        // forget previous model
        disconnect(m_tasksModel, 0, this, 0);
    }

    // set new model
    m_tasksModel = model;

    // handle new model's data
    if (m_tasksModel) {
        // use current model
        connect(m_tasksModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(slotTasksChanged()));
        connect(m_tasksModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(slotTasksChanged()));
        connect(m_tasksModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotTasksChanged()));
    }
}

void TasksViewWidget::tempAddTest()
{
    static quint32 id = 1;
    m_tasksModel->addTask(id, tr("Task%1").arg(id), "No Description Provided");
    id++;
}

QSize TasksViewWidget::sizeHint() const
{
    return minimumSizeHint();
}

QSize TasksViewWidget::minimumSizeHint() const
{
    return QSize(100, TasksScene::fixedHeight());
}

void TasksViewWidget::slotAbortTask(quint32 tid)
{
    qWarning("TasksViewWidget::slotAbortTask: %d TODO", tid);
}

void TasksViewWidget::slotTasksChanged()
{
    QList<quint32> tasks = m_tasksModel->activeTasksId();
/*
    // delete exceeding buttons
    QList<KillTaskButton *>::iterator it = m_buttons.begin();
    while (it != m_buttons.end()) {
        KillTaskButton *button = *it;
        quint32 tid = button->tid();
        if (!tasks.contains(tid))
            emit removeActiveTask(tid);
        tasks.removeAll(tid);
        ++it;
    }
*/
    // create new buttons
    foreach (quint32 tid, tasks)
        emit newActiveTask(tid, m_tasksModel->taskName(tid));
}
