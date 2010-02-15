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

#include "runcontrolwatcher.h"
#include "inspectorrunner.h"
#include <debugger/debuggerdialogs.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/runconfiguration.h>
#include <utils/stylehelper.h>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QVBoxLayout>

using namespace Inspector::Internal;

//
// RunningTargetSelectorWidget
//
RunningTargetSelectorWidget::RunningTargetSelectorWidget(QWidget *parent)
  : QWidget(parent)
{
    m_buttonGroup = new QButtonGroup(this);

    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);

    /*m_noRunningLabel = new QRadioButton;
    m_noRunningLabel->setText(tr("No Application"));
    m_noRunningLabel->setEnabled(false);
    m_layout->addWidget(m_noRunningLabel);*/

    AttachToPidWidget *ap = new AttachToPidWidget(m_buttonGroup);
    connect(ap, SIGNAL(attachPidSelected(quint64)),
            this, SLOT(slotPidSelected(quint64)));
    m_layout->addWidget(ap);

    ProjectExplorer::ProjectExplorerPlugin *pep = ProjectExplorer::ProjectExplorerPlugin::instance();
    //foreach (ProjectExplorer::RunControl *rc, pep->runControls())
    //    slotAddRunControl(rc);
    connect(pep, SIGNAL(aboutToStartRunControl(ProjectExplorer::RunControl*)),
            this, SLOT(slotRunControlAdded(ProjectExplorer::RunControl*)));
}

void RunningTargetSelectorWidget::slotRunControlAdded(ProjectExplorer::RunControl *rc)
{
    // listen for RunControl removal
    connect(rc, SIGNAL(destroyed()), this, SLOT(slotRunControlDestroyed()));

    // add a new RunControlWidget
    RunControlWidget *widget = new RunControlWidget(m_buttonGroup, rc);
    m_layout->insertWidget(m_layout->count() - 1, widget);
    m_runWidgets.append(widget);
    connect(widget, SIGNAL(runControlSelected(ProjectExplorer::RunControl*)),
            this, SLOT(slotRunControlSelected(ProjectExplorer::RunControl*)));

    // hide the no-label if have something
    /*m_noRunningLabel->hide();*/
}

void RunningTargetSelectorWidget::slotRunControlDestroyed()
{
    // remove the associated widget
    ProjectExplorer::RunControl *rc = static_cast<ProjectExplorer::RunControl *>(sender());
    foreach (RunControlWidget *widget, m_runWidgets) {
        if (widget->runControl() == rc) {
            m_runWidgets.removeAll(widget);
            disconnect(widget, 0, this, 0);
            widget->deleteLater();
            break;
        }
    }

    // show the no-label if empty
    /*if (m_runWidgets.isEmpty())
        m_noRunningLabel->show();*/
}

void RunningTargetSelectorWidget::slotRunControlSelected(ProjectExplorer::RunControl *rc)
{
    InspectionTarget target;
    target.type = InspectionTarget::HijackRunControl;
    target.runControl = rc;
    target.displayName = rc->displayName();
    emit inspectionTargetSelected(target);
}

void RunningTargetSelectorWidget::slotPidSelected(quint64 pid)
{
    InspectionTarget target;
    target.type = InspectionTarget::AttachToPid;
    target.pid = pid;
    target.displayName = tr("Process %1").arg(pid);
    emit inspectionTargetSelected(target);
}

