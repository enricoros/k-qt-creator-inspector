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

#ifndef PERFORMANCEINFORMATION_H
#define PERFORMANCEINFORMATION_H

#include <QWidget>

namespace Ui {
    class PerformanceInformation;
}

namespace Performance {
namespace Internal {

class PerformanceInformation : public QWidget {
    Q_OBJECT
public:
    PerformanceInformation(QWidget *parent = 0);
    ~PerformanceInformation();

private:
    Ui::PerformanceInformation *m_ui;
};

} // namespace Internal
} // namespace Performance

#endif // PERFORMANCEINFORMATION_H
