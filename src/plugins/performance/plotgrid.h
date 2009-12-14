/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Started on 13 Jul 2009 by root.                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PLOTGRID_H
#define PLOTGRID_H

#include <QWidget>
#include <QSvgRenderer>

namespace Performance {
namespace Internal {

class PlotGrid : public QWidget
{
    Q_OBJECT

public:
    PlotGrid(QWidget *parent = 0);

    // completely TODO

    void paintEvent(QPaintEvent *event);

private:
    QSvgRenderer m_backRenderer;

};

} // namespace Internal
} // namespace Performance

#endif // PLOTGRID_H
