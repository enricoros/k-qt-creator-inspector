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

#ifndef DASHBOARDWINDOW_H
#define DASHBOARDWINDOW_H

#include <QtCore/QList>
#include <QtGui/QComboBox>
#include <QtGui/QScrollArea>
#include "inspectiontarget.h"
class QAbstractButton;
class QGridLayout;
class QLabel;
class QVBoxLayout;

namespace ProjectExplorer {
class Project;
class RunConfiguration;
class Target;
}

namespace Inspector {
namespace Internal {

class IBackendFactory;
class Inspection;
class DevicesComboBox;
class BackendsComboBox;
class ProjectsComboBox;
class RunconfComboBox;
class TasksScroller;

/**
  \brief The Dashboard of Inspector. Can start Inspections
*/
class DashboardWindow : public QScrollArea
{
    Q_OBJECT

public:
    DashboardWindow(QWidget *parent = 0);

    void newInspection(const InspectionTarget &, IBackendFactory *);

signals:
    void requestInspectionDisplay(Inspection *);

private slots:
    void slotProjectChanged();
    void slotDeviceChanged();
    void slotRunconfChanged();

    void slotEvaluateNewTarget();
    void slotStartNewTarget();

    void slotExistingTargetSelected(const InspectionTarget &);
    void slotEvaluateExistingTarget();
    void slotStartExistingTarget();

    void slotUpdateActionStatus();

    void slotInspectionAdded(Inspection *);
    void slotInspectionRemoved(Inspection *);

    void slotCloseInspection(Inspection *);

private:
    void appendWrappedWidget(const QString &title, const QIcon &icon, QWidget *widget);
    void appendSubWidget(QGridLayout *layout, QWidget *widget,
                         const QString &title = QString(),
                         const QString &subTitle = QString());
    QWidget *m_root;
    QGridLayout *m_layout;

    // new run
    ProjectsComboBox *m_projectsCombo;
    QLabel *m_runconfLabel;
    DevicesComboBox *m_devicesCombo;
    RunconfComboBox *m_runconfsCombo;
    BackendsComboBox *m_backendsCombo;
    InspectionTarget m_runTarget;
    QAbstractButton *m_newRunButton;

    // attach to running
    InspectionTarget m_attTarget;
    BackendsComboBox *m_attBackends;
    QAbstractButton *m_attButton;

    // running Inspections
    QLabel *m_noInspectionsLabel;
    QVBoxLayout *m_inspectionsLayout;
    QList<Inspection *> m_inspections;
    QList<QWidget *> m_inspectionWidgets;
};

/**
  \brif A QComboBox synced with current projects
*/
class ProjectsComboBox : public QComboBox
{
    Q_OBJECT

public:
    ProjectsComboBox(QWidget *parent = 0);

    bool isEmpty() const;
    ProjectExplorer::Project *currentProject() const;

signals:
    void currentProjectChanged();

private slots:
    void add(ProjectExplorer::Project *);
    void remove(ProjectExplorer::Project *);
    void activeChanged(ProjectExplorer::Project *);

private:
    QList<ProjectExplorer::Project *> m_comboIndexToProject;
};

/**
  \brif A QComboBox synced with Targets of a Project
  The Targets are called Devices here, to avoid clashing within
  thig plugin.
*/
class DevicesComboBox : public QComboBox
{
    Q_OBJECT

public:
    DevicesComboBox(QWidget *parent = 0);

    void setProject(ProjectExplorer::Project *);
    ProjectExplorer::Target *currentDevice() const;

signals:
    void currentDeviceChanged();

private slots:
    void add(ProjectExplorer::Target *);
    void remove(ProjectExplorer::Target *);
    void activeChanged(ProjectExplorer::Target *);
    void updateDisplayName();

private:
    ProjectExplorer::Project *m_project;
};

/**
  \brief A QComboBox synced with the RunConfigurations of a Target
*/
class RunconfComboBox : public QComboBox
{
    Q_OBJECT

public:
    RunconfComboBox(QWidget *parent = 0);

    void setDevice(ProjectExplorer::Target *);
    ProjectExplorer::RunConfiguration *currentRunConfiguration() const;

signals:
    void currentRunconfChanged();

private slots:
    void add(ProjectExplorer::RunConfiguration *);
    void remove(ProjectExplorer::RunConfiguration *);
    void activeChanged(ProjectExplorer::RunConfiguration *);
    void updateDisplayName();

private:
    ProjectExplorer::Target *m_device;
};

/**
  \brief A QComboBox synced with the current IBackendFactories
*/
class BackendsComboBox : public QComboBox
{
    Q_OBJECT

public:
    BackendsComboBox(QWidget *parent = 0);

    IBackendFactory *currentFactory() const;
    static QList<IBackendFactory *> allFactories();

signals:
    void currentBackendChanged();
};

/**
  \brief Shows compact information about an Inspection
*/
class RunningInspectionWidget : public QWidget
{
    Q_OBJECT

public:
    RunningInspectionWidget(Inspection *, QWidget *parent = 0);

signals:
    void displayInspection(Inspection *);
    void closeInspection(Inspection *);

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

private slots:
    void slotBackendConnected(bool connected);
    void slotDisplayClicked();
    void slotCloseClicked();

private:
    Inspection *m_inspection;
    QLabel *m_connLabel;
    TasksScroller *m_tasksWidget;
};

} // namespace Internal
} // namespace Inspector

#endif // DASHBOARDWINDOW_H
