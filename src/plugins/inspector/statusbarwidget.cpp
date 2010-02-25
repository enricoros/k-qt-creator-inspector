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

#include "statusbarwidget.h"
#include "inspection.h"
#include "inspectorstyle.h"
#include "tasksscroller.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOption>
#include <QToolButton>


//#define DRAW_STYLED

using namespace Inspector::Internal;

class Inspector::Internal::KillTaskButton : public QToolButton
{
public:
    KillTaskButton(quint32 tid, const QString &text, QWidget *parent = 0)
      : QToolButton(parent)
      , m_tid(tid)
    {
        setAutoRaise(true);
        setIcon(QIcon(":/trolltech/assistant/images/win/closetab.png"));
        setText(text);
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }

    quint32 tid() const
    {
        return m_tid;
    }

private:
    quint32 m_tid;
};

StatusBarWidget::StatusBarWidget(QWidget *parent)
  : SunkenBar(false, parent)
  , m_taskScroller(0)
  , m_layout(0)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(2, 1, 2, 1);

    m_taskScroller = new TasksScroller(this);
    connect(m_taskScroller, SIGNAL(newActiveTask(quint32,QString)),
            this, SLOT(slotNewActiveTask(quint32,QString)));
    connect(m_taskScroller, SIGNAL(removeActiveTask(quint32)),
            this, SLOT(slotRemoveActiveTask(quint32)));
    connect(this, SIGNAL(stopTask(quint32)),
            m_taskScroller, SLOT(slotStopTask(quint32)));
    m_layout->addWidget(m_taskScroller);
    m_layout->setAlignment(m_taskScroller, Qt::AlignBottom);

    m_layout->addStretch();
}

void StatusBarWidget::setInspection(Inspection *inspection)
{
    // clear previous data
    qDeleteAll(m_buttons);
    m_buttons.clear();

    // apply the model to the tasks widget
    TasksModel *model = inspection ? inspection->tasksModel() : 0;
    m_taskScroller->setTasksModel(model);
}

void StatusBarWidget::slotNewActiveTask(quint32 tid, const QString &taskName)
{
    foreach (KillTaskButton *button, m_buttons) {
        if (button->tid() == tid) {
            qWarning("StatusBarWidget::slotNewActiveTask: task %d already in", tid);
            return;
        }
    }
    KillTaskButton *button = new KillTaskButton(tid, taskName, this);
    connect(button, SIGNAL(clicked()), this, SLOT(slotStopTaskClicked()));
    m_buttons.append(button);
    m_layout->insertWidget(m_layout->count() - 1, button);
}

void StatusBarWidget::slotRemoveActiveTask(quint32 tid)
{
    foreach (KillTaskButton *button, m_buttons) {
        if (button->tid() == tid) {
            m_buttons.removeAll(button);
            button->deleteLater();
            return;
        }
    }
    qWarning("StatusBarWidget::slotRemoveActiveTask: task %d not present", tid);
}

void StatusBarWidget::slotStopTaskClicked()
{
    quint32 taskId = static_cast<KillTaskButton *>(sender())->tid();
    // don't use the button after this line, because it should be deleted
    emit stopTask(taskId);
}