//
// RunControlWidget
//
RunControlWidget::RunControlWidget(QButtonGroup *group, ProjectExplorer::RunControl *runControl, QWidget *parent)
  : QWidget(parent)
  , m_runControl(runControl)
  , m_rcType(Unknown)
  , m_rcRunning(false)
{
    // heuristics to determine the runcontrol type
    if (qobject_cast<InspectorRunControl *>(runControl))
        m_rcType = InspectorRunning;
    else if (m_runControl->inherits("Debugger::Internal::DebuggerRunControl")) // HARDCODED, FIXME
        m_rcType = DebuggerRunning;
    else
        m_rcType = LocalAppRunning;

    // TEMP - show that this row is 'special'
    /*if (m_viaDebugger) {
        QPalette pal;
        pal.setBrush(QPalette::Window, Qt::darkRed);
        setPalette(pal);
        setAutoFillBackground(true);
    }*/

    connect(m_runControl, SIGNAL(started()),
            this, SLOT(slotRunControlStarted()));
    connect(m_runControl, SIGNAL(finished()),
            this, SLOT(slotRunControlFinished()));

    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setMargin(0);
    lay->setSpacing(0);

    QRadioButton *radio = new QRadioButton;
    switch (m_rcType) {
    case Unknown:
        radio->setText(m_runControl->displayName());
        break;
    case LocalAppRunning:
        radio->setText(tr("Running: %1").arg(m_runControl->displayName()));
        break;
    case DebuggerRunning:
        radio->setText(tr("Debugging: %1").arg(m_runControl->displayName()));
        break;
    case InspectorRunning:
        radio->setText(tr("Inspecting: %1").arg(m_runControl->displayName()));
        break;
    }
    connect(radio, SIGNAL(toggled(bool)),
            this, SLOT(slotToggled(bool)));
    radio->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    group->addButton(radio);
    lay->addWidget(radio);

    lay->addStretch(2);

    m_debuggingLabel = new QLabel;
    m_debuggingLabel->setText(tr("<b>[debugging]</b>"));
    lay->addWidget(m_debuggingLabel);

    lay->addStretch(2);

    QFont italicFont;
    italicFont.setItalic(true);
    m_statusLabel = new QLabel;
    m_statusLabel->setFont(italicFont);
    lay->addWidget(m_statusLabel);

    lay->addStretch(2);

    m_startLabel = new QLabel;
    m_startLabel->setFont(italicFont);
    m_startLabel->setText("<a href='start'>" + tr("start") + "</a>");
    connect(m_startLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotStartClicked()));
    lay->addWidget(m_startLabel);

    m_stopLabel = new QLabel;
    m_stopLabel->setFont(italicFont);
    m_stopLabel->setText("<a href='stop'>" + tr("stop") + "</a>");
    connect(m_stopLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotStopClicked()));
    lay->addWidget(m_stopLabel);

    lay->addStretch(100);

    updateActions();
}

ProjectExplorer::RunControl *RunControlWidget::runControl() const
{
    return m_runControl;
}

RunControlWidget::Type RunControlWidget::runControlType() const
{
    return m_rcType;
}

void RunControlWidget::slotRunControlStarted()
{
    m_statusLabel->setText(tr("running..."));
    m_rcRunning = true;
    updateActions();
}

void RunControlWidget::slotRunControlFinished()
{
    m_statusLabel->setText(tr("finished"));
    m_rcRunning = false;
    updateActions();
}

void RunControlWidget::slotStartClicked()
{
    m_runControl->start();
}

void RunControlWidget::slotStopClicked()
{
    m_runControl->stop();
}

void RunControlWidget::slotToggled(bool checked)
{
    if (checked)
        emit runControlSelected(m_runControl);
}

void RunControlWidget::updateActions()
{
    m_debuggingLabel->setVisible(m_rcType == DebuggerRunning);
    m_startLabel->setVisible(m_rcType != DebuggerRunning && !m_rcRunning);
    m_stopLabel->setVisible(m_rcRunning);
}

//
// AttachToPidWidget
//
AttachToPidWidget::AttachToPidWidget(QButtonGroup *group, QWidget *parent)
  : QWidget(parent)
  , m_pid(0)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setMargin(0);
    lay->setSpacing(0);

    m_radio = new QRadioButton;
    m_radio->setText(tr("Attach to Running External Application..."));
    connect(m_radio, SIGNAL(toggled(bool)),
            this, SLOT(slotToggled(bool)));
    group->addButton(m_radio);
    lay->addWidget(m_radio);

    m_pidSelectLabel = new QLabel;
    m_pidSelectLabel->setText("<a href='extproc'>" + tr("select application") + "</a>");
    connect(m_pidSelectLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotSelectPidClicked()));
    lay->addWidget(m_pidSelectLabel);

    lay->addStretch(100);
}

void AttachToPidWidget::slotSelectPidClicked()
{
    Debugger::Internal::AttachExternalDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted || !dlg.attachPID())
        return;
    m_pid = dlg.attachPID();
    m_pidSelectLabel->setText(tr("[<a href='extproc'>%1</a>]").arg(m_pid));
    if (!m_radio->isChecked())
        m_radio->setChecked(true);
    emit attachPidSelected(m_pid);
}

void AttachToPidWidget::slotToggled(bool checked)
{
    if (!checked)
        return;
    if (m_pid) {
        emit attachPidSelected(m_pid);
        return;
    }
    slotSelectPidClicked();
    if (!m_pid)
        m_radio->setChecked(false);
}
