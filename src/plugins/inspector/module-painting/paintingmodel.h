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

#ifndef PAINTINGMODEL_H
#define PAINTINGMODEL_H

#include "abstracteasymodel.h"
#include <QDateTime>
#include <QPixmap>
#include <QStandardItem>

namespace Inspector {
namespace Internal {

class TemperatureItem : public QStandardItem
{
public:
    TemperatureItem(const QDateTime &dt, qreal duration, const QString &desc, const QString &options, const QPixmap &image);

    QDateTime date() const;
    qreal duration() const;
    QString description() const;
    QString options() const;
    QPixmap image() const;
    QPixmap previewImage() const;

    static const int previewWidth  = 80;
    static const int previewHeight = 60;

private:
    QDateTime m_dateTime;
    qreal m_duration;
    QString m_description;
    QString m_options;
    QPixmap m_pixmap;
    QPixmap m_previewPixmap;
};

class PaintingModel : public AbstractEasyModel
{
    Q_OBJECT

public:
    PaintingModel(QObject *parent = 0);
    ~PaintingModel();

    void addResult(const QDateTime &, qreal duration, const QString &description, const QString &options, const QPixmap &image);
    QModelIndex resultsTableIndex() const;
    const TemperatureItem *result(int row) const;

    void setPtProgress(int progress);
    int ptProgress() const;

private:
    void loadData();
    void saveData();
};

} // namespace Internal
} // namespace Inspector

#endif // PAINTINGMODEL_H
