/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>        *
 *   Started on 13 Jul 2009 by root.
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "performanceplot.h"
#include <QPainter>

using namespace Performance::Internal;

PerformancePlot::PerformancePlot(QWidget *parent)
  : QWidget(parent)
  , m_backRenderer(QString(":/performance/images/plot-background.svg"))
{

}

void PerformancePlot::paintEvent(QPaintEvent */*event*/)
{
    QPainter p(this);
    m_backRenderer.render(&p);
}
