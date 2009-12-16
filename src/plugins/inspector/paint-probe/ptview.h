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

#ifndef PTVIEW_H
#define PTVIEW_H

#include <QWidget>
#include "ui_ptview.h"

namespace Inspector {
namespace Internal {

class PaintTemperatureView : public QWidget, public Ui::PTView
{
    Q_OBJECT

public:
    PaintTemperatureView(QWidget *parent = 0);

private:
    QPixmap m_pixmap;

private slots:
    void on_defaultsButton_clicked();
    void on_runButton_clicked();
    void slotCheckPasses();
    void slotUpdatePops();
};

} // namespace Internal
} // namespace Inspector

#endif // PTVIEW_H
