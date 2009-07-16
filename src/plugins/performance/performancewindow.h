/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Started on 14 Jul 2009 by root.                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PERFORMANCEWINDOW_H
#define PERFORMANCEWINDOW_H

#include <QWidget>
#include "ui_performancewindow.h"

namespace Performance {
namespace Internal {

class PerformanceWindow : public QWidget, public Ui::PerformanceWindow
{
    Q_OBJECT

public:
    PerformanceWindow(QWidget *parent = 0);
    ~PerformanceWindow();

    //void setCentralWidget(QWidget *);
    //QWidget *centralWidget() const;
};

} // namespace Internal
} // namespace Performance

#endif // PERFORMANCEWINDOW_H
