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

namespace Ui {
    class PerformanceWindow;
}

namespace Performance {
namespace Internal {

class PerformanceWindow : public QWidget
{
    Q_OBJECT

public:
    PerformanceWindow(QWidget *parent = 0);
    ~PerformanceWindow();

    void setCentralWidget(QWidget *);
    QWidget *centralWidget() const;

private:
    Ui::PerformanceWindow *m_ui;
    QWidget *m_centralWidget;
};

} // namespace Internal
} // namespace Performance

#endif // PERFORMANCEWINDOW_H
