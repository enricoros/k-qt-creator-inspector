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

QSize TasksViewWidget::sizeHint() const
{
    return minimumSizeHint();
}

QSize TasksViewWidget::minimumSizeHint() const
{
    return QSize(200, TasksScene::fixedHeight());
}

void TasksViewWidget::slotStopTask(quint32 tid)
{
    if (m_tasksModel)
        m_tasksModel->requestStopTask(tid);
}

void TasksViewWidget::slotTasksChanged()
{
    QList<quint32> newTasks = m_tasksModel->activeTasksId();

    // notify exceeding tasks
    QList<quint32>::iterator it = m_activeTasks.begin();
    while (it != m_activeTasks.end()) {
        quint32 tid = *it;
        if (!newTasks.removeAll(tid)) {
            it = m_activeTasks.erase(it);
            emit removeActiveTask(tid);
            continue;
        }
        ++it;
    }

    // notify new tasks
    foreach (quint32 tid, newTasks) {
        m_activeTasks.append(tid);
        emit newActiveTask(tid, m_tasksModel->taskName(tid));
    }
}
