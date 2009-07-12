/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Started on 10 Jul 2009 by root.                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "performancepane.h"
#include <QLinearGradient>

using namespace Performance::Internal;

PerformancePane::PerformancePane(QObject *parent)
    : Core::IOutputPane(parent)
{
}

QWidget * PerformancePane::outputWidget(QWidget *parent)
{
    m_editWidget = new QPlainTextEdit(parent);
    QPalette pal;
    QLinearGradient lg(0, 0, 0, 300);
    lg.setColorAt(0.0, Qt::white);
    lg.setColorAt(1.0, Qt::lightGray);
    pal.setBrush(QPalette::Base, lg);
    m_editWidget->setFrameStyle(QFrame::NoFrame);
    m_editWidget->setPalette(pal);
    return m_editWidget;
}

void PerformancePane::addString(const QString & string)
{
    m_editWidget->appendHtml("<font color='#800'>" + string + "</font>");
}

#include <coreplugin/stylehelper.h>
#include <QPainter>
#if QT_VERSION >= 0x040600
#include <QPropertyAnimation>
#endif

PerformanceMiniWidget::PerformanceMiniWidget(QWidget * parent)
  : QWidget(parent)
  , m_signOpacity(1.0)
  , m_pixmap(":/performance/images/mark-32.png")
  , m_cWarnings(0)
{
    setMinimumSize(60, 50);
}

void PerformanceMiniWidget::addWarning()
{
    m_cWarnings++;
#if QT_VERSION >= 0x040600
    QPropertyAnimation * ani = new QPropertyAnimation(this, "signOpacity");
    ani->setEasingCurve(QEasingCurve::OutCirc);
    ani->setDuration(400);
    ani->setStartValue(0.1);
    ani->setEndValue(1.0);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
#else
    update();
#endif
}

void PerformanceMiniWidget::paintEvent(QPaintEvent * /*event*/)
{
    // see progresspie.cpp for drawing...
    QPainter p(this);
    QFont boldFont(p.font());
    boldFont.setPointSizeF(Utils::StyleHelper::sidebarFontSize());
    boldFont.setBold(true);
    p.setFont(boldFont);
    //QFontMetrics fm(boldFont);

    // Draw top separator
    p.setPen(QColor(0, 0, 0, 70));
    p.drawLine(0,0, size().width(), 0);
    p.setPen(QColor(255, 255, 255, 70));
    p.drawLine(0, 1, size().width(), 1);

    // Draw top text
    p.setPen(Qt::white);
    p.drawText(0, 4, width(), 10, Qt::AlignHCenter, tr("Runtime"));

    // Draw left pixmap
    int baseLine = 18;
    int left = 20;
    if (m_signOpacity < 1.0)
        p.setOpacity(m_signOpacity);
    p.drawPixmap(left, (height() + baseLine - m_pixmap.height()) / 2, m_pixmap);
    if (m_signOpacity < 1.0)
        p.setOpacity(1.0);

    // Draw Warnings
    baseLine += 10;
    left += m_pixmap.width() + 5;
    p.drawText(left, baseLine, width() - left, 12, Qt::AlignVCenter, tr("%1").arg(m_cWarnings));
}

void PerformanceMiniWidget::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
}

qreal PerformanceMiniWidget::signOpacity() const
{
    return m_signOpacity;
}

void PerformanceMiniWidget::setSignOpacity(qreal opacity)
{
    m_signOpacity = opacity;
    update();
}
