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
#include "instance.h"
#include "inspectorplugin.h"
#include "inspectorwindow.h"
#include "singletabwidget.h"
#include "targetwindow.h"
#include <QtGui/QStackedWidget>
#include <QtGui/QVBoxLayout>

using namespace Inspector::Internal;

InspectorContainer::InspectorContainer(QWidget *parent)
  : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_topbarWidget = new SingleTabWidget(this);
    m_topbarWidget->setTitle(tr("Select a Target:"));
    layout->addWidget(m_topbarWidget);

    m_centralWidget = new QStackedWidget(this);
    layout->addWidget(m_centralWidget);

    m_topbarWidget->addTab(tr("Workbench"));
    m_inspectorWindow = new InspectorWindow;
    m_centralWidget->insertWidget(0, m_inspectorWindow);
    m_centralWidget->setCurrentWidget(m_inspectorWindow);

    connect(m_topbarWidget, SIGNAL(currentIndexChanged(int)),
            m_centralWidget, SLOT(setCurrentIndex(int)));

    connect(m_inspectorWindow, SIGNAL(requestDisplay()),
            this, SLOT(slotDisplayInspectorWindow()));

    InspectorPlugin *plugin = InspectorPlugin::pluginInstance();
    foreach (Instance *instance, plugin->instances())
        slotInstanceAdded(instance);
    connect(plugin, SIGNAL(instanceAdded(Instance*)),
            this, SLOT(slotInstanceAdded(Instance*)));
    connect(plugin, SIGNAL(instanceRemoved(Instance*)),
            this, SLOT(slotInstanceRemoved(Instance*)));
}

void InspectorContainer::slotInstanceAdded(Instance *instance)
{
    // if already present, just show it
    foreach (TargetWindow *target, m_targets) {
        if (target->targetInstance() == instance) {
            m_centralWidget->setCurrentWidget(target);
            return;
        }
    }

    // create a new TargetWindow
    TargetWindow *targetWindow = new TargetWindow(instance);
    connect(targetWindow, SIGNAL(requestTargetDisplay()),
            this, SLOT(slotDisplayTargetWindow()));
    m_centralWidget->addWidget(targetWindow);
    m_topbarWidget->addTab(instance->instanceModel()->displayName());
    m_targets.append(targetWindow);

    // switch to that
    m_topbarWidget->setCurrentIndex(m_topbarWidget->tabCount() - 1);

    // enforce re-display because the debugger window steals
    // the focus here
    slotDisplayTargetWindow();
}

void InspectorContainer::slotInstanceRemoved(Instance *instance)
{
    // remove all the widgetry associated to the instance
    int tabIndex = 1;   // 0 is the 'Workbench' label
    foreach (TargetWindow *targetWindow, m_targets) {
        if (targetWindow->targetInstance() == instance) {
            m_targets.removeAll(targetWindow);
            disconnect(targetWindow, 0, this, 0);
            m_centralWidget->removeWidget(targetWindow);
            targetWindow->deleteLater();
            m_topbarWidget->removeTab(tabIndex);
            return;
        }
        ++tabIndex;
    }
}

void InspectorContainer::slotDisplayInspectorWindow()
{
    // show myself
    emit requestWindowDisplay();
}

void InspectorContainer::slotDisplayTargetWindow()
{
    // switch to the target
    TargetWindow *targetWindow = dynamic_cast<TargetWindow *>(sender());
    if (targetWindow)
        m_centralWidget->setCurrentWidget(targetWindow);

    // show myself
    emit requestWindowDisplay();
}
