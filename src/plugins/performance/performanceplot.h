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

#ifndef PERFORMANCEPLOT_H
#define PERFORMANCEPLOT_H

#include <QWidget>
#include <QSvgRenderer>

namespace Performance {
namespace Internal {

class PerformancePlot : public QWidget
{
    Q_OBJECT

public:
    PerformancePlot(QWidget *parent = 0);

    // completely TODO

    void paintEvent(QPaintEvent *event);

private:
    QSvgRenderer m_backRenderer;

};

} // namespace Internal
} // namespace Performance

#endif // PERFORMANCEPLOT_H
