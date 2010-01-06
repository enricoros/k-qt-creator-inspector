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

#include "statusbarwidget.h"
#include "tasksmodel.h"
#include "taskswidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QToolButton>

using namespace Inspector::Internal;

KillTaskButton::KillTaskButton(quint32 tid, QWidget *parent)
  : QToolButton(parent)
  , m_tid(tid)
{
}

quint32 KillTaskButton::tid() const
{
    return m_tid;
}

StatusBarWidget::StatusBarWidget(QWidget *parent)
  : QWidget(parent)
  , m_shadowTile(0)
  , m_model(0)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    QPalette pal;
    pal.setBrush(QPalette::Window, QColor(100, 100, 100));
    pal.setBrush(QPalette::WindowText, Qt::white);
    pal.setBrush(QPalette::Base, Qt::transparent);
    setPalette(pal);
    setAutoFillBackground(true);

    QLabel * l1 = new QLabel(tr("Task Graph (test)"), this);
    TasksWidget * w1 = new TasksWidget(this);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(9, 2, 9, 1);
    m_layout->addWidget(l1, 0);
    m_layout->addWidget(w1, 1);
    m_layout->addStretch(1);
}

void StatusBarWidget::setTasksModel(TasksModel *model)
{
    if (m_model) {
        // forget previous model
        disconnect(m_model, 0, this, 0);
    }
    m_model = model;
    if (m_model) {
        // use current model
        connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(slotTasksChanged()));
        connect(m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(slotTasksChanged()));
        connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotTasksChanged()));
       m_model->addTask(1, "uno", "un");
       m_model->addTask(2, "tre", "t");
       m_model->addTask(4, "qua", "q");
    }
}

static void drawVerticalShadow(QPainter * painter, int width, int height)
{
    QLinearGradient lg( 0, 0, 0, height );
    lg.setColorAt( 0.0, QColor( 0, 0, 0, 64 ) );
    lg.setColorAt( 0.4, QColor( 0, 0, 0, 16 ) );
    lg.setColorAt( 0.7, QColor( 0, 0, 0, 5 ) );
    lg.setColorAt( 1.0, QColor( 0, 0, 0, 0 ) );
    painter->fillRect( 0, 0, width, height, lg );
}

void StatusBarWidget::paintEvent(QPaintEvent * event)
{
    // the first time create the Shadow Tile
    if (!m_shadowTile) {
        m_shadowTile = new QPixmap(64, 8);
        m_shadowTile->fill(Qt::transparent);
        QPainter shadowPainter(m_shadowTile);
        drawVerticalShadow(&shadowPainter, 64, 8);
    }

    // draw dubtle shadow
    if (event->rect().top() < 8) {
        QRect shadowRect = event->rect();
        shadowRect.setTop(0);
        shadowRect.setHeight(8);
        QPainter p(this);
        p.drawTiledPixmap(shadowRect, *m_shadowTile);
    }
}

void StatusBarWidget::slotTasksChanged()
{
    // look for rows added under the tasks table parent
    //if (parent != m_model->tasksTableIndex())
    //    return;

    QList<quint32> tasks = m_model->activeTasksId();

    // delete exceeding buttons
    QList<KillTaskButton *>::iterator it = m_buttons.begin();
    while (it != m_buttons.end()) {
        KillTaskButton *button = *it;
        quint32 tid = button->tid();
        if (!tasks.contains(tid)) {
            it = m_buttons.erase(it);
            delete button;
            continue;
        }
        tasks.removeAll(tid);
        ++it;
    }

    // create new buttons
    foreach (quint32 tid, tasks) {
        QString taskName = m_model->taskName(tid);
        KillTaskButton *button = new KillTaskButton(tid, this);
        connect(button, SIGNAL(clicked()), this, SLOT(slotKillTask()), Qt::QueuedConnection);
        button->setText(tr("Kill %1").arg(taskName));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setIcon(QIcon(":/inspector/images/status-ok.png"));
        m_buttons.append(button);
        m_layout->insertWidget(m_layout->count() - 1, button);
    }
}

void StatusBarWidget::slotKillTask()
{
    KillTaskButton *button = static_cast<KillTaskButton *>(sender());
    quint32 taskId = button->tid();
    // don't use 'button' after this line, because it should be deleted!
    m_model->killTask(taskId);
}
