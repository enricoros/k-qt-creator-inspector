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

#include "datautils.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QRect>
#include <QtCore/QTextStream>

#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPainter>

using namespace Inspector::Internal;

template<typename T>
bool DataUtils::exportOctaveArray(const QString &fileName, const QString &varName, int rows, int cols, T *data)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("DataUtils::exportOctaveArray: can't open file");
        return false;
    }

    QTextStream ts(&file);
    ts << QObject::tr("# Created by QtCreator %1, %2\n").arg(QCoreApplication::applicationVersion()).arg(QDateTime::currentDateTime().toString());
    ts << QObject::tr("# name: %1\n").arg(varName);
    ts << "# type: matrix\n";
    ts << QObject::tr("# rows: %1\n").arg(rows);
    ts << QObject::tr("# columns: %1\n").arg(cols);
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            ts << " ";
            ts << QString::number(data[r * cols + c]);
        }
        ts << "\n";
    }
    return true;
}

template<typename T>
bool DataUtils::importOctaveArray(const QString &fileName, const QString &varName, int *rows, int *cols, T *data)
{
    Q_UNUSED(fileName);
    Q_UNUSED(varName);
    Q_UNUSED(rows);
    Q_UNUSED(cols);
    Q_UNUSED(data);
    qWarning("DataUtils::importOctaveArray<...>: not implemented");
    return false;
}

void DataUtils::paintMeshOverImage(QImage *image, const Inspector::Probe::RegularMeshRealData &mesh, bool scaleToImage, bool showLegend)
{
    // find out the input range
    double tTotal = 0, tMax = 0, tMin = 0;
    foreach (qreal value, mesh.data) {
        if (value > tMax)
            tMax = value;
        if (value < tMin || tMin == 0)
            tMin = value;
        tTotal += value;
    }

    // draw over the image with the same rect-subdivision done by the probe
    QPainter painter(image);
    const int wW = scaleToImage ? image->width() : mesh.physicalSize.width();
    const int wH = scaleToImage ? image->height() : mesh.physicalSize.height();
    const int wRows = mesh.rows;
    const int wCols = mesh.columns;
    int index = 0;
    int y1 = 0;
    for (int row = 0; row < wRows; ++row) {
        int y2 = (wH * (row + 1)) / wRows;
        int x1 = 0;
        for (int col = 0; col < wCols; ++col) {
            int x2 = (wW * (col + 1)) / wCols;
            QRect paintRect(x1, y1, x2 - x1, y2 - y1);
            double nValue = (mesh.data[index++] - tMin) / (tMax - tMin);
            QColor mappedColor = QColor::fromHsvF(0.67 - nValue * 0.67, 1.0, 1.0, 0.5 + 0.25*nValue);
            painter.fillRect(paintRect, mappedColor);
            x1 = x2;
        }
        y1 = y2;
    }

    // superimpose the legend
    if (showLegend) {
        for (int x = 0; x <= 100; x++) {
            double alpha = (double)x / 100.0;
            QColor col = QColor::fromHsvF(0.67 - alpha * 0.67, 1.0, 1.0, 0.5 + 0.25*alpha);
            painter.fillRect(wW - 10 - x, 10, 1, 20, col);
        }
    }
}
