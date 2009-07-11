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

#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/progressmanager/progresspie.h>
#include <coreplugin/stylehelper.h>
#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>

class WarnWidget : public QWidget {
    public:
        WarnWidget(QWidget * parent = 0)
            : QWidget(parent)
        {
            QHBoxLayout * lay = new QHBoxLayout(this);

            QLabel * label = new QLabel();
            label->setMinimumSize(32,32);
            label->setPixmap(QPixmap(":/performance/images/mark-32.png"));
            label->setAlignment(Qt::AlignCenter);
            lay->addWidget(label);


        }

        void paintEvent(QPaintEvent * event)
        {
            // see progresspie.cpp for drawing...
            QPainter p(this);
            QFont boldFont(p.font());
            boldFont.setPointSizeF(StyleHelper::sidebarFontSize());
            boldFont.setBold(true);
            p.setFont(boldFont);
            QFontMetrics fm(boldFont);

            // Draw separator
            p.setPen(QColor(0, 0, 0, 70));
            p.drawLine(0,0, size().width(), 0);

            p.setPen(QColor(255, 255, 255, 70));
            p.drawLine(0, 1, size().width(), 1);

        }
};

void PerformancePane::visibilityChanged(bool visible)
{
    // insert temp label into the left panel
    WarnWidget * ww = new WarnWidget();
    Core::ICore::instance()->modeManager()->addWidget(ww);
}
