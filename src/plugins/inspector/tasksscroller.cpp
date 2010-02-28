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

#include "tasksscroller.h"
#include "inspectorstyle.h"
#include "tasksmodel.h"

#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPalette>
#include <QtGui/QScrollBar>

using namespace Inspector::Internal;

//
// TasksScroller
//
TasksScroller::TasksScroller(QWidget *parent)
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

TasksScroller::~TasksScroller()
{
    delete m_scene;
}

void TasksScroller::setTasksModel(TasksModel *model)
{
    // clear previous model data
    if (m_tasksModel) {
        // forget previous model
        disconnect(m_tasksModel, 0, this, 0);
    }

    // set new model
    m_tasksModel = model;
    m_scene->setTasksModel(m_tasksModel);

    // handle new model's data
    if (m_tasksModel) {
        // use current model
        connect(m_tasksModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(slotTasksChanged()));
        connect(m_tasksModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(slotTasksChanged()));
        connect(m_tasksModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(slotTasksChanged()));
    }
}

void TasksScroller::setTransparentBackground(bool transparent)
{
    setAutoFillBackground(!transparent);
    viewport()->setAutoFillBackground(!transparent);
}

QSize TasksScroller::sizeHint() const
{
    return minimumSizeHint();
}

QSize TasksScroller::minimumSizeHint() const
{
    return QSize(200, TasksScene::fixedHeight());
}

void TasksScroller::slotStopTask(quint32 tid)
{
    if (m_tasksModel)
        m_tasksModel->requestStopTask(tid);
}

void TasksScroller::slotTasksChanged()
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

void TasksScroller::drawBackground(QPainter *painter, const QRectF &/*rect*/)
{
    // draw the subtle bottom white line
    painter->save();
    painter->resetTransform();
    QLinearGradient lg(0, 0, width(), 0);
    lg.setColorAt(0.0,  QColor(250, 250, 250,   0));
    lg.setColorAt(0.27, QColor(250, 250, 250, 160));
    lg.setColorAt(0.5,  QColor(255, 255, 255));
    lg.setColorAt(0.73, QColor(250, 250, 250, 160));
    lg.setColorAt(1.0,  QColor(250, 250, 250,   0));
    painter->fillRect(0, height() - 1, width(), 1, lg);
    painter->restore();
}


//
// TasksScene
//
TasksScene::TasksScene(QObject * parent)
  : QGraphicsScene(parent)
  , m_tasksModel(0)
  , m_scrollLocked(true)
  , m_pixelPerSecond(10)
{
    m_updateTimer.start(100, this);
}

void TasksScene::clear()
{
    // remove all items
    foreach (QGraphicsItem * item, items()) {
        removeItem(item);
        delete item;
    }

    // clear start time
    m_startTime.start();
}

bool TasksScene::scrollLocked() const
{
    return m_scrollLocked;
}

void TasksScene::setScrollLocked(bool locked)
{
    if (locked != m_scrollLocked) {
        m_scrollLocked = locked;
        if (m_scrollLocked)
            regenScene();
    }
}

void TasksScene::setTasksModel(TasksModel *model)
{
    m_tasksModel = model;
}

int TasksScene::pixelPerSecond() const
{
    return m_pixelPerSecond;
}

void TasksScene::setPixelPerSecond(int pps)
{
    if (pps != m_pixelPerSecond) {
        m_pixelPerSecond = pps;
        regenScene();
    }
}

void TasksScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // just consume the event
    event->accept();
}

void TasksScene::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    int sceneWidth = sceneRect().width();
    if (sceneWidth < 1)
        return;
    qreal scrollPercent = event->scenePos().x() / (qreal)sceneWidth;
    if (scrollPercent < 0.95) {
        m_scrollLocked = false;
        TasksScroller * widget = dynamic_cast<TasksScroller *>(views().first());
        int newVal = (int)(scrollPercent * (qreal)widget->horizontalScrollBar()->maximum());
        widget->horizontalScrollBar()->setValue(newVal);
    } else
        m_scrollLocked = true;
    event->accept();
}

void TasksScene::timerEvent(QTimerEvent * event)
{
    if (event->timerId() != m_updateTimer.timerId())
        return QGraphicsScene::timerEvent(event);

    if (m_startTime.isNull())
        m_startTime.start();

    updateCurrentScene();
}

