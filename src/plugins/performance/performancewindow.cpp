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

using namespace Performance::Internal;

PerformanceWindow::PerformanceWindow(QWidget *parent)
  : QWidget(parent)
{
    setupUi(this);

    connect(generalSectButton, SIGNAL(toggled(bool)), this, SLOT(slotSetGeneral(bool)));
    connect(eventSectButton, SIGNAL(toggled(bool)), this, SLOT(slotSetEvent(bool)));
    connect(editSectButton, SIGNAL(toggled(bool)), this, SLOT(slotSetEdit(bool)));
    connect(paintingSectButton, SIGNAL(toggled(bool)), this, SLOT(slotSetPainting(bool)));
    connect(timersSectButton, SIGNAL(toggled(bool)), this, SLOT(slotSetTimers(bool)));
    connect(networkSectButton, SIGNAL(toggled(bool)), this, SLOT(slotSetNetwork(bool)));
    connect(inputSectButton, SIGNAL(toggled(bool)), this, SLOT(slotSetInput(bool)));
    connect(parallelSectButton, SIGNAL(toggled(bool)), this, SLOT(slotSetParallel(bool)));

    generalSectButton->setChecked(true);
}

PerformanceWindow::~PerformanceWindow()
{
}

void PerformanceWindow::slotSetGeneral(bool on)
{
    if (on)
        mainStackWidget->setCurrentIndex(0);
}

void PerformanceWindow::slotSetEvent(bool on)
{
    if (on)
        mainStackWidget->setCurrentIndex(1);
}

void PerformanceWindow::slotSetEdit(bool on)
{
    if (on)
        mainStackWidget->setCurrentIndex(2);
}

void PerformanceWindow::slotSetPainting(bool on)
{
    if (on)
        mainStackWidget->setCurrentIndex(3);
}

void PerformanceWindow::slotSetTimers(bool on)
{
    if (on)
        mainStackWidget->setCurrentIndex(4);
}

void PerformanceWindow::slotSetNetwork(bool on)
{
    if (on)
        mainStackWidget->setCurrentIndex(5);
}

void PerformanceWindow::slotSetInput(bool on)
{
    if (on)
        mainStackWidget->setCurrentIndex(6);
}

void PerformanceWindow::slotSetParallel(bool on)
{
    if (on)
        mainStackWidget->setCurrentIndex(7);
}
