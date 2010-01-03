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
#include <QPainter>
#include <QPalette>
#include <QStyledItemDelegate>

namespace Inspector {
namespace Internal {

class TemperatureResultsDelegate : public QStyledItemDelegate
{
public:
    TemperatureResultsDelegate(QObject *parent = 0)
      : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        // get the TemperatureItem
        const PaintingModel *model = static_cast<const PaintingModel *>(index.model());
        const TemperatureItem *item = model->result(index.row());
        if (!item) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        // draw the TemperatureItem
        const QRect rect = option.rect;
        const int rt = rect.top();
        const int rl = rect.left();
        const int textRectHeight = (rect.height() - 10) / 3;

        // selection
        QColor textColor = option.palette.color(QPalette::Text);
        if (option.state & QStyle::QStyle::State_Selected) {
            painter->fillRect(rect, option.palette.color(QPalette::Highlight));
            if (!(option.state & QStyle::State_MouseOver))
                textColor = option.palette.color(QPalette::HighlightedText);
        } else if (option.state & QStyle::State_MouseOver) {
            QColor color = option.palette.color(QPalette::Highlight);
            color.setAlpha(color.alpha() / 4);
            painter->fillRect(rect, color);
        }
        QColor subtleTextColor = textColor;
        subtleTextColor.setAlpha(subtleTextColor.alpha() / 2);

        // preview pixmap
        const QPixmap preview = item->previewImage();
        painter->drawPixmap(rl + (88 - preview.width()) / 2, rt + (rect.height() - preview.height()) / 2, preview);

        // text: date + duration
        QFont normalFont = option.font;
        QFont smallFont = normalFont;
        smallFont.setPointSize(smallFont.pointSize() - 1);
        painter->setFont(normalFont);
        painter->setPen(textColor);
        painter->drawText(QRect(rl + 88, rt + 5, rect.width() - 88, textRectHeight), Qt::AlignVCenter, item->date().toString());
        int minutes = (int)(item->duration() / 60.0);
        int seconds = (int)(item->duration() - (minutes * 60));
        QString timeString = tr("%1' %2'' ").arg(minutes).arg(seconds);
        painter->setFont(smallFont);
        painter->drawText(QRect(rl + 88, rt + 5, rect.width() - 88, textRectHeight), Qt::AlignVCenter | Qt::AlignRight, timeString);

        // text: description
        painter->setFont(normalFont);
        painter->drawText(QRect(rl + 88, rt + 5 + textRectHeight, rect.width() - 88, textRectHeight), Qt::AlignVCenter, item->description());

        // text: options
        painter->setFont(smallFont);
        painter->setPen(subtleTextColor);
        painter->drawText(QRect(rl + 88, rt + 5 + 2 * textRectHeight, rect.width() - 88, textRectHeight), Qt::AlignVCenter, item->options());
    }

    QSize sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
    {
        return QSize(TemperatureItem::previewWidth + 150, TemperatureItem::previewHeight + 8);
    }
};


PaintTemperatureView::PaintTemperatureView(PaintingModule *parentModule)
  : AbstractView(parentModule)
{
    setupUi(this);

    // wire-up controls
    connect(passesBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckIterations()));
    connect(lowBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckIterations()));
    connect(highBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckIterations()));
    connect(passesBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckWeight()));
    connect(innerBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckWeight()));
    connect(widthBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckWeight()));
    connect(heightBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckWeight()));
    connect(defaultsButton, SIGNAL(clicked()), this, SLOT(slotLoadDefaults()));
    connect(runButton, SIGNAL(clicked()), this, SLOT(slotTestClicked()));
    imageScrollArea->setWidget(imageLabel);

    // wire-up the results listview
    connect(resultsView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotResultActivated(QModelIndex)));
    resultsView->setItemDelegate(new TemperatureResultsDelegate(resultsView));
    resultsView->setModel(parentModule->model());
    resultsView->setRootIndex(parentModule->model()->resultsTableIndex());

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
    QPalette transPal;
    transPal.setColor(QPalette::Base, Qt::transparent);
    resultsView->setPalette(transPal);

    // init fields
    slotLoadDefaults();

    // listen for model changes
    connect(parentModule->model(), SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(slotModelItemChanged()));
}

void PaintTemperatureView::slotCheckIterations()
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

void PaintTemperatureView::slotCheckWeight()
{
    qreal pops = 100 * (qreal)passesBox->value() * (qreal)innerBox->value() / (qreal)(widthBox->value() * heightBox->value());
    popsBox->setText(tr("%1%").arg(QString::number(pops)));
}

void PaintTemperatureView::slotLoadDefaults()
{
    passesBox->setValue(5);
    lowBox->setValue(1);
    highBox->setValue(2);
    innerBox->setValue(4);
    widthBox->setValue(10);
    heightBox->setValue(10);
}

void PaintTemperatureView::slotTestClicked()
{
    // Build the args list: passes << headDrops << tailDrops << innerPasses << chunkWidth << chunkHeight << consoleDebug
    QVariantList args;
    args << passesBox->value() << lowBox->value() << highBox->value() << innerBox->value() << widthBox->value() << heightBox->value() << debugBox->isChecked();
    parentModule()->parentInstance()->model()->callProbeFunction("qWindowTemperature", args);
}

void PaintTemperatureView::slotModelItemChanged()
{
    PaintingModel *model = static_cast<PaintingModule *>(parentModule())->model();

}

void PaintTemperatureView::slotResultActivated(const QModelIndex &index)
{
    PaintingModel *model = static_cast<PaintingModule *>(parentModule())->model();
    const TemperatureItem *item = model->result(index.row());
    if (!item || item->image().isNull())
        return;
    imageLabel->setFixedSize(item->image().size());
    imageLabel->setPixmap(item->image());
    resultsTabWidget->setCurrentIndex(1);
    imageScrollArea->setFocus();
}

} // namespace Internal
} // namespace Inspector
