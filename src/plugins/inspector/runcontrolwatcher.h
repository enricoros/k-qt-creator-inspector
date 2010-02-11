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
class QLabel;
class QPushButton;
class QVBoxLayout;

namespace ProjectExplorer {
class RunControl;
}

namespace Inspector {
namespace Internal {

class RunControlWidget;

/**
  \brief A widget in sync with all the ProjectExplorer's RunControls
*/
class RunControlList : public QWidget
{
    Q_OBJECT

public:
    RunControlList(QWidget *parent = 0);

    //void setRunControlAttached(ProjectExplorer::RunControl *rc, bool attached);
    //bool runControlAttached(ProjectExplorer::RunControl *rc) const;

public slots:
    void setButtonsEnabled(bool enabled);

signals:
    void attachToRunControl(ProjectExplorer::RunControl *rc);

private slots:
    void slotRunControlAdded(ProjectExplorer::RunControl *);
    void slotRunControlDestroyed();

private:
    QVBoxLayout *m_layout;
    QLabel *m_noRunningLabel;
    QList<RunControlWidget *> m_runWidgets;
    bool m_buttonsEnabled;
};

/**
  \brief A widget showing information about a RunControl (running/stopped/etc..)
*/
class RunControlWidget : public QWidget
{
    Q_OBJECT

public:
    RunControlWidget(ProjectExplorer::RunControl *, QWidget *parent = 0);

    ProjectExplorer::RunControl *runControl() const;

    void setAttachEnabled(bool enabled);

signals:
    void attachToRunControl(ProjectExplorer::RunControl *rc);

private slots:
    void slotRunControlStarted();
    void slotRunControlFinished();

    void slotStartClicked();
    void slotStopClicked();
    void slotAttachClicked();

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
    QPushButton *m_attachButton;
};

} // namespace Internal
} // namespace Inspector

#endif // RUNCONTROLWATCHER_H
