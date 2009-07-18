/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Started on 14 Jul 2009 by root.                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PERFORMANCEWINDOW_H
#define PERFORMANCEWINDOW_H

#include <QWidget>
class QComboBox;
class QVBoxLayout;

namespace Performance {
namespace Internal {

class PerformanceWindow : public QWidget
{
    Q_OBJECT

public:
    PerformanceWindow(QWidget *parent = 0);
    ~PerformanceWindow();

private slots:
    void slotMainChanged(int);
    void slotSubChanged(int);

private:
    void activateRInformation();
    void activateRDebugging();
    void activatePainting(int subChoice);
    void activateSubSelector();

    void setCentralWidget(QWidget * widget);
    void updateMainCombo(bool enabled);

    QComboBox *m_mainCombo;
    QComboBox *m_subCombo;
    QVBoxLayout *m_mainLayout;
    QWidget *m_centralWidget;
};

} // namespace Internal
} // namespace Performance

#endif // PERFORMANCEWINDOW_H
