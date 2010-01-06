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

#include "paintingmodel.h"

using namespace Inspector::Internal;

/* == PaintingModel Usage ==
Row 'Results_Row':
  0: results count          int
  1: results                LIST(TemperatureItem)
  2: saved                  bool

Row 'CurrentPt_Row'
  0: started                bool
  1: label                  string
  2: options                string
  3: progress               int (0..100)
*/

#define Results_Row 0
#define CurrentPt_Row 1

TemperatureItem::TemperatureItem(const QDateTime &dt, qreal duration, const QString &desc, const QString &options, const QPixmap &image)
  : QStandardItem(desc)
  , m_dateTime(dt)
  , m_duration(duration)
  , m_description(desc)
  , m_options(options)
  , m_pixmap(image)
{
    m_previewPixmap = m_pixmap.scaled(QSize(previewWidth, previewHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    setEditable(false);
}

QDateTime TemperatureItem::date() const
{
    return m_dateTime;
}

qreal TemperatureItem::duration() const
{
    return m_duration;
}

QString TemperatureItem::description() const
{
    return m_description;
}

QString TemperatureItem::options() const
{
    return m_options;
}

QPixmap TemperatureItem::image() const
{
    return m_pixmap;
}

QPixmap TemperatureItem::previewImage() const
{
    return m_previewPixmap;
}



PaintingModel::PaintingModel(QObject *parent)
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

    // load data from settings
    loadData();
}

PaintingModel::~PaintingModel()
{
    saveData();
}

void PaintingModel::addResult(const QDateTime &date, qreal duration, const QString &description, const QString &options, const QPixmap &image)
{
    // add item
    TemperatureItem *resultItem = new TemperatureItem(date, duration, description, options, image);
    item(Results_Row, 1)->insertRow(0, resultItem);

    // refresh counter
    setItemValue(Results_Row, 0, item(Results_Row, 1)->rowCount());

    // mark as not saved
    setItemValue(Results_Row, 2, false);
}

QModelIndex PaintingModel::resultsTableIndex() const
{
    return index(Results_Row, 1);
}

const TemperatureItem *PaintingModel::result(int row) const
{
    return dynamic_cast<TemperatureItem *>(item(Results_Row, 1)->child(row));
}

void PaintingModel::setPtProgress(int progress)
{
    setItemValue(CurrentPt_Row, 3, progress);
}

int PaintingModel::ptProgress() const
{
    return itemValue(CurrentPt_Row, 3).toInt();
}

void PaintingModel::loadData()
{
    // TODO...
}

void PaintingModel::saveData()
{
    // check if already saved
    if (itemValue(Results_Row, 2).toBool())
        return;

    // TODO...

    // mark as saved
    setItemValue(Results_Row, 2, true);
}
