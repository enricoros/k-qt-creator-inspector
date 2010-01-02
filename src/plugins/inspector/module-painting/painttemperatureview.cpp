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

#include "painttemperatureview.h"
#include "paintingmodel.h"
#include "paintingmodule.h"
#include "instance.h"
#include <QApplication>
#include <QIcon>
#include <QLabel>
#include <QPalette>
#include <QPixmap>
#include <QPushButton>

using namespace Inspector::Internal;

PaintTemperatureView::PaintTemperatureView(PaintingModule *parentModule)
  : AbstractView(parentModule)
{
    setupUi(this);
    connect(passesBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckPasses()));
    connect(lowBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckPasses()));
    connect(highBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckPasses()));
    connect(passesBox, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateWeight()));
    connect(innerBox, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateWeight()));
    connect(widthBox, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateWeight()));
    connect(heightBox, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateWeight()));
    connect(resultsView, SIGNAL(activated(QModelIndex)), this, SLOT(slotResultActivated(QModelIndex)));

    // change looks
    QFont smallerFont = samplesLabel->font();
    smallerFont.setPointSize(smallerFont.pointSize() - 1);
    QPalette lighterPal;
    lighterPal.setColor(QPalette::WindowText, QApplication::palette().color(QPalette::Mid));
    samplesLabel->setFont(smallerFont);
    samplesLabel->setPalette(lighterPal);
    samplesBox->setFont(smallerFont);
    samplesBox->setPalette(lighterPal);
    popsLabel->setFont(smallerFont);
    popsLabel->setPalette(lighterPal);
    popsBox->setFont(smallerFont);
    popsBox->setPalette(lighterPal);

    // init fields
    on_defaultsButton_clicked();

    // ###TEMP
    resultsView->setModel(parentModule->model());
    resultsView->setRootIndex(parentModule->model()->resultsTableIndex());
}

void PaintTemperatureView::on_defaultsButton_clicked()
{
    passesBox->setValue(5);
    lowBox->setValue(1);
    highBox->setValue(2);
    innerBox->setValue(4);
    widthBox->setValue(10);
    heightBox->setValue(10);
}

void PaintTemperatureView::on_runButton_clicked()
{
    // Build the args list: passes << headDrops << tailDrops << innerPasses << chunkWidth << chunkHeight << consoleDebug
    QVariantList args;
    args << passesBox->value() << lowBox->value() << highBox->value() << innerBox->value() << widthBox->value() << heightBox->value() << debugBox->isChecked();
    parentModule()->parentInstance()->model()->callProbeFunction("qWindowTemperature", args);
}

void PaintTemperatureView::slotCheckPasses()
{
    int count = passesBox->value() - lowBox->value() - highBox->value();
    QPalette pal = palette();
    if (count < 1) {
        pal.setBrush(QPalette::WindowText, Qt::red);
        samplesBox->setText(tr("invalid"));
        runButton->setEnabled(false);
    } else {
        pal.setColor(QPalette::WindowText, QApplication::palette().color(QPalette::Mid));
        samplesBox->setText(tr("%1").arg(count));
        runButton->setEnabled(true);
    }
    samplesBox->setPalette(pal);
}

void PaintTemperatureView::slotUpdateWeight()
{
    qreal pops = 100 * (qreal)passesBox->value() * (qreal)innerBox->value() / (qreal)(widthBox->value() * heightBox->value());
    popsBox->setText(tr("%1%").arg(QString::number(pops)));
}

void PaintTemperatureView::slotResultActivated(const QModelIndex &index)
{
    PaintingModel *model = static_cast<PaintingModule *>(parentModule())->model();
    const TemperatureItem *item = model->result(index.row());
    if (!item)
        return;

    QLabel *label = new QLabel();
    label->setWindowIcon(QIcon(":/inspector/images/menu-icon.png"));
    label->setWindowTitle(tr("Image Viewer - Painting Temperature"));
    label->setFixedSize(item->image().size());
    label->setPixmap(item->image());
    label->show();
}
