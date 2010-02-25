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

#ifndef STATUSBARWIDGET_H
#define STATUSBARWIDGET_H

#include "inspectorstyle.h"
#include <QtCore/QList>
class QHBoxLayout;
class QLabel;
class QPixmap;

namespace Inspector {
namespace Internal {

class Inspection;
class KillTaskButton;
class TasksScroller;

/**
  \brief Shows Inspection information, like running Tasks
*/
class StatusBarWidget : public SunkenBar
{
    Q_OBJECT

public:
    StatusBarWidget(QWidget *parent = 0);

    void setInspection(Inspection *inspection);

signals:
    void stopTask(quint32 tid);

private slots:
    void slotNewActiveTask(quint32 tid, const QString &name);
    void slotRemoveActiveTask(quint32 tid);
    void slotStopTaskClicked();

private:
    TasksScroller *m_taskScroller;
    QList<KillTaskButton *> m_buttons;
    QHBoxLayout *m_layout;
};

} // namespace Internal
} // namespace Inspector

#endif // STATUSBARWIDGET_H
