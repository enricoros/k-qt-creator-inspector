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

#include "inspectorstyle.h"
#include <utils/stylehelper.h>
#include <QtGui/QGradient>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QWidget>

using namespace Inspector::Internal;

//
// Style
//
int InspectorStyle::defaultComboHeight()
{
    return Utils::StyleHelper::navigationWidgetHeight() - 2;
}

int InspectorStyle::defaultBarHeight()
{
    return Utils::StyleHelper::navigationWidgetHeight();
}

QPalette InspectorStyle::invertedPalette()
{
    QPalette pal;
     pal.setBrush(QPalette::Window, QColor(100, 100, 100));
     pal.setBrush(QPalette::WindowText, Qt::white);
     pal.setBrush(QPalette::Button, Qt::darkGray);
     pal.setBrush(QPalette::ButtonText, Qt::white);
     pal.setBrush(QPalette::Base, Qt::transparent);
    return pal;
}

QColor InspectorStyle::invertedBackColor()
{
    return QColor(100, 100, 100);
}

QColor InspectorStyle::invertedTextColor()
{
    return Qt::white;
}

void InspectorStyle::drawVerticalShadow(QPainter *painter, int width, int height, const QColor &color, bool topToBottom)
{
    drawVerticalShadow(painter, QRect(0, 0, width, height), color, topToBottom);
}

void InspectorStyle::drawVerticalShadow(QPainter *painter, const QRect &rect, const QColor &color, bool topToBottom)
{
    QLinearGradient lg(rect.left(), topToBottom ? rect.top() : rect.bottom(),
                       rect.left(), topToBottom ? rect.bottom() : rect.top());
    QColor col = color;
    col.setAlpha(128); lg.setColorAt(0.0, col);
    col.setAlpha( 32); lg.setColorAt(0.4, col);
    col.setAlpha( 10); lg.setColorAt(0.7, col);
    col.setAlpha(  0); lg.setColorAt(1.0, col);
    painter->fillRect(rect, lg);
}

class Inspector::Internal::OnePixelBlackLine : public QWidget
{
public:
    OnePixelBlackLine(QWidget *parent)
        : QWidget(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setFixedHeight(1);
    }
    void paintEvent(QPaintEvent *e)
    {
        Q_UNUSED(e);
        QPainter p(this);
        p.fillRect(contentsRect(), QBrush(Utils::StyleHelper::borderColor()));
    }
};

QWidget *InspectorStyle::createOnePixelBlackLine(QWidget *parent)
{
    return new Inspector::Internal::OnePixelBlackLine(parent);
}
