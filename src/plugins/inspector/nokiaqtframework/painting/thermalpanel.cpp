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

#include "thermalpanel.h"
#include "iframework.h"
#include "iinspectionmodel.h"
#include "paintingmodule.h"
#include "thermalmodel.h"

#include "../datautils.h"

#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QStyledItemDelegate>

#if defined(INSPECTOR_PAINTING_VTK)
#include "thermal3danalysis.h"
#endif

using namespace Inspector::Internal;

class Inspector::Internal::ThermalItemDelegate : public QStyledItemDelegate
{
public:
    ThermalItemDelegate(QObject *parent = 0)
      : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        // get the ThermalItem
        const ThermalModel *model = static_cast<const ThermalModel *>(index.model());
        const ThermalItem *item = model->result(index.row());
        if (!item) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        // draw the ThermalItem
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
        painter->drawText(QRect(rl + 88, rt + 5, rect.width() - 88, textRectHeight), Qt::AlignVCenter, item->startDate().toString());
        int minutes = (int)(item->duration() / 60.0);
        int seconds = (int)(item->duration() - (minutes * 60));
        QString timeString = tr("%1' %2'' ").arg(minutes).arg(seconds);
        painter->setFont(smallFont);
        painter->drawText(QRect(rl + 88, rt + 5, rect.width() - 88, textRectHeight), Qt::AlignVCenter | Qt::AlignRight, timeString);

        // text: description
        painter->setFont(normalFont);
        painter->drawText(QRect(rl + 88, rt + 5 + textRectHeight, rect.width() - 88, textRectHeight), Qt::AlignVCenter, item->label());

        // text: options
        painter->setFont(smallFont);
        painter->setPen(subtleTextColor);
        painter->drawText(QRect(rl + 88, rt + 5 + 2 * textRectHeight, rect.width() - 88, textRectHeight), Qt::AlignVCenter, item->options());
    }

    QSize sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
    {
        return QSize(ThermalItem::previewWidth + 150, ThermalItem::previewHeight + 8);
    }
};

//
// ThermalPanel
//
ThermalPanel::ThermalPanel(PaintingModule *module)
  : AbstractPanel(module)
  , m_paintingModule(module)
{
    setupUi(this);

#if defined(INSPECTOR_PAINTING_VTK)
    Thermal3DAnalysis *tView = new Thermal3DAnalysis(m_paintingModule);
    resultsTabWidget->addTab(tView, tr("Surface Analysis"));
#endif

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
    connect(exportButton, SIGNAL(clicked()), this, SLOT(slotExportClicked()));
    imageScrollArea->setWidget(imageLabel);

    // wire-up the results listview
    connect(resultsView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotResultActivated(QModelIndex)));
    resultsView->setItemDelegate(new ThermalItemDelegate(resultsView));
    resultsView->setModel(m_paintingModule->thermalModel());
    resultsView->setRootIndex(m_paintingModule->thermalModel()->resultsTableIndex());

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
    connect(m_paintingModule->thermalModel(), SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(slotModelItemChanged()));
    slotModelItemChanged();
}

void ThermalPanel::slotCheckIterations()
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

void ThermalPanel::slotCheckWeight()
{
    qreal pops = 100 * (qreal)passesBox->value() * (qreal)innerBox->value() / (qreal)(widthBox->value() * heightBox->value());
    popsBox->setText(tr("%1%").arg(QString::number(pops)));
}

void ThermalPanel::slotLoadDefaults()
{
    passesBox->setValue(5);
    lowBox->setValue(1);
    highBox->setValue(2);
    innerBox->setValue(4);
    widthBox->setValue(10);
    heightBox->setValue(10);
}

void ThermalPanel::slotTestClicked()
{
    // Build the options list: passes, headDrops, tailDrops, innerPasses, chunkWidth, chunkHeight, consoleDebug
    QVariantList options;
    options << passesBox->value() << lowBox->value() << highBox->value() << innerBox->value() << widthBox->value() << heightBox->value() << debugBox->isChecked();

    // start the test, we'll watch the results in the model
    m_paintingModule->startThermalTest(testNameLabel->text(), options);
}

void ThermalPanel::slotModelItemChanged()
{
    ThermalModel *model = static_cast<PaintingModule *>(parentModule())->thermalModel();
    int value = model->ptProgress();
    ptProgress->setValue(value);
    ptProgress->setVisible(value > 0 && value < 100);
}

void ThermalPanel::slotResultActivated(const QModelIndex &index)
{
    ThermalModel *model = static_cast<PaintingModule *>(parentModule())->thermalModel();
    const ThermalItem *item = model->result(index.row());
    if (!item || item->image().isNull())
        return;
    imageLabel->setFixedSize(item->image().size());
    imageLabel->setPixmap(item->image());
    resultsTabWidget->setCurrentIndex(1);
    imageScrollArea->setFocus();
}

void ThermalPanel::slotExportClicked()
{
    qint32 test[100];
    DataUtils::exportOctaveArray<qint32>("test", "A", 13, 7, test);
}
