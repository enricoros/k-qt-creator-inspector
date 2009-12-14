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
    p//newTestGroup->setEnabled(false);
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
