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
#include <QScrollBar>
#include <QTimerEvent>
#include <QTimer>

using namespace Performance::Internal;

TasksScene::TasksScene(QObject * parent)
  : QGraphicsScene(parent)
  , m_scrollLocked(true)
  , m_pixelPerSecond(100)
{
    m_updateTimer.start(50, this);
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
}

