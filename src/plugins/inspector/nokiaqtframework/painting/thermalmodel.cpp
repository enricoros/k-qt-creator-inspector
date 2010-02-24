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

#include "thermalmodel.h"
#include "../datautils.h"
#include <coreplugin/icore.h>
#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

using namespace Inspector::Internal;

/* == ThermalModel Usage ==
Row 'Results_Row':
  0: results count          int
  1: results                LIST(ThermalItem)
  2: saved                  bool

Row 'CurrentPt_Row'
  0: started                bool
  1: label                  string
  2: options                string
  3: progress               int (0..100)
*/

#define Results_Row 0
#define CurrentPt_Row 1

//
// ThermalItem
//
ThermalItem::ThermalItem(const QDateTime &dt, qreal duration, const QString &label, const QString &options,
                         const QImage &image, const Probe::RegularMeshRealData &mesh)
  : QStandardItem(label)
  , m_dateTime(dt)
  , m_duration(duration)
  , m_label(label)
  , m_options(options)
  , m_image(image)
  , m_mesh(mesh)
{
    setEditable(false);
    m_previewPixmap = coloredPixmap(false).scaled(QSize(previewWidth, previewHeight),
                                                  Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QDateTime ThermalItem::startDate() const
{
    return m_dateTime;
}

qreal ThermalItem::duration() const
{
    return m_duration;
}

QString ThermalItem::label() const
{
    return m_label;
}

QString ThermalItem::options() const
{
    return m_options;
}

QImage ThermalItem::originalImage() const
{
    return m_image;
}

Inspector::Probe::RegularMeshRealData ThermalItem::originalMesh() const
{
    return m_mesh;
}

QPixmap ThermalItem::previewPixmap() const
{
    return m_previewPixmap;
}

QPixmap ThermalItem::coloredPixmap(bool legend) const
{
    QImage coloredImage = m_image;
    DataUtils::paintMeshOverImage(&coloredImage, m_mesh, false, legend);
    return QPixmap::fromImage(coloredImage);
}


//
// ThermalModel
//
ThermalModel::ThermalModel(QObject *parent)
  : AbstractEasyModel(parent)
{
    // init model
    setItemValue(Results_Row, 0, 0);
    setItemValue(Results_Row, 1, "results");
    setItemValue(Results_Row, 2, false);
    setItemValue(CurrentPt_Row, 0, false);
    setItemValue(CurrentPt_Row, 1, QString());
    setItemValue(CurrentPt_Row, 2, QString());
    setItemValue(CurrentPt_Row, 3, 0);

    // load data from last session
    if (QFile::exists(storageFileName()))
        importFromFile(storageFileName());

    // mark as don't need saving
    setItemValue(Results_Row, 2, true);
}

ThermalModel::~ThermalModel()
{
    // if not saved to disk, do it
    if (!itemValue(Results_Row, 2).toBool()) {
        QModelIndexList itemsList;
        QStandardItem *itemsRoot = item(Results_Row, 1);
        for (int row = 0; row < itemsRoot->rowCount(); ++row)
            itemsList.append(itemsRoot->child(row, 0)->index());

        exportToFile(storageFileName(), itemsList);
    }
}

void ThermalModel::addResult(const QDateTime &date, qreal duration, const QString &label, const QString &options,
                             const QImage &image, const Probe::RegularMeshRealData &mesh)
{
    // add item
    ThermalItem *resultItem = new ThermalItem(date, duration, label, options, image, mesh);
    item(Results_Row, 1)->insertRow(0, resultItem);

    // refresh counter
    setItemValue(Results_Row, 0, item(Results_Row, 1)->rowCount());

    // mark as not saved
    setItemValue(Results_Row, 2, false);
}

QModelIndex ThermalModel::resultsTableIndex() const
{
    return index(Results_Row, 1);
}

void ThermalModel::clearResults()
{
    QStandardItem *itemsRoot = item(Results_Row, 1);
    while (itemsRoot->hasChildren())
        removeRow(0, itemsRoot->index());

    // mark as not saved
    setItemValue(Results_Row, 2, false);
}

QPixmap ThermalModel::resultColoredPixmap(const QModelIndex &index)
{
    const ThermalItem *item = result(index.row());
    if (!item || item->originalImage().isNull())
        return QPixmap();
    return item->coloredPixmap(true);
}

int ThermalModel::resultsCount() const
{
    return item(Results_Row, 1)->rowCount();
}

bool ThermalModel::containsResultLabel(const QString &label) const
{
    QStandardItem *itemsRoot = item(Results_Row, 1);
    for (int row = 0; row < itemsRoot->rowCount(); ++row)
        if (result(row)->label() == label)
            return true;
    return false;
}

void ThermalModel::setPtProgress(int progress)
{
    setItemValue(CurrentPt_Row, 3, progress);
}

int ThermalModel::ptProgress() const
{
    return itemValue(CurrentPt_Row, 3).toInt();
}

bool ThermalModel::exportToFile(const QString &fileName, const QModelIndexList &indices)
{
    // build up the list of selected items
    QList<const ThermalItem *> exportedItems;
    foreach (const QModelIndex &index, indices) {
        const ThermalItem *thermalItem = result(index.row());
        if (thermalItem && !thermalItem->originalImage().isNull())
            exportedItems.append(thermalItem);
    }
    if (exportedItems.isEmpty() && !indices.isEmpty())
        return false;

    // open the file for writing
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("ThermalModel::exportToFile: can't open '%s' for writing", qPrintable(fileName));
        return false;
    }
    QDataStream ds(&file);
    ds << (quint32)1;               // version
    ds << exportedItems.count();    // items count
    foreach (const ThermalItem *item, exportedItems) {
        ds << item->startDate();
        ds << item->duration();
        ds << item->label();
        ds << item->options();
        ds << item->originalImage();
        const Inspector::Probe::RegularMeshRealData &mesh = item->originalMesh();
        ds << mesh.physicalSize;
        ds << mesh.rows;
        ds << mesh.columns;
        ds << mesh.meanPatchSize;
        ds << mesh.data;
    }
    return true;
}

int ThermalModel::importFromFile(const QString &fileName)
{
    // open a file for reading
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("ThermalModel::importFromFile: can't open '%s' for reading", qPrintable(fileName));
        return 0;
    }

    // read file contents
    QDataStream ds(&file);

    quint32 version = 0;
    ds >> version;
    if (version != 1)
        return 0;

    int itemCount = 0;
    ds >> itemCount;

    for (int i = 0; i < itemCount; i++) {
        QDateTime startDate;
         ds >> startDate;
        qreal duration;
         ds >> duration;
        QString label;
         ds >> label;
        QString options;
         ds >> options;
        QImage originalImage;
         ds >> originalImage;
        Inspector::Probe::RegularMeshRealData mesh;
         ds >> mesh.physicalSize;
         ds >> mesh.rows;
         ds >> mesh.columns;
         ds >> mesh.meanPatchSize;
         ds >> mesh.data;
        addResult(startDate, duration, label, options, originalImage, mesh);
    }
    return itemCount;
}

const ThermalItem *ThermalModel::result(int row) const
{
    return dynamic_cast<ThermalItem *>(item(Results_Row, 1)->child(row));
}

QString ThermalModel::storageFileName() const
{
    // here I mimic what other do everyehere
    const QString configDir = QFileInfo(Core::ICore::instance()->settings()->fileName()).absolutePath();
    const QDir directory(configDir + QLatin1String("/qtcreator"));
    if (!directory.exists())
        directory.mkpath(directory.absolutePath());
    return directory.absolutePath() + QLatin1String("/inspector.thermal");
}
