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
#include <QtCore/QTextStream>

using namespace Inspector::Internal;

bool DataUtils::exportOctaveIntArray(const QString &fileName, const QString &varName, int rows, int cols, qint32 *data)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("DataUtils::exportOctaveIntArray: can't open file");
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
}
/*
bool DataUtils::exportOctaveRealArray(const QString &file, int rows, int cols, qreal *data)
{

}
*/
