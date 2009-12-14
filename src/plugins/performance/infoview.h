/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Started on 12 Jul 2009 by root.                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INFOVIEW_H
#define INFOVIEW_H

#include <QDialog>
#include "ui_infoview.h"

namespace Performance {
namespace Internal {

class InfoView : public QDialog, public Ui::InfoView
{
    Q_OBJECT

public:
    InfoView(QWidget *parent = 0);

    void setFieldState(QWidget * field, int state);

private:
    QPixmap m_pOk;
    QPixmap m_pErr;
};

} // namespace Internal
} // namespace Performance

#endif // INFOVIEW_H
