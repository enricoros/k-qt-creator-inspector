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
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/runconfiguration.h>
#include <utils/stylehelper.h>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

using namespace Inspector::Internal;

//
// RunControlList
//
RunControlList::RunControlList(QWidget *parent)
  : QWidget(parent)
  , m_buttonsEnabled(true)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    m_noRunningLabel = new QLabel;
    m_noRunningLabel->setText(tr("No Application"));
    m_layout->addWidget(m_noRunningLabel);

    QLabel *epLabel = new QLabel;
    epLabel->setText("<a href='extproc'>" + tr("Attach to Running External Application...") + "</a>");
    epLabel->setContentsMargins(0, 10, 0, 0);
    epLabel->setEnabled(false);
    m_layout->addWidget(epLabel);

    ProjectExplorer::ProjectExplorerPlugin *pep = ProjectExplorer::ProjectExplorerPlugin::instance();
    //foreach (ProjectExplorer::RunControl *rc, pep->runControls())
    //    slotAddRunControl(rc);
    connect(pep, SIGNAL(aboutToStartRunControl(ProjectExplorer::RunControl*)),
            this, SLOT(slotRunControlAdded(ProjectExplorer::RunControl*)));
}

void RunControlList::setButtonsEnabled(bool enabled)
{
    m_buttonsEnabled = enabled;
    foreach (RunControlWidget *widget, m_runWidgets)
        widget->setAttachEnabled(m_buttonsEnabled);
}

void RunControlList::slotRunControlAdded(ProjectExplorer::RunControl *rc)
{
    // listen for RunControl removal
    connect(rc, SIGNAL(destroyed()), this, SLOT(slotRunControlDestroyed()));

    // add a new RunControlWidget
    RunControlWidget *widget = new RunControlWidget(rc);
    widget->setAttachEnabled(m_buttonsEnabled);
    m_layout->insertWidget(m_layout->count() - 1, widget);
    m_runWidgets.append(widget);
    connect(widget, SIGNAL(attachToRunControl(ProjectExplorer::RunControl*)),
            this, SIGNAL(attachToRunControl(ProjectExplorer::RunControl*)));

    // hide the no-label if have something
    m_noRunningLabel->hide();
}

void RunControlList::slotRunControlDestroyed()
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
    if (m_runWidgets.isEmpty())
        m_noRunningLabel->show();
}

//
// RunControlWidget
//
RunControlWidget::RunControlWidget(ProjectExplorer::RunControl *runControl, QWidget *parent)
  : QWidget(parent)
  , m_runControl(runControl)
  , m_attachEnabled(true)
  , m_running(false)
  , m_viaDebugger(false)
{
    // heuristic to detect if running in debugger (FIXME)
    m_viaDebugger = m_runControl->inherits("Debugger::Internal::DebuggerRunControl");

    // TODO: find out if it's tainted (or launched) by Inspector or not

    // TEMP - show that this row is 'special'
    if (m_viaDebugger) {
        QPalette pal;
        pal.setBrush(QPalette::Window, Qt::darkRed);
        setPalette(pal);
        setAutoFillBackground(true);
    }

    connect(m_runControl, SIGNAL(started()),
            this, SLOT(slotRunControlStarted()));
    connect(m_runControl, SIGNAL(finished()),
            this, SLOT(slotRunControlFinished()));

    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setMargin(0);

    QLabel *label1 = new QLabel;
    label1->setText(m_runControl->displayName());
    lay->addWidget(label1);

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

    m_attachButton = new QPushButton;
    m_attachButton->setMaximumHeight(Utils::StyleHelper::navigationWidgetHeight() - 2);
    m_attachButton->setText(tr("Attach"));
    m_attachButton->setIcon(QIcon(":/projectexplorer/images/debugger_start_small.png"));
    connect(m_attachButton, SIGNAL(clicked()), this, SLOT(slotAttachClicked()));
    lay->addWidget(m_attachButton);

    updateActions();
}

ProjectExplorer::RunControl *RunControlWidget::runControl() const
{
    return m_runControl;
}

void RunControlWidget::setAttachEnabled(bool enabled)
{
    m_attachEnabled = enabled;
    updateActions();
}

void RunControlWidget::slotRunControlStarted()
{
    m_statusLabel->setText(tr("running..."));
    m_running = true;
    updateActions();
}

void RunControlWidget::slotRunControlFinished()
{
    m_statusLabel->setText(tr("finished"));
    m_running = false;
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

void RunControlWidget::slotAttachClicked()
{
    emit attachToRunControl(m_runControl);
}

void RunControlWidget::updateActions()
{
    m_debuggingLabel->setVisible(m_viaDebugger);
    m_startLabel->setVisible(!m_viaDebugger && !m_running);
    m_stopLabel->setVisible(m_running);
    m_attachButton->setEnabled(m_attachEnabled && m_running);
}
