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

#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>

using namespace Performance::Internal;

PerformanceInformation::PerformanceInformation(QWidget *parent)
  : QWidget(parent)
  , m_pOk(":/performance/images/status-ok.png")
  , m_pErr(":/performance/images/status-err.png")
{
    setupUi(this);
    buttonBox->setFocus();
}

void PerformanceInformation::setFieldState(QWidget * field, int state)
{
    if (QAbstractButton *b = dynamic_cast<QAbstractButton *>(field)) {
        switch (state) {
            case 0:
                b->setText(tr("?"));
                break;
            case 1:
                b->setIcon(m_pOk);
                if (b->isCheckable())
                    b->setChecked(true);
                break;
            case -1:
                b->setIcon(m_pErr);
                if (b->isCheckable())
                    b->setChecked(false);
                break;
        }
    }

    if (QLabel *l = dynamic_cast<QLabel *>(field)) {
        switch (state) {
            case 0:
                l->setText(tr("unknown"));
                break;
            case 1:
                l->setPixmap(m_pOk);
                break;
            case -1:
                l->setPixmap(m_pErr);
                break;
        }
    }
}
