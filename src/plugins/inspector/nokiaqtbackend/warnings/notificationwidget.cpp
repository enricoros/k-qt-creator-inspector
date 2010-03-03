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

#include "notificationwidget.h"
#include <utils/stylehelper.h>
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>

using namespace Inspector::Internal;

NotificationWidget::NotificationWidget(QWidget * parent)
  : QWidget(parent)
  , m_signOpacity(1.0)
  , m_pixmap(":/inspector/warnings/mark-32.png")
  , m_cWarnings(0)
{
    setMinimumSize(60, 50);
}

void NotificationWidget::addWarning()
{
    m_cWarnings++;
    QPropertyAnimation * ani = new QPropertyAnimation(this, "signOpacity");
    ani->setEasingCurve(QEasingCurve::OutCirc);
    ani->setDuration(400);
    ani->setStartValue(0.1);
    ani->setEndValue(1.0);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
}

void NotificationWidget::clearWarnings()
{
    m_cWarnings = 0;
    update();
}

void NotificationWidget::paintEvent(QPaintEvent * /*event*/)
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
    p.drawText(0, 4, width(), 10, Qt::AlignHCenter, tr("Defect"));

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

void NotificationWidget::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
}

qreal NotificationWidget::signOpacity() const
{
    return m_signOpacity;
}

void NotificationWidget::setSignOpacity(qreal opacity)
{
    m_signOpacity = opacity;
    update();
}
