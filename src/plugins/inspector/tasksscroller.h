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

#ifndef TASKSSCROLLER_H
#define TASKSSCROLLER_H

#include <QtCore/QBasicTimer>
#include <QtCore/QTime>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsWidget>

namespace Inspector {
namespace Internal {

class TasksModel;
class TasksScene;
class TaskRectangle;

class TasksScroller : public QGraphicsView
{
    Q_OBJECT

public:
    TasksScroller(QWidget *parent = 0);
    ~TasksScroller();

    void setTasksModel(TasksModel *model);
    void setTransparentBackground(bool);

    // ::QWidget
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

signals:
    void newActiveTask(quint32 tid, const QString &name);
    void removeActiveTask(quint32 tid);

public slots:
    void slotStopTask(quint32 tid);

private slots:
    void slotTasksChanged();

private:
    QList<quint32> m_activeTasks;
    TasksModel *m_tasksModel;
    TasksScene *m_scene;
};

//
// The Scene to show scrolling tasks
//
class TasksScene : public QGraphicsScene
{
    Q_OBJECT

public:
    TasksScene(QObject * parent = 0);

    static int fixedHeight();

    void clear();

    int pixelPerSecond() const;
    void setPixelPerSecond(int pps);

    bool scrollLocked() const;
    void setScrollLocked(bool locked);

    void setTasksModel(TasksModel *model);

protected:
    // ::QGraphicsScene
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    // ::QObject
    void timerEvent(QTimerEvent *);

private:
    void regenScene();
    void updateCurrentScene();
    TasksModel *m_tasksModel;
    QBasicTimer m_updateTimer;
    QTime m_startTime;
    bool m_scrollLocked;
    int m_pixelPerSecond;
    QList<TaskRectangle *> m_currentTasks;
};

class TaskRectangle : public QGraphicsWidget
{
public:
    TaskRectangle(quint32 taskId, int left, const QColor &color, QGraphicsItem * parent = 0);

    quint32 taskId() const;
    void updateSize(int right, int currentPercent);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

private:
    quint32 m_taskId;
    QPen m_contourPen;
    QBrush m_backBrush;
    QBrush m_foreBrush;
    QList<int> m_percs;
};

} // namespace Internal
} // namespace Inspector

#endif // TASKSSCROLLER_H