void TasksScene::regenScene()
{

}

void TasksScene::updateCurrentScene()
{
    if (!m_startTime.isValid())
        return;

    // update scene size based on pixels from start
    QList<QGraphicsView *> viewports = views();
    int minWidth = viewports.isEmpty() ? 100 : viewports.first()->width();
    int contentsWidth = (m_startTime.elapsed() * m_pixelPerSecond) / 1000;
    setSceneRect(0, 0, qMax(contentsWidth, minWidth), fixedHeight());
    update();

    // if locked scrolling, update view's scrollbar too
    if (m_scrollLocked && !views().isEmpty()) {
        if (TasksScroller * widget = qobject_cast<TasksScroller *>(views().first()))
            widget->horizontalScrollBar()->setValue(contentsWidth);
    }

    // SYNC TASKS - FIXME change this with async 'add/remove' notifications from the model
    if (!m_tasksModel)
        return;

    QList<quint32> taskIds = m_tasksModel->activeTasksId();
    foreach (quint32 taskId, taskIds) {
        const TaskItem *task = m_tasksModel->constTask(taskId);

        // update if found
        bool missing = true;
        foreach (TaskRectangle *rect, m_currentTasks) {
            if (rect->taskId() == taskId) {
                missing = false;

                // delete if expired
                if (!task->isActive()) {
                    m_currentTasks.removeAll(rect);
                    delete rect;
                }
                // or update size
                else {
                    rect->updateSize(contentsWidth, task->progress());
                }
                break;
            }
        }

        // create if missing
        if (missing && task->isActive()) {
            TaskRectangle * i = new TaskRectangle(taskId, contentsWidth, task->name(), task->color());
            m_currentTasks.append(i);
            addItem(i);
            continue;
        }
    }
}


//
// TaskRectangle
//
TaskRectangle::TaskRectangle(quint32 taskId, int left, const QString &label,
                             const QColor &baseColor, QGraphicsItem *parent)
  : QGraphicsWidget(parent)
  , m_taskId(taskId)
  , m_label(label)
{
    setPos(left - 1, 0);
    resize(2, TasksScene::fixedHeight() - 1);

    QColor color = baseColor;

#if 1
    QLinearGradient backGradient(0, 0, 0, TasksScene::fixedHeight() - 1);
    color.setAlpha(128);
    backGradient.setColorAt(0.0, color.dark(280));
    color.setAlpha(192);
    backGradient.setColorAt(1.0, color.dark(200));
    m_backBrush = backGradient;
#else
    color.setAlpha(64);
    m_backBrush = color.dark();
#endif

    color.setAlpha(192);
    m_foreBrush = color.light(200);
}

quint32 TaskRectangle::taskId() const
{
    return m_taskId;
}

void TaskRectangle::updateSize(int right, int currentPercent)
{
    resize(right - (int)x(), TasksScene::fixedHeight() - 1);
    m_percs.append(currentPercent);
    update();
}

void TaskRectangle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    emit clicked(m_taskId);
}

void TaskRectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QRect rect = boundingRect().toRect();
    if (rect.width() < 1)
        return;

    // draw the task rectangle
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->fillRect(rect, m_backBrush);

    // draw the percent polygon
    painter->setBrush(m_foreBrush);
    const int pH = rect.height();
    int x = 1;
    foreach (int val, m_percs) {
        int y = (pH * val) / 100.0;
        painter->fillRect(x, pH - y, 1, y, m_foreBrush);
        x++;
    }

    QRect r = rect.adjusted(2, 0, -2, -1);
    if (r.width() > 2) {
        QFont font = painter->font();
        font.setPointSize(font.pointSize() - 2);
        painter->setFont(font);
        painter->setPen(QColor(32, 32, 32));
        painter->setBrush(Qt::NoBrush);
        painter->setRenderHint(QPainter::TextAntialiasing, true);
        painter->drawText(r.adjusted(1, 1, 1, 1), Qt::AlignVCenter, m_label);
        painter->setPen(Qt::white);
        painter->drawText(r, Qt::AlignVCenter, m_label);
    }
}
