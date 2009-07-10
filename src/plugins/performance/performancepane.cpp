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
