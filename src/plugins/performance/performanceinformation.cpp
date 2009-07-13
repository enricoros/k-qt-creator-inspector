/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>        *
 *   Started on 12 Jul 2009 by root.
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "performanceinformation.h"
#include "ui_performanceinformation.h"

using namespace Performance::Internal;

PerformanceInformation::PerformanceInformation(QWidget *parent)
  : QWidget(parent)
  , m_ui(new Ui::PerformanceInformation)
{
    m_ui->setupUi(this);
}

PerformanceInformation::~PerformanceInformation()
{
    delete m_ui;
}
