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

#include "inspectionwindow.h"
#include "abstractpanel.h"
#include "ibackend.h"
#include "inspection.h"
#include "inspectorplugin.h"
#include "inspectorstyle.h"
#include "modulemenuwidget.h"
#include "panelcontainerwidget.h"
#include "statusbarwidget.h"
#include <coreplugin/coreconstants.h>
#include <coreplugin/minisplitter.h>
#include <utils/styledbar.h>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>
#include <QtGui/QStackedWidget>
#include <QtGui/QToolButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QPropertyAnimation>

using namespace Inspector::Internal;

// uncomment following to animate the PanelInfoLabel
#define ANIMATE_PANELINFOLABEL

//
// InspectionWindow
//
InspectionWindow::InspectionWindow(Inspection *inspection, QWidget *parent)
  : QWidget(parent)
  , m_inspection(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    QToolButton *toggleSidebar = new QToolButton;
    toggleSidebar->setIcon(QIcon(Core::Constants::ICON_TOGGLE_SIDEBAR));
    toggleSidebar->setToolTip(tr("Show Sidebar"));
    toggleSidebar->setCheckable(true);
    toggleSidebar->setChecked(true);

    // top
    m_panelInfoLabel = new PanelInfoLabel;
    connect(m_panelInfoLabel, SIGNAL(buttonClicked()),
            this, SLOT(slotCloseInspection()), Qt::QueuedConnection);
    layout->addWidget(m_panelInfoLabel);

    Core::MiniSplitter *horSplitter = new Core::MiniSplitter(Qt::Horizontal);
    layout->addWidget(horSplitter);

    // left panel
    {
        Core::MiniSplitter *leftSplitter = new Core::MiniSplitter(Qt::Vertical);
        horSplitter->addWidget(leftSplitter);

        connect(toggleSidebar, SIGNAL(toggled(bool)),
                leftSplitter, SLOT(setVisible(bool)));

        // upper part
        InspectionWindowSidebar *topPanel = new InspectionWindowSidebar;
        leftSplitter->addWidget(topPanel);

        m_modulesMenu = new ModuleMenuWidget;
        connect(m_modulesMenu, SIGNAL(panelSelected(quint32)),
                this, SLOT(slotShowPanel(quint32)));
        topPanel->addWidget(tr("Panels"), m_modulesMenu);

        // lower part
        InspectionWindowSidebar *bottomPanel = new InspectionWindowSidebar;
        leftSplitter->addWidget(bottomPanel);

        m_sideHelp = new SideHelpWidget;
        bottomPanel->addWidget(tr("Help"), m_sideHelp);

        bottomPanel->addWidget(tr("Notes"), new ColorWidget(Qt::darkGray));

        SideHelpWidget *workflowLabel = new SideHelpWidget;
        workflowLabel->setHelpHtml(tr("<h3>Typical <b>Workflow</b> of a <b>Profiling Process</b>:</h3> * Setup<br> * Data collection<br> * Data preparation<br> * Data mining<br> * Interpretation<br> * Application<br> * Institutional decision<br>"));
        bottomPanel->addWidget(tr("Workflow"), workflowLabel);

        bottomPanel->addWidget(tr("/* Collected Data */"), new ColorWidget(Qt::blue));

        leftSplitter->setSizes(QList<int>() << 200 << 400);
    }

    // right panel
    {
        m_panelContainer = new PanelContainerWidget(this);
        horSplitter->addWidget(m_panelContainer);
    }

    // bottom status bar
    m_statusbarWidget = new Inspector::Internal::StatusBarWidget;
    layout->addWidget(m_statusbarWidget);
    m_statusbarWidget->addButton(toggleSidebar);

    // initial size
    horSplitter->setSizes(QList<int>() << 140 << 660);

    setInspection(inspection);
}

Inspection *InspectionWindow::inspection() const
{
    return m_inspection;
}

void InspectionWindow::slotCloseInspection()
{
    InspectorPlugin::instance()->deleteInspection(m_inspection);
}

void InspectionWindow::slotShowPanel(quint32 compoId)
{
    // create the panel of a module
    int moduleUid = compoId >> 8;
    int panelId = compoId & 0xFF;
    showPanel(moduleUid, panelId);
}

void InspectionWindow::slotSetCurrentPanel(int moduleUid, int panelId)
{
    if ((moduleUid & 0xFF000000) || (panelId & 0xFFFFFF00)) {
        qWarning("InspectionWindow::slotSetCurrentPanel: moduleUid (%d) or panelId (%d) not valid", moduleUid, panelId);
        return;
    }
    quint32 compoId = (moduleUid << 8) + panelId;
    m_modulesMenu->setCurrentItem(compoId);
    emit requestInspectionDisplay();
}

void InspectionWindow::slotBackendConnected(bool connected)
{
    if (connected) {
        m_panelInfoLabel->setContents(tr("Connected"), false);
        m_panelInfoLabel->hideContents();
    } else
        m_panelInfoLabel->setContents(tr("Target disconnected."), true);
}

