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

#ifndef PERFORMANCENOTIFICATION_H
#define PERFORMANCENOTIFICATION_H

#include <QWidget>

namespace Performance {
namespace Internal {

class PerformanceNotification : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal signOpacity READ signOpacity WRITE setSignOpacity)

public:
    PerformanceNotification(QWidget * parent = 0);

    void addWarning();
    void clearWarnings();

    // ::QWidget
    void paintEvent(QPaintEvent * event);
    void mousePressEvent(QMouseEvent * event);

signals:
    void clicked();

private:
    qreal signOpacity() const;
    void setSignOpacity(qreal opacity);
    qreal m_signOpacity;
    QPixmap m_pixmap;
    int m_cWarnings;
};

} // namespace Internal
} // namespace Performance

#endif
