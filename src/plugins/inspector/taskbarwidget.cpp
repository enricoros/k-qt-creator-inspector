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

#include "taskbarwidget.h"

#include "taskswidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QToolButton>

using namespace Inspector::Internal;

TaskbarWidget::TaskbarWidget(QWidget *parent)
  : QWidget(parent)
  , m_shadowTile(0)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    QPalette pal;
    pal.setBrush(QPalette::Window, QColor(100, 100, 100));
    pal.setBrush(QPalette::WindowText, Qt::white);
    pal.setBrush(QPalette::Base, Qt::transparent);
    setPalette(pal);
    setAutoFillBackground(true);

    QLabel * l1 = new QLabel(tr("Task Graph:"), this);
    TasksWidget * w1 = new TasksWidget(this);
    QToolButton * b1 = new QToolButton(this);
    b1->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    b1->setText(tr("Sample Task 1"));
    b1->setIcon(QIcon(":/inspector/images/status-err.png"));
    QToolButton * b2 = new QToolButton(this);
    b2->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    b2->setText(tr("Sample Task 2"));
    b2->setIcon(QIcon(":/inspector/images/status-err.png"));

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->setContentsMargins(9, 2, 9, 1);
    layout->addWidget(l1, 0);
    layout->addWidget(w1, 1);
    layout->addWidget(b1, 0);
    layout->addWidget(b2, 0);
    layout->addStretch(1);
}

void TaskbarWidget::clear()
{
    qWarning("TaskbarWidget::clear: TODO");
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

void TaskbarWidget::paintEvent(QPaintEvent * event)
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
