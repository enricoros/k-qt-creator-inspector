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
#include "paintingmodule.h"
#include "thermalmodel.h"

#include <QtGui/QAction>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QMenu>
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
        const QPixmap preview = item->previewPixmap();
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
  , m_thermalModel(module->thermalModel())
{
    setupUi(this);

    Thermal3DOnDemand *onDemand = new Thermal3DOnDemand(m_paintingModule, false /*TODO: use Settings*/);
    resultsTabWidget->addTab(onDemand, tr("Surface Analysis"));

    // wire-up controls
    connect(presetCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotActivatePreset(int)));
    connect(testNameLabel, SIGNAL(textChanged(QString)), this, SLOT(slotValidateTestLabel()));

    connect(passesBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckIterations()));
    connect(lowBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckIterations()));
    connect(highBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckIterations()));

    connect(passesBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckWeight()));
    connect(innerBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckWeight()));
    connect(widthBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckWeight()));
    connect(heightBox, SIGNAL(valueChanged(int)), this, SLOT(slotCheckWeight()));

    connect(runButton, SIGNAL(clicked()), this, SLOT(slotRunThermalClicked()));
    connect(viewButton, SIGNAL(clicked()), this, SLOT(slotDisplayClicked()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(slotRemoveClicked()));
    connect(resultButton, SIGNAL(clicked()), this, SLOT(slotResultButtonClicked()));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(slotClearClicked()));
    connect(exportButton, SIGNAL(clicked()), this, SLOT(slotExportClicked()));
    connect(importButton, SIGNAL(clicked()), this, SLOT(slotImportClicked()));
    imageScrollArea->setWidget(imageLabel);

    // wire-up the results listview
    resultsView->setItemDelegate(new ThermalItemDelegate(resultsView));
    resultsView->setModel(m_thermalModel);
    resultsView->setRootIndex(m_thermalModel->resultsTableIndex());
    resultsView->setContextMenuPolicy(Qt::CustomContextMenu);

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
    splitter->setSizes(QList<int>() << 100 << 700);

    // listen for model changes
    connect(m_thermalModel, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(slotModelItemChanged()));
    connect(resultsView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotViewSelectionChanged()));
    connect(resultsView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotViewContextMenuRequested(QPoint)));
    connect(resultsView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(slotDisplayResultImage(QModelIndex)));
    slotModelItemChanged();
    slotViewSelectionChanged();

    // init fields
    slotActivatePreset(presetCombo->currentIndex());
    slotValidateTestLabel();
    if (!runButton->isEnabled())
        slotRegenLabel();
}

QString ThermalPanel::helpHtml() const
{
    return tr("From this panel you can start data acquisition from the target.<br/>From the <i>Options</i> combo choose a trade-off between performance and precision, then set a title in the <i>Label</i> field and run the test by pressing the <i>Test</i>button.<br/><br/>The results will appear when the test is done and can be viewed as a colored map or as a 3D surface.<br/><br/>The more the 'heat' or 'height' of the pixel, the more heavy and energy consumptive the operation is.");
}

void ThermalPanel::slotActivatePreset(int comboIndex)
{
    groupBox->setVisible(comboIndex >= 4);
    switch (comboIndex) {
    case 0: // fast
        passesBox->setValue(4);
        lowBox->setValue(0);
        highBox->setValue(2);
        innerBox->setValue(2);
        widthBox->setValue(20);
        heightBox->setValue(20);
        break;
    case 1: // default
        passesBox->setValue(5);
        lowBox->setValue(1);
        highBox->setValue(2);
        innerBox->setValue(4);
        widthBox->setValue(10);
        heightBox->setValue(10);
        break;
    case 2: // accurate
        passesBox->setValue(7);
        lowBox->setValue(1);
        highBox->setValue(3);
        innerBox->setValue(5);
        widthBox->setValue(5);
        heightBox->setValue(5);
        break;
    case 3: // high resolution
        passesBox->setValue(10);
        lowBox->setValue(1);
        highBox->setValue(4);
        innerBox->setValue(5);
        widthBox->setValue(1);
        heightBox->setValue(1);
        break;
    case 4: // custom
    case 5: // [DEV debug]
        // keep the actual values
        break;
    }
}

void ThermalPanel::slotValidateTestLabel()
{
    if (testNameLabel->text().isEmpty())
        runButton->setEnabled(false);
    else if (m_thermalModel->containsResultLabel(testNameLabel->text()))
        runButton->setEnabled(false);
    else
        runButton->setEnabled(true);
}

