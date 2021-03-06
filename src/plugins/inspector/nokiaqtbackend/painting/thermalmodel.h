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

#ifndef THERMALMODEL_H
#define THERMALMODEL_H

#include "abstracteasymodel.h"
#include <QtCore/QDateTime>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QStandardItem>

// include Probe data types
#include "../../../../../share/qtcreator/gdbmacros/probedata.h"

namespace Inspector {
namespace Internal {

class ThermalItem;

class ThermalModel : public AbstractEasyModel
{
    Q_OBJECT

public:
    ThermalModel(QObject *parent = 0);
    ~ThermalModel();

    void addResult(const QDateTime &, qreal duration, const QString &label, const QString &options,
                   const QImage &image, const Probe::RegularMeshRealData &mesh);
    QModelIndex resultsTableIndex() const;
    void clearResults();

    QString resultName(const QModelIndex &) const;
    void setResultName(const QString &, const QModelIndex &);
    QPixmap resultOriginalPixmap(const QModelIndex &) const;
    QPixmap resultColoredPixmap(const QModelIndex &) const;
    int resultsCount() const;

    bool containsResultLabel(const QString &) const;

    void setPtProgress(int progress);
    int ptProgress() const;

    bool exportToFile(const QString &fileName, const QModelIndexList &indices);
    int importFromFile(const QString &fileName);

private:
    friend class ThermalItemDelegate;
    const ThermalItem *result(int row) const;
    QString storageFileName() const;
};

class ThermalItem : public QStandardItem
{
public:
    ThermalItem(const QDateTime &dt, qreal duration, const QString &label, const QString &options,
                const QImage &image, const Probe::RegularMeshRealData &mesh);

    void setLabel(const QString &);

    QDateTime startDate() const;
    qreal duration() const;
    QString label() const;
    QString options() const;
    QImage originalImage() const;
    Probe::RegularMeshRealData originalMesh() const;
    QPixmap previewPixmap() const;

    QPixmap coloredPixmap(bool legend) const;

    static const int previewWidth  = 80;
    static const int previewHeight = 60;

private:
    QDateTime m_dateTime;
    qreal m_duration;
    QString m_label;
    QString m_options;
    QImage m_image;
    Probe::RegularMeshRealData m_mesh;
    QPixmap m_previewPixmap;
};

} // namespace Internal
} // namespace Inspector

#endif // THERMALMODEL_H
