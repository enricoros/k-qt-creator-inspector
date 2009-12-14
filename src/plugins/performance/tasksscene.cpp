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

#include "tasksscene.h"

#include "taskswidget.h"

#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <QGraphicsSceneMouseEvent>
#include <QScrollBar>
#include <QTimerEvent>
#include <QTimer>

namespace Performance {
namespace Internal {

TasksScene::TasksScene(QObject * parent)
  : QGraphicsScene(parent)
  , m_scrollLocked(true)
  , m_pixelPerSecond(10)
{
    m_updateTimer.start(100, this);
}

int TasksScene::fixedHeight()
{
    return 20;
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

void TasksScene::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    int sceneWidth = sceneRect().width();
    if (sceneWidth < 1)
        return;
    qreal scrollPercent = event->scenePos().x() / (qreal)sceneWidth;
    if (scrollPercent < 0.95) {
        m_scrollLocked = false;
        TasksWidget * widget = dynamic_cast<TasksWidget *>(views().first());
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

    if (m_startTime.isNull()) {
        m_startTime.start();
        return;
    }

    updateCurrentScene();
}

void TasksScene::regenScene()
{

}

class TaskItem : public QGraphicsWidget
{
    public:
        TaskItem(int start, QGraphicsItem * parent = 0)
          : QGraphicsWidget(parent)
        {
            setPos(start, 2);
            resize(1, TasksScene::fixedHeight() - 4);
            m_brush = QColor::fromHsv(qrand() % 360, 255, 180, 128);
        }

        void setEnd(int end)
        {
            resize(end - (int)x(), TasksScene::fixedHeight() - 4);
            int pInc = qrand() % 4;
            if (m_percs.isEmpty())
                m_percs.append(pInc);
            else
                m_percs.append(qMin(100, m_percs.last() + pInc));
            update();
        }

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0)
        {
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(QPen(Qt::lightGray, 1));
            painter->setBrush(m_brush);
            QRectF rect = boundingRect().adjusted(0.5, 0.5, -0.5, -0.5);
            if (rect.width() > 2) {
                painter->drawRoundedRect(boundingRect().adjusted(0.5, 0.5, -0.5, -0.5), 4, 4, Qt::AbsoluteSize);

                // draw the percent polygon
                painter->setBrush(Qt::NoBrush);
                painter->setPen(Qt::lightGray);
                int h = size().height();
                int x = 0;
                foreach (int val, m_percs) {
                    int y = h * (0.9 - 0.8*(qreal)val / 100.0);
                    painter->drawPoint(++x, y);
                }

                QRect r = boundingRect().toRect().adjusted(2, 1, -2, 0);
                if (r.width() > 2) {
                    QFont font = painter->font();
                    font.setPointSize(font.pointSize() - 2);
                    painter->setFont(font);
                    painter->setPen(Qt::black);
                    painter->setBrush(Qt::NoBrush);
                    painter->setRenderHint(QPainter::TextAntialiasing, true);
                    painter->drawText(r.adjusted(1, 1, 1, 1), Qt::AlignVCenter, tr("fake"));
                    painter->setPen(Qt::white);
                    painter->drawText(r, Qt::AlignVCenter, tr("fake"));
                }
            }
        }

    private:
        QBrush m_brush;
        QList<int> m_percs;
};

void TasksScene::updateCurrentScene()
{
    if (!m_startTime.isValid())
        return;
    int contentsWidth = (m_startTime.elapsed() * m_pixelPerSecond) / 1000;
    setSceneRect(0, 0, contentsWidth, fixedHeight());
    update();

    // if locked scrolling, update view's scrollbar too
    if (m_scrollLocked && !views().isEmpty()) {
        TasksWidget * widget = dynamic_cast<TasksWidget *>(views().first());
        widget->horizontalScrollBar()->setValue(contentsWidth);
    }

    // ### HACK AHEAD

    // RANDOM TASK DELETION
    if ((qrand() % 50) == 42 && !m_currentTasks.isEmpty()) {
        TaskItem * i = m_currentTasks.takeAt(qrand() % m_currentTasks.size());
        (void)i;
        //removeItem(i);
        //delete i;
    }

    // TASK UPDATE
    foreach (TaskItem * item, m_currentTasks)
        item->setEnd(contentsWidth);

    // RANDOM TASK ACTIVATION
    static bool first = true;
    if (first || (qrand() % 70) == 42) {
        first = false;
        TaskItem * i = new TaskItem(contentsWidth);
        addItem(i);
        m_currentTasks.append(i);
    }
}

} // namespace Internal
} // namespace Performance
