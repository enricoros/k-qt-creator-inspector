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

#ifndef PAINTTEMPERATUREVIEW_H
#define PAINTTEMPERATUREVIEW_H

#include "abstractview.h"
#include "ui_painttemperatureview.h"

namespace Inspector {
namespace Internal {
class PaintingModule;

class PaintTemperatureView : public AbstractView, public Ui::PaintTemperatureView
{
    Q_OBJECT

public:
    PaintTemperatureView(PaintingModule *parentModule);

private slots:
    // 'new test' slots
    void slotCheckIterations();
    void slotCheckWeight();
    void slotLoadDefaults();
    void slotTestClicked();

    // results slots
    void slotResultActivated(const QModelIndex &index);
};

} // namespace Internal
} // namespace Inspector

#endif // PAINTTEMPERATUREVIEW_H
