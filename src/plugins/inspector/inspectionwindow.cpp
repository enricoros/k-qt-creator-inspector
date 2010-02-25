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
#include "iframework.h"
#include "inspection.h"
#include "inspectorplugin.h"
#include "inspectorstyle.h"
#include "modulemenuwidget.h"
#include "panelcontainerwidget.h"
#include "statusbarwidget.h"
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

    layout->addWidget(new SunkenBar(true, this));

    m_panelInfoLabel = new PanelInfoLabel;
    connect(m_panelInfoLabel, SIGNAL(buttonClicked()),
            this, SLOT(slotCloseInspection()), Qt::QueuedConnection);
    layout->addWidget(m_panelInfoLabel);

    m_horSplitter = new Core::MiniSplitter;
    layout->addWidget(m_horSplitter);

    m_sideBar = new InspectionWindowSidebar;
    connect(m_sideBar, SIGNAL(collapse()),
            this, SLOT(slotCollapseSideBar()));
    m_horSplitter->addWidget(m_sideBar);

    m_panelContainer = new PanelContainerWidget(this);
    m_horSplitter->addWidget(m_panelContainer);

    m_statusbarWidget = new Inspector::Internal::StatusBarWidget;
    layout->addWidget(m_statusbarWidget);

    // add panels to the sidebar
    m_sideMenu = new ModuleMenuWidget;
    connect(m_sideMenu, SIGNAL(panelSelected(quint32)),
            this, SLOT(slotMenuChanged(quint32)));
    m_sideBar->addEntry(tr("Modules / Panels"), m_sideMenu);

    m_sideBar->addEntry(tr("Notes"), new ColorWidget(Qt::darkGray));
    m_sideBar->addEntry(tr("/* Collected Data */"), new ColorWidget(Qt::blue));
    m_sideBar->addEntry(tr("/* Workflow */"), new QWidget);

    m_horSplitter->setCollapsible(0, true);
    m_horSplitter->setSizes(QList<int>() << 150 << 600);

    setInspection(inspection);
}

Inspection *InspectionWindow::inspection() const
{
    return m_inspection;
}

void InspectionWindow::slotCollapseSideBar()
{
    m_horSplitter->setSizes(QList<int>() << 0 << width());
}

void InspectionWindow::slotCloseInspection()
{
    InspectorPlugin::instance()->deleteInspection(m_inspection);
}

void InspectionWindow::slotMenuChanged(quint32 compoId)
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
    m_sideMenu->setCurrentItem(compoId);
    emit requestInspectionDisplay();
}

void InspectionWindow::slotFrameworkConnected(bool connected)
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
        disconnect(m_inspection->framework(), 0, this, 0);
        m_sideMenu->clear();
        m_panelContainer->setPanel(new QWidget);
        m_statusbarWidget->setInspection(0);
    }

    // set the new reference
    m_inspection = inspection;

    if (m_inspection) {
        // connect it
        connect(m_inspection->framework(), SIGNAL(requestPanelDisplay(int,int)),
                this, SLOT(slotSetCurrentPanel(int,int)));
        connect(m_inspection->framework(), SIGNAL(targetConnected(bool)),
                this, SLOT(slotFrameworkConnected(bool)));
        if (!m_inspection->framework()->isTargetConnected())
            m_panelInfoLabel->setContents(tr("Waiting for connection..."), false);
        else
            slotFrameworkConnected(true);

        // menu: add all entries by the plugged modules
        ModuleMenuEntries entries = m_inspection->framework()->menuItems();
        foreach (const ModuleMenuEntry &entry, entries) {
            if ((entry.moduleUid & 0xFF000000) || (entry.panelId & 0xFFFFFF00)) {
                qWarning("InspectionWindow::setInspection: moduleUid (%d) or panelId (%d) not valid", entry.moduleUid, entry.panelId);
                continue;
            }
            quint32 compoId = (entry.moduleUid << 8) + entry.panelId;
            m_sideMenu->addItem(entry.path, compoId, entry.icon);
        }

        // link the taskbar
        m_statusbarWidget->setInspection(m_inspection);

        // show default panel
        showPanel(m_inspection->framework()->defaultModuleUid(), 0);
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
    AbstractPanel *panel = m_inspection->framework()->createPanel(moduleUid, panelId);
    if (!panel) {
        qWarning("InspectionWindow::showPanel: can't create panel %d for module %d", panelId, moduleUid);
        m_panelContainer->setPanel(new QWidget);
        return;
    }

    // set the panel
    m_panelContainer->setPanel(panel);
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

    QToolButton *close = new QToolButton;
    close->setIcon(QIcon(":/core/images/closebutton.png"));
    close->setToolTip(tr("Close"));
    connect(close, SIGNAL(clicked()), this, SIGNAL(collapse()));
    toolBarLayout->addWidget(close);

    m_stack = new QStackedWidget;
    lay->addWidget(m_stack);

    connect(m_navigationComboBox, SIGNAL(currentIndexChanged(int)),
            m_stack, SLOT(setCurrentIndex(int)));
}

void InspectionWindowSidebar::addEntry(const QString &label, QWidget *widget)
{
    m_stack->addWidget(widget);
    m_navigationComboBox->addItem(label);
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
    m_closeButton->setIcon(QIcon(":/projectexplorer/images/stop_small.png"));
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
