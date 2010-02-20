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

#include "inspectorcontainer.h"
#include "dashboardwindow.h"
#include "inspection.h"
#include "inspectionwindow.h"
#include "inspectorplugin.h"
#include "singletabwidget.h"
#include <QtGui/QGradient>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QStackedWidget>
#include <QtGui/QVBoxLayout>

using namespace Inspector::Internal;

InspectorContainer::InspectorContainer(QWidget *parent)
  : WatermarkedWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_topbarWidget = new SingleTabWidget(this);
    m_topbarWidget->setTitle(tr("Select:"));
    layout->addWidget(m_topbarWidget);

    m_centralWidget = new QStackedWidget(this);
    layout->addWidget(m_centralWidget);

    m_topbarWidget->addTab(tr("Dashboard"));
    m_dashboardWindow = new DashboardWindow;
    m_centralWidget->insertWidget(0, m_dashboardWindow);
    m_centralWidget->setCurrentWidget(m_dashboardWindow);

    connect(m_topbarWidget, SIGNAL(currentIndexChanged(int)),
            m_centralWidget, SLOT(setCurrentIndex(int)));

    connect(m_dashboardWindow, SIGNAL(requestInspectionDisplay(Inspection*)),
            this, SLOT(slotDisplayInspection(Inspection*)));

    InspectorPlugin *plugin = InspectorPlugin::instance();
    foreach (Inspection *inspection, plugin->inspections())
        slotInspectionAdded(inspection);
    connect(plugin, SIGNAL(inspectionAdded(Inspection*)),
            this, SLOT(slotInspectionAdded(Inspection*)));
    connect(plugin, SIGNAL(inspectionRemoved(Inspection*)),
            this, SLOT(slotInspectionRemoved(Inspection*)));
}

void InspectorContainer::slotInspectionAdded(Inspection *inspection)
{
    // if already present, just show it
    foreach (InspectionWindow *inspectionWindow, m_inspections) {
        if (inspectionWindow->inspection() == inspection) {
            m_centralWidget->setCurrentWidget(inspectionWindow);
            return;
        }
    }

    // create a new InspectionWindow
    InspectionWindow *inspectionWindow = new InspectionWindow(inspection);
    connect(inspectionWindow, SIGNAL(requestInspectionDisplay()),
            this, SLOT(slotDisplayInspectionWindow()));
    m_centralWidget->addWidget(inspectionWindow);
    m_topbarWidget->addTab(inspection->inspectionModel()->displayName());
    m_inspections.append(inspectionWindow);

    // switch to that
    m_topbarWidget->setCurrentIndex(m_topbarWidget->tabCount() - 1);
}

void InspectorContainer::slotInspectionRemoved(Inspection *inspection)
{
    // remove all the associated widgetry
    int tabIndex = 1;   // 0 is the 'Dashboard' label
    foreach (InspectionWindow *inspectionWindow, m_inspections) {
        if (inspectionWindow->inspection() == inspection) {
            m_inspections.removeAll(inspectionWindow);
            disconnect(inspectionWindow, 0, this, 0);
            m_centralWidget->removeWidget(inspectionWindow);
            delete inspectionWindow;
            m_topbarWidget->removeTab(tabIndex);
            return;
        }
        ++tabIndex;
    }
}

void InspectorContainer::slotDisplayInspection(Inspection *inspection)
{
    int tabIndex = 1;   // 0 is the 'Dashboard' label
    foreach (InspectionWindow *inspectionWindow, m_inspections) {
        if (inspectionWindow->inspection() == inspection) {
            m_topbarWidget->setCurrentIndex(tabIndex);
            return;
        }
        ++tabIndex;
    }
}

void InspectorContainer::slotDisplayInspectionWindow()
{
    // switch to the InspectionWindow
    if (InspectionWindow *inspectionWindow = dynamic_cast<InspectionWindow *>(sender()))
        slotDisplayInspection(inspectionWindow->inspection());

    // show myself
    emit requestWindowDisplay();
}
