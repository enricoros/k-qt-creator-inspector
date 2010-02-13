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

#ifndef RUNCONTROLWATCHER_H
#define RUNCONTROLWATCHER_H

#include <QtCore/QList>
#include <QtGui/QWidget>
#include "inspectiontarget.h"
class QButtonGroup;
class QLabel;
class QPushButton;
class QRadioButton;
class QVBoxLayout;

namespace Inspector {
namespace Internal {

class RunControlWidget;

/**
  \brief A widget in sync with all the ProjectExplorer's RunControls
*/
class RunningTargetSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    RunningTargetSelectorWidget(QWidget *parent = 0);

    //void setRunControlAttached(ProjectExplorer::RunControl *rc, bool attached);
    //bool runControlAttached(ProjectExplorer::RunControl *rc) const;

signals:
    void inspectionTargetSelected(const InspectionTarget &);

private slots:
    void slotRunControlAdded(ProjectExplorer::RunControl *);
    void slotRunControlDestroyed();

    void slotRunControlSelected(ProjectExplorer::RunControl *);
    void slotPidSelected(quint64);

private:
    QButtonGroup *m_buttonGroup;
    QVBoxLayout *m_layout;
    //QRadioButton *m_noRunningLabel;
    QList<RunControlWidget *> m_runWidgets;
};

/**
  \brief A widget showing information about a RunControl (running/stopped/etc..)
*/
class RunControlWidget : public QWidget
{
    Q_OBJECT

public:
    RunControlWidget(QButtonGroup *, ProjectExplorer::RunControl *, QWidget *parent = 0);

    ProjectExplorer::RunControl *runControl() const;

    void setAttachEnabled(bool enabled);

signals:
    void runControlSelected(ProjectExplorer::RunControl *rc);

private slots:
    void slotRunControlStarted();
    void slotRunControlFinished();

    void slotStartClicked();
    void slotStopClicked();
    void slotToggled(bool checked);

private:
    void updateActions();
    ProjectExplorer::RunControl *m_runControl;
    bool m_attachEnabled;
    bool m_running;
    bool m_viaDebugger;
    QLabel *m_debuggingLabel;
    QLabel *m_statusLabel;
    QLabel *m_startLabel;
    QLabel *m_stopLabel;
};

/**
  \brief A simple widget to ask for a PID
*/
class AttachToPidWidget : public QWidget
{
    Q_OBJECT

public:
    AttachToPidWidget(QButtonGroup *, QWidget *parent = 0);

signals:
    void attachPidSelected(quint64 pid);

private slots:
    void slotSelectPidClicked();
    void slotToggled(bool checked);

private:
    quint64 m_pid;
    QRadioButton *m_radio;
    QLabel *m_pidSelectLabel;
};

} // namespace Internal
} // namespace Inspector

#endif // RUNCONTROLWATCHER_H
