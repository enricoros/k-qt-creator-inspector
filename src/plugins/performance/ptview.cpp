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

#include "ptview.h"

#include "performancemanager.h"

#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>

using namespace Performance::Internal;

PaintTemperatureView::PaintTemperatureView(QWidget *parent)
  : QWidget(parent)
{
    setupUi(this);
    connect(passesBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckPasses()));
    connect(lowBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckPasses()));
    connect(highBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckPasses()));
    connect(passesBox, SIGNAL(valueChanged(int)), this, SLOT(slotUpdatePops()));
    connect(innerBox, SIGNAL(valueChanged(int)), this, SLOT(slotUpdatePops()));
    connect(widthBox, SIGNAL(valueChanged(int)), this, SLOT(slotUpdatePops()));
    connect(heightBox, SIGNAL(valueChanged(int)), this, SLOT(slotUpdatePops()));

    // apply the smaller font to a label
    QFont smallerFont = disclaimerLabel->font();
    smallerFont.setPointSize(smallerFont.pointSize() - 1);
    disclaimerLabel->setFont(smallerFont);
    samplesLabel->setFont(smallerFont);
    samplesBox->setFont(smallerFont);
    popsLabel->setFont(smallerFont);
    popsBox->setFont(smallerFont);

    // init fields
    on_defaultsButton_clicked();
}

void Performance::Internal::PaintTemperatureView::on_defaultsButton_clicked()
{
    passesBox->setValue(5);
    lowBox->setValue(1);
    highBox->setValue(2);
    innerBox->setValue(4);
    widthBox->setValue(10);
    heightBox->setValue(10);
}

void Performance::Internal::PaintTemperatureView::on_runButton_clicked()
{
    // Build the args list: passes << headDrops << tailDrops << innerPasses << chunkWidth << chunkHeight << consoleDebug
    QVariantList args;
    args << passesBox->value() << lowBox->value() << highBox->value() << innerBox->value() << widthBox->value() << heightBox->value() << debugBox->isChecked();
    // execute the probe
    PerformanceManager::instance()->defaultServerCallFunction("qWindowTemperature", args);
    newTestGroup->setEnabled(false);
}

void Performance::Internal::PaintTemperatureView::slotCheckPasses()
{
    int count = passesBox->value() - lowBox->value() - highBox->value();
    QPalette pal = palette();
    if (count < 1) {
        pal.setBrush(QPalette::WindowText, Qt::red);
        samplesBox->setText(tr("invalid"));
        runButton->setEnabled(false);
    } else {
        samplesBox->setText(tr("%1").arg(count));
        runButton->setEnabled(true);
    }
    samplesBox->setPalette(pal);
}

void Performance::Internal::PaintTemperatureView::slotUpdatePops()
{
    qreal pops = 100 * (qreal)passesBox->value() * (qreal)innerBox->value() / (qreal)(widthBox->value() * heightBox->value());
    popsBox->setText(tr("%1%").arg(QString::number(pops)));
}
