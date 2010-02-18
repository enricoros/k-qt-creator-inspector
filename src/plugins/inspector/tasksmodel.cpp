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
  , m_requestStop(false)
  , m_progress(0)
{
}

bool TaskItem::start()
{
    m_started = true;
    m_start = QDateTime::currentDateTime();
    emitDataChanged();
    return true;
}

bool TaskItem::stop()
{
    if (m_started) {
        m_stopped = true;
        m_stop = QDateTime::currentDateTime();
        m_duration = m_stop.toTime_t() - m_start.toTime_t()
                     + ((qreal)(m_stop.time().msec() - m_start.time().msec())) / 1000.0;
        emitDataChanged();
        return true;
    }
    return false;
}

bool TaskItem::setRequestStop()
{
    if (m_stopped)
        return false;
    m_requestStop = true;
    emitDataChanged();
    return true;
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

bool TaskItem::isActive() const
{
    return m_started && !m_stopped;
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

bool TaskItem::requestStop() const
{
    return m_requestStop;
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
    //openDebugWidget();
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
        if (item->isActive())
            ids.append(item->tid());
    }
    return ids;
}

QString TasksModel::taskName(quint32 taskId) const
{
    const TaskItem *item = task(taskId);
    return item ? item->name() : QString();
}

bool TasksModel::addTask(quint32 tid, const QString &name, const QString &description)
{
    // safety check
    if (task(tid)) {
        qWarning("TasksModel::addTask: already present %d", tid);
        return false;
    }

    // add item
    TaskItem *taskItem = new TaskItem(tid, name, description);
    tasksRoot()->insertRow(0, taskItem);

    // refresh total tasks counter
    setItemValue(Tasks_Row, 0, tasksRoot()->rowCount());

    // if task is already active, increment active count
    if (taskItem->isActive())
        incrementIntValue(Tasks_Row, 1);
    return true;
}

bool TasksModel::startTask(quint32 tid)
{
    // safety checks
    TaskItem *item = task(tid);
    if (!item) {
        qWarning("TasksModel::startTask: can't find task %d", tid);
        return false;
    }
    if (item->isStarted()) {
        qWarning("TasksModel::startTask: task %d already started", tid);
        return false;
    }

    // start task
    bool started = item->start();

    // if started, increment active count
    if (started)
        setItemValue(Tasks_Row, 1, itemValue(Tasks_Row, 1).toInt() + 1);
    return true;
}

bool TasksModel::stopTask(quint32 tid)
{
    // safety checks
    TaskItem *item = task(tid);
    if (!item) {
        qWarning("TasksModel::stopTask: can't find task %d", tid);
        return false;
    }
    if (item->isEnded()) {
        qWarning("TasksModel::stopTask: task %d already stopped", tid);
        return false;
    }

    // stop task
    bool stopped = item->stop();

    // if stopped, decrement active count
    if (stopped)
        setItemValue(Tasks_Row, 1, itemValue(Tasks_Row, 1).toInt() - 1);
    return true;
}

bool TasksModel::requestStopTask(quint32 tid)
{
    // safety checks
    TaskItem *item = task(tid);
    if (!item) {
        qWarning("TasksModel::requestStopTask: can't find task %d", tid);
        return false;
    }
    if (item->isEnded()) {
        qWarning("TasksModel::requestStopTask: task %d already stopped", tid);
        return true;
    }

    // mark as requesting stop
    item->setRequestStop();
    return true;
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
