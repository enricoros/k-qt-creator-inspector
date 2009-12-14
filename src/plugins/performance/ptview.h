/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Started on 12 Jul 2009 by root.                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PTVIEW_H
#define PTVIEW_H

#include <QWidget>
#include "ui_ptview.h"

namespace Performance {
namespace Internal {

class PaintTemperatureView : public QWidget, public Ui::PTView
{
    Q_OBJECT

public:
    PaintTemperatureView(QWidget *parent = 0);

private:
    QPixmap m_pixmap;

private slots:
    void on_defaultsButton_clicked();
    void on_runButton_clicked();
    void slotCheckPasses();
    void slotUpdatePops();
};

} // namespace Internal
} // namespace Performance

#endif // PTVIEW_H
