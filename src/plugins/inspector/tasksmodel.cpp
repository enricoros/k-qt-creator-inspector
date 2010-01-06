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

#include "tasksmodel.h"

using namespace Inspector::Internal;

/* == TasksModel Usage ==
Row 'Tasks_Row': tasks
  0: count                  int
  1: active count           int
  2: tasks                  LIST(TaskItem)
*/

TaskItem::TaskItem(quint32 tid, const QString &name, const QString &description)
  : QStandardItem(name)
  , m_tid(tid)
  , m_name(name)
  , m_description(description)
  , m_started(false)
  , m_stopped(false)
  , m_duration(0)
  , m_progress(0)
{
    start();
}

void TaskItem::start()
{
    m_started = true;
    m_start = QDateTime::currentDateTime();
    emitDataChanged();
}

void TaskItem::stop()
{
    if (m_started) {
        m_stopped = true;
        m_stop = QDateTime::currentDateTime();
        m_duration = m_stop.toTime_t() - m_start.toTime_t()
                     + ((qreal)(m_stop.time().msec() - m_start.time().msec())) / 1000.0;
        emitDataChanged();
    }
}

void TaskItem::setProgress(int progress)
{
    m_progress = progress;
    emitDataChanged();
}

quint32 TaskItem::tid() const
{
    return m_tid;
}

QString TaskItem::name() const
{
    return m_name;
}

QString TaskItem::description() const
{
    return m_description;
}

bool TaskItem::isStarted() const
{
    return m_started;
}

QDateTime TaskItem::startDate() const
{
    return m_start;
}

bool TaskItem::isEnded() const
{
    return m_stopped;
}

QDateTime TaskItem::endDate() const
{
    return m_stop;
}

qreal TaskItem::duration() const
{
    return m_duration;
}

int TaskItem::progress() const
{
    return m_progress;
}


#define Tasks_Row     0

TasksModel::TasksModel(QObject *parent)
  : Internal::AbstractEasyModel(parent)
{
    // init model
    setItemValue(Tasks_Row, 0, 0);
    setItemValue(Tasks_Row, 1, 0);
    setItemValue(Tasks_Row, 2, "tasks");
    openDebugWidget();
}

int TasksModel::totalTasksCount() const
{
    return itemValue(Tasks_Row, 0).toInt();
}

int TasksModel::activeTasksCount() const
{
    return itemValue(Tasks_Row, 1).toInt();
}

QList<quint32> TasksModel::activeTasksId() const
{
    QList<quint32> ids;
    const QStandardItem *root = tasksRoot();
    int tasksCount = root->rowCount();
    for (int task = 0; task < tasksCount; ++task) {
        TaskItem *item = static_cast<TaskItem*>(root->child(task));
        if (item->isStarted() && !item->isEnded())
            ids.append(item->tid());
    }
    return ids;
}

QString TasksModel::taskName(quint32 taskId) const
{
    const TaskItem *item = task(taskId);
    return item ? item->name() : QString();
}

void TasksModel::addTask(quint32 tid, const QString &name, const QString &description)
{
    // add item
    TaskItem *taskItem = new TaskItem(tid, name, description);
    tasksRoot()->insertRow(0, taskItem);

    // refresh total taskss counter
    setItemValue(Tasks_Row, 0, tasksRoot()->rowCount());
}

void TasksModel::startTask(quint32 tid)
{
    TaskItem *item = task(tid);
    if (!item)
        qWarning("TasksModel::startTask: can't find task %d", tid);
    else if (item->isStarted())
        qWarning("TasksModel::startTask: task %d already started", tid);
    else
        item->start();
}

void TasksModel::stopTask(quint32 tid)
{
    TaskItem *item = task(tid);
    if (!item)
        qWarning("TasksModel::stopTask: can't find task %d", tid);
    else if (item->isEnded())
        qWarning("TasksModel::stopTask: task %d already stopped", tid);
    else
        item->stop();
}

QModelIndex TasksModel::tasksTableIndex() const
{
    return index(Tasks_Row, 2);
}

TaskItem *TasksModel::task(quint32 tid) const
{
    QStandardItem *root = tasksRoot();
    int tasksCount = root->rowCount();
    for (int task = 0; task < tasksCount; ++task) {
        TaskItem *item = static_cast<TaskItem*>(root->child(task));
        if (item->tid() == tid)
            return item;
    }
    return 0;
}

QStandardItem *TasksModel::tasksRoot() const
{
    return item(Tasks_Row, 2);
}
