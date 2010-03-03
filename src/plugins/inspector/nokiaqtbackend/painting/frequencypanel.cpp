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

#include "frequencypanel.h"
#include "paintingmodule.h"

#include <QtGui/QFileDialog>

using namespace Inspector::Internal;

FrequencyPanel::FrequencyPanel(PaintingModule *module)
  : AbstractPanel(module)
  , m_paintingModule(module)
{
    setupUi(this);

    // wire-up controls
    connect(runButton, SIGNAL(clicked()), this, SLOT(slotStartTestClicked()));
    connect(viewButton, SIGNAL(clicked()), this, SLOT(slotDisplayClicked()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(slotRemoveClicked()));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(slotClearClicked()));
    connect(exportButton, SIGNAL(clicked()), this, SLOT(slotExportClicked()));
    connect(importButton, SIGNAL(clicked()), this, SLOT(slotImportClicked()));
}

void FrequencyPanel::slotStartTestClicked()
{
    m_paintingModule->startFrequencyTest();
}

void FrequencyPanel::slotDisplayClicked()
{
    foreach (const QModelIndex &index, resultsView->selectionModel()->selectedIndexes()) {
        Q_UNUSED(index);
    }
}

void FrequencyPanel::slotRemoveClicked()
{
    while (resultsView->selectionModel()->hasSelection()) {
        QModelIndex index = resultsView->selectionModel()->selectedRows().first();
        resultsView->model()->removeRow(index.row(), index.parent());
    }
}

void FrequencyPanel::slotClearClicked()
{
}

void FrequencyPanel::slotExportClicked()
{
    // ask the file name
    const QString fileName = QFileDialog::getSaveFileName(this, tr("Export Frequency File"),
        "untitled.frequency", tr("Frequency Files (*.frequency)"));
    if (!fileName.isEmpty()) {
        Q_UNUSED(fileName);
    }
}

void FrequencyPanel::slotImportClicked()
{
    // ask the file name
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Export Frequency File"),
        "untitled.frequency", tr("Frequency Files (*.frequency)"));
    if (!fileName.isEmpty()) {
        Q_UNUSED(fileName);
    }
}
