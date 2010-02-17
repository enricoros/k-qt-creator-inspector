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

#ifndef INSPECTORSTYLE_H
#define INSPECTORSTYLE_H

#include <QtCore/QList>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtGui/QWidget>

namespace Inspector {
namespace Internal {

class OnePixelBlackLine;

class InspectorStyle
{
public:
    static int defaultComboHeight();
    static int defaultBarHeight();

    static QPalette invertedPalette();
    static QColor invertedBackColor();
    static QColor invertedTextColor();

    static void drawVerticalShadow(QPainter *, int width, int height, const QColor &, bool topToBottom = true);
    static void drawVerticalShadow(QPainter *, const QRect &rect, const QColor &, bool topToBottom = true);
    static void drawCoolGradient(QPainter *, const QRect &rect, const QColor &);

    static QWidget *createOnePixelBlackLine(QWidget *parent = 0);
};

class WatermarkedWidget : public QWidget
{
public:
    WatermarkedWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);

private:
    QPixmap m_watermarkPixmap;
};

} // namespace Internal
} // namespace Inspector

#endif // INSPECTORSTYLE_H
