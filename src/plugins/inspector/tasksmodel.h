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

#ifndef TASKSMODEL_H
#define TASKSMODEL_H

#include "abstracteasymodel.h"
#include <QDateTime>

namespace Inspector {
namespace Internal {

class ModuleController;
class TaskItem;

class Q_DECL_EXPORT TasksModel : public AbstractEasyModel
{
    Q_OBJECT

public:
    TasksModel(QObject *parent = 0);

    // query tasks info
    int totalTasksCount() const;
    int activeTasksCount() const;
    QList<quint32> activeTasksId() const;
    QString taskName(quint32 taskId) const;

    // called by the normal views
    bool requestStopTask(quint32 tid);

private:
    friend class ModuleController;
    bool addTask(quint32 tid, const QString &name, const QString &description);
    bool startTask(quint32 tid);
    bool stopTask(quint32 tid);
    TaskItem *task(quint32 tid) const;
    QStandardItem *tasksRoot() const;
};

class TaskItem : public QStandardItem
{
public:
    TaskItem(quint32 tid, const QString &name, const QString &description);

    // operations
    bool start();
    bool stop();
    bool setRequestStop();
    void setProgress(int progress);

    // query status
    quint32 tid() const;
    QString name() const;
    QString description() const;
    bool isActive() const;
    bool isStarted() const;
    QDateTime startDate() const;
    bool isEnded() const;
    QDateTime endDate() const;
    qreal duration() const;
    bool requestStop() const;
    int progress() const;

private:
    quint32 m_tid;
    QString m_name;
    QString m_description;
    bool m_started;
    QDateTime m_start;
    bool m_stopped;
    QDateTime m_stop;
    qreal m_duration;
    bool m_requestStop;
    int m_progress;
};

} // namespace Internal
} // namespace Inspector

#endif // TASKSMODEL_H
