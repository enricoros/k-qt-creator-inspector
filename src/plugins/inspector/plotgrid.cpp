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

#include "plotgrid.h"
#include <QPainter>
#include <QPen>

using namespace Inspector::Internal;

PlotGrid::PlotGrid(QWidget *parent)
  : QWidget(parent)
  , m_backRenderer(QString(":/inspector/images/plot-background.svg"))
{
}

void PlotGrid::paintEvent(QPaintEvent */*event*/)
{
    QPainter p(this);
    m_backRenderer.render(&p);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(Qt::lightGray));
    for (qreal x = 0.1; x <= 0.9; x += 0.1) {
        qreal sx = x * (qreal)width();
        qreal sy = (qreal)height();
        p.drawLine(sx, 0, sx, sy);
    }
    for (qreal y = 0.2; y <= 0.8; y += 0.2) {
        qreal sx = (qreal)width();
        qreal sy = y * (qreal)height();
        p.drawLine(0, sy, sx, sy);
    }
}
