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
#include <QtSvg/QSvgRenderer>
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

void InspectorStyle::drawCoolGradient(QPainter *painter, const QRect &rect, const QColor &baseColor)
{
    QLinearGradient lg(rect.left(), rect.top(), rect.left(), rect.bottom());
    lg.setColorAt(0.0, baseColor.light(121));
    lg.setColorAt(0.49, baseColor);
    lg.setColorAt(0.50, baseColor.dark(150));
    lg.setColorAt(1.0, baseColor);
    QColor penColor = baseColor;
    penColor.setAlpha(128);
    painter->setPen(penColor);
    painter->setBrush(lg);
    painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 4, 4);
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


//
// SunkenBar
//
SunkenBar::SunkenBar(bool onTop, QWidget *parent)
  : QWidget(parent)
  , m_onTop(onTop)
  , m_shadowTile(0)
{
    setPalette(InspectorStyle::invertedPalette());
#if defined(DRAW_STYLED_SUNKENBAR)
    setProperty("panelwidget", true);
    setProperty("panelwidget_singlerow", true);
#else
    setAutoFillBackground(true);
#endif
    setFixedHeight(InspectorStyle::defaultBarHeight());
}

SunkenBar::~SunkenBar()
{
    delete m_shadowTile;
}

void SunkenBar::paintEvent(QPaintEvent *event)
{
    // the first time create the Shadow Tile
    if (!m_shadowTile) {
        const int height = m_onTop ? 4 : 10;
        m_shadowTile = new QPixmap(64, height);
        m_shadowTile->fill(Qt::transparent);
        QPainter shadowPainter(m_shadowTile);
        InspectorStyle::drawVerticalShadow(&shadowPainter, 64, height,
                                           m_onTop ? QColor(50, 50, 50) : Qt::black,
                                           !m_onTop);
    }

    // draw styled background
    QPainter p(this);
#if defined(DRAW_STYLED_SUNKENBAR)
    QStyleOption option;
    option.rect = rect();
    option.state = QStyle::State_Horizontal;
    style()->drawControl(QStyle::CE_ToolBar, &option, &p, this);
#endif

    // draw dubtle shadow
    if (m_shadowTile) {
        QRect shadowRect = event->rect();
        if (m_onTop) {
            shadowRect.setTop(height() - 4);
            shadowRect.setHeight(4);
        } else {
            shadowRect.setTop(0);
            shadowRect.setHeight(10);
        }
        p.drawTiledPixmap(shadowRect, *m_shadowTile);
    }
}


//
// WatermarkedWidget
//
WatermarkedWidget::WatermarkedWidget(QWidget *parent)
  : QWidget(parent)
{
    QSvgRenderer wmRender(QString(":/inspector/images/inspector-watermark.svg"));
    if (wmRender.isValid()) {
        m_watermarkPixmap = QPixmap(wmRender.defaultSize());
        m_watermarkPixmap.fill(Qt::transparent);
        QPainter wmPainter(&m_watermarkPixmap);
        wmRender.render(&wmPainter);
    }
}

void WatermarkedWidget::paintEvent(QPaintEvent *event)
{
    // draw a light gradient as the background
    QPainter p(this);
#if 0
    QLinearGradient bg(0, 0, 0, 1);
    bg.setCoordinateMode(QGradient::StretchToDeviceMode);
    bg.setColorAt(0.0, QColor(247, 247, 247));
    bg.setColorAt(1.0, QColor(215, 215, 215));
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(event->rect(), bg);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif

    // draw the watermark
    if (!m_watermarkPixmap.isNull()) {
        QRect wmRect(isLeftToRight() ? (width() - m_watermarkPixmap.width()) : 0, 30,
                     m_watermarkPixmap.width(), m_watermarkPixmap.height());
        if (event->rect().intersects(wmRect))
            p.drawPixmap(wmRect.topLeft(), m_watermarkPixmap);
    }
}

//
// ColorWidget
//
ColorWidget::ColorWidget(const QColor &color, QWidget *parent)
  : QWidget(parent)
{
    QColor tColor = color;
    tColor.setAlpha(qMin(color.alpha(), 200));

    QPalette pal;
    pal.setBrush(QPalette::Window, tColor);
    setPalette(pal);

    setAutoFillBackground(true);
}
