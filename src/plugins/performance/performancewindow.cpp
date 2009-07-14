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

#include "performancewindow.h"
#include "ui_performancewindow.h"

using namespace Performance::Internal;

PerformanceWindow::PerformanceWindow(QWidget *parent)
  : QWidget(parent)
  , m_ui(new Ui::PerformanceWindow)
  , m_centralWidget(0)
{
    m_ui->setupUi(this);
    m_ui->eventSectButton->setChecked(true);
}

PerformanceWindow::~PerformanceWindow()
{
    delete m_ui;
}

void PerformanceWindow::setCentralWidget(QWidget *widget)
{
    delete m_centralWidget;
    m_centralWidget = widget;

    foreach (QObject * child, m_ui->mainFrame->children())
        delete child;
    m_centralWidget->setParent(m_ui->mainFrame);

}

QWidget * PerformanceWindow::centralWidget() const
{
    return m_centralWidget;
}