void ThermalPanel::slotRegenLabel()
{
    QString label = testNameLabel->text();
    int antiLoop = 999;
    while (antiLoop--) {
        // parse existing number (after the dash) if any
        int number = 0;
        int dashIndex = label.lastIndexOf(QLatin1String("#"));
        if (dashIndex != -1)
            number = label.mid(dashIndex + 1).toInt();

        // append the dash or increment the number
        if (!number)
            label.append(QLatin1String(" #1"));
        else
            label = label.left(dashIndex + 1).append(QString::number(number + 1));

        // test the string, if ok set it, else continue looping
        if (!m_thermalModel->containsResultLabel(label)) {
            testNameLabel->setText(label);
            break;
        }
    }
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

void ThermalPanel::slotRunThermalClicked()
{
    // Build the options list: passes, headDrops, tailDrops, innerPasses, chunkWidth, chunkHeight, consoleDebug
    bool consoleDebug = presetCombo->currentIndex() == 5;
    QVariantList options;
    options << passesBox->value() << lowBox->value() << highBox->value() << innerBox->value() << widthBox->value() << heightBox->value() << consoleDebug;

    // start the test, we'll watch the results in the model
    m_paintingModule->startThermalTest(testNameLabel->text(), options);

    slotRegenLabel();
}

void ThermalPanel::slotDisplayClicked()
{
    foreach (const QModelIndex &index, resultsView->selectionModel()->selectedIndexes()) {
        slotDisplayResultImage(index);
        break;
    }
}

void ThermalPanel::slotRemoveClicked()
{
    while (resultsView->selectionModel()->hasSelection()) {
        QModelIndex index = resultsView->selectionModel()->selectedRows().first();
        resultsView->model()->removeRow(index.row(), index.parent());
    }
}

void ThermalPanel::slotResultButtonClicked()
{
    slotViewContextMenuRequested(resultButton->pos());
}

void ThermalPanel::slotClearClicked()
{
    m_thermalModel->clearResults();
}

void ThermalPanel::slotExportClicked()
{
    // ask the file name
    const QString fileName = QFileDialog::getSaveFileName(this, tr("Export Thermal File"),
        "untitled.thermal", tr("Thermal Files (*.thermal)"));
    if (!fileName.isEmpty()) {
        if (!m_thermalModel->exportToFile(fileName, resultsView->selectionModel()->selectedIndexes()))
            exportButton->setEnabled(false);
    }
}

void ThermalPanel::slotImportClicked()
{
    // ask the file name
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Export Thermal File"),
        "untitled.thermal", tr("Thermal Files (*.thermal)"));
    if (!fileName.isEmpty())
        m_thermalModel->importFromFile(fileName);
}

void ThermalPanel::slotModelItemChanged()
{
    int value = m_thermalModel->ptProgress();
    ptProgress->setValue(value);
    ptProgress->setVisible(value > 0 && value < 100);
}

void ThermalPanel::slotViewSelectionChanged()
{
    QModelIndexList selection = resultsView->selectionModel()->selectedIndexes();
    bool empty = selection.isEmpty();
    bool single = selection.size() == 1;
    viewButton->setEnabled(single);
    exportButton->setEnabled(!empty);
    resultButton->setEnabled(single);
    removeButton->setEnabled(!empty);
    clearButton->setEnabled(m_thermalModel->resultsCount());
}

void ThermalPanel::slotViewContextMenuRequested(const QPoint &point)
{
    QMenu menu;
    menu.setTitle("Test Result");
    QAction *aRename = menu.addAction(tr("Rename..."));
    QAction *aSaveImage = menu.addAction(tr("Save Image..."));
    QAction *aSaveCm = menu.addAction(tr("Save Color Map..."));
    if (QAction *choice = menu.exec(resultsView->mapToGlobal(point))) {
        QModelIndex resultIndex = resultsView->currentIndex();

        // rename action
        if (choice == aRename) {
            QString name = QInputDialog::getText(this, tr("Rename Test Result"),
                tr("Enter the new name"), QLineEdit::Normal, m_thermalModel->resultName(resultIndex));
            if (!name.isEmpty())
                m_thermalModel->setResultName(name, resultIndex);
            return;
        }

        // export pixmap actions
        QPixmap pixmap;
        if (choice == aSaveImage)
            pixmap = m_thermalModel->resultOriginalPixmap(resultIndex);
        else if (choice == aSaveCm)
            pixmap = m_thermalModel->resultColoredPixmap(resultIndex);
        if (!pixmap.isNull()) {
            const QString fileName = QFileDialog::getSaveFileName(this, tr("Export Image File"),
                "untitled.png", tr("Images (*.png *.jpg *.bmp)"));
            if (!fileName.isEmpty())
                pixmap.save(fileName);
        }
    }
}

void ThermalPanel::slotDisplayResultImage(const QModelIndex &index)
{
    QPixmap pixmap = m_thermalModel->resultColoredPixmap(index);
    imageLabel->setFixedSize(pixmap.size());
    imageLabel->setPixmap(pixmap);
    resultsTabWidget->setCurrentIndex(1);
    imageScrollArea->setFocus();
}


//
// Thermal3DOnDemand
//
Thermal3DOnDemand::Thermal3DOnDemand(PaintingModule *module, bool useDepthPeeling, QWidget *parent)
  : QWidget(parent)
  , m_paintingModule(module)
  , m_useDepthPeeling(useDepthPeeling)
  , m_created(false)
{
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);
    lay->setSpacing(0);
    setLayout(lay);
}

void Thermal3DOnDemand::showEvent(QShowEvent *event)
{
    if (!m_created) {
        m_created = true;
#if defined(INSPECTOR_PAINTING_VTK)
        Thermal3DAnalysis *widget = new Thermal3DAnalysis(m_paintingModule, m_useDepthPeeling, this);
        layout()->addWidget(widget);
#else
        QLabel *widget = new QLabel(tr("Qt Creator was build without QTCREATOR_WITH_INSPECTOR_VTK"));
        layout()->addWidget(widget);
        layout()->setAlignment(widget, Qt::AlignCenter);
#endif
    }
    QWidget::showEvent(event);
}