void InspectionWindow::setInspection(Inspection *inspection)
{
    // remove previous references
    if (m_inspection) {
        disconnect(m_inspection->backend(), 0, this, 0);
        m_modulesMenu->clear();
        m_panelContainer->setPanel(new QWidget);
        m_statusbarWidget->setInspection(0);
    }

    // set the new reference
    m_inspection = inspection;

    if (m_inspection) {
        // connect it
        connect(m_inspection->backend(), SIGNAL(requestPanelDisplay(int,int)),
                this, SLOT(slotSetCurrentPanel(int,int)));
        connect(m_inspection->backend(), SIGNAL(targetConnected(bool)),
                this, SLOT(slotBackendConnected(bool)));
        if (!m_inspection->backend()->isTargetConnected())
            m_panelInfoLabel->setContents(tr("Waiting for connection..."), false);
        else
            slotBackendConnected(true);

        // menu: add all entries by the plugged modules
        ModuleMenuEntries entries = m_inspection->backend()->menuItems();
        foreach (const ModuleMenuEntry &entry, entries) {
            if ((entry.moduleUid & 0xFF000000) || (entry.panelId & 0xFFFFFF00)) {
                qWarning("InspectionWindow::setInspection: moduleUid (%d) or panelId (%d) not valid", entry.moduleUid, entry.panelId);
                continue;
            }
            quint32 compoId = (entry.moduleUid << 8) + entry.panelId;
            m_modulesMenu->addItem(entry.path, compoId, entry.icon);
        }

        // link the taskbar
        m_statusbarWidget->setInspection(m_inspection);

        // show default panel (by selecting the Menu entry)
        quint32 defaultId = m_inspection->backend()->defaultModuleUid() << 8;
        m_modulesMenu->setCurrentItem(defaultId);
    }
}

void InspectionWindow::showPanel(int moduleUid, int panelId)
{
    if (!m_inspection) {
        qWarning("InspectionWindow::showPanel: requested panel %d:%d with not inspection", moduleUid, panelId);
        m_panelContainer->setPanel(new QWidget);
        return;
    }

    // ask for panel creation
    AbstractPanel *panel = m_inspection->backend()->createPanel(moduleUid, panelId);
    if (!panel) {
        qWarning("InspectionWindow::showPanel: can't create panel %d for module %d", panelId, moduleUid);
        m_panelContainer->setPanel(new QWidget);
        m_sideHelp->clearHelp();
        return;
    }

    // set the panel help
    m_sideHelp->setHelpHtml(panel->helpHtml());

    // set the panel
    m_panelContainer->setPanel(panel);
}


//
// PanelInfoLabel
//
PanelInfoLabel::PanelInfoLabel(QWidget *parent)
  : QWidget(parent)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    QHBoxLayout *lay = new QHBoxLayout(this);
    m_label = new QLabel;
    lay->addWidget(m_label);
    lay->addStretch(10);
    m_closeButton = new QPushButton(this);
    m_closeButton->setFlat(true);
    m_closeButton->setMaximumHeight(InspectorStyle::defaultComboHeight());
    m_closeButton->setText(tr("Close Inspection"));
    m_closeButton->setIcon(QIcon(":/debugger/images/debugger_stop_small.png"));
    m_closeButton->hide();
    lay->addWidget(m_closeButton);

    connect(m_closeButton, SIGNAL(clicked()), this, SIGNAL(buttonClicked()));
}

void PanelInfoLabel::setContents(const QString &message, bool showClose)
{
    m_label->setText(message);
    m_closeButton->setVisible(showClose);

#if defined(ANIMATE_PANELINFOLABEL)
    int initialHeight = isVisible() ? height() : 0;
    show();
    layout()->activate();
    animateHeight(initialHeight, layout()->sizeHint().height(), false);
#else
    show();
#endif
}

void PanelInfoLabel::hideContents()
{
#if defined(ANIMATE_PANELINFOLABEL)
    animateHeight(height(), 0, true);
#else
    hide();
#endif
}

void PanelInfoLabel::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(rect(), QColor(255, 255, 200));
    p.fillRect(0, 0, width(), 1, Qt::black);
    p.fillRect(0, height() - 1, width(), 1, Qt::black);
}

void PanelInfoLabel::animateHeight(int from, int to, bool hideAtEnd)
{
    QPropertyAnimation *prop = new QPropertyAnimation(this, "fixedHeight", 0);
    if (hideAtEnd)
        connect(prop, SIGNAL(finished()), this, SLOT(hide()));
    prop->setStartValue(from);
    prop->setEndValue(to);
    prop->setDuration(100);
    prop->setEasingCurve(QEasingCurve::OutQuad);
    prop->start(QPropertyAnimation::DeleteWhenStopped);
}


//
// InspectionWindowSidebar
//
InspectionWindowSidebar::InspectionWindowSidebar(QWidget *parent)
  : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setMargin(0);
    lay->setSpacing(0);

    m_navigationComboBox = new QComboBox(this);
    m_navigationComboBox->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_navigationComboBox->setMinimumContentsLength(0);

    Utils::StyledBar *toolBar = new Utils::StyledBar(this);
    lay->addWidget(toolBar);

    QHBoxLayout *toolBarLayout = new QHBoxLayout;
    toolBarLayout->setMargin(0);
    toolBarLayout->setSpacing(0);
    toolBar->setLayout(toolBarLayout);
    toolBarLayout->addWidget(m_navigationComboBox);
/*
    QToolButton *close = new QToolButton;
    close->setIcon(QIcon(":/core/images/closebutton.png"));
    close->setToolTip(tr("Close"));
    connect(close, SIGNAL(clicked()), this, ...);
    toolBarLayout->addWidget(close);
*/
    m_stack = new QStackedWidget;
    lay->addWidget(m_stack);

    connect(m_navigationComboBox, SIGNAL(currentIndexChanged(int)),
            m_stack, SLOT(setCurrentIndex(int)));
}

void InspectionWindowSidebar::addWidget(const QString &label, QWidget *widget)
{
    m_stack->addWidget(widget);
    m_navigationComboBox->addItem(label);
}


//
// SideHelpWidget
//
SideHelpWidget::SideHelpWidget(QWidget *parent)
  : QTextBrowser(parent)
{
    setFrameStyle(QFrame::NoFrame);
    setTextInteractionFlags(Qt::TextBrowserInteraction);
}

void SideHelpWidget::setHelpHtml(const QString &data)
{
    setHtml(data);
}

void SideHelpWidget::clearHelp()
{
    clear();
}
