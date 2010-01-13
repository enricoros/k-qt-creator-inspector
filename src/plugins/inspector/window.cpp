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

#include "window.h"
#include "abstractpanel.h"
#include "combotreewidget.h"
#include "instance.h"
#include "modulecontroller.h"
#include "panelcontainerwidget.h"
#include "statusbarwidget.h"
#include "module-info/infomodule.h"
#include <QVBoxLayout>

namespace Inspector {
namespace Internal {

Window::Window(QWidget *parent)
  : QWidget(parent)
  , m_extInstance(0)
  , m_menuWidget(0)
  , m_panelContainer(0)
  , m_statusWidget(0)
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_menuWidget = new ComboTreeWidget(this);
    connect(m_menuWidget, SIGNAL(pathSelected(QStringList,QVariant)), this, SLOT(slotMenuChanged(QStringList,QVariant)));
    layout->addWidget(m_menuWidget);

    m_panelContainer = new PanelContainerWidget(this);
    layout->addWidget(m_panelContainer);

    m_statusWidget = new Inspector::Internal::StatusBarWidget(this);
    layout->addWidget(m_statusWidget);
}

void Window::setInstance(Inspector::Instance *instance)
{
    // remove references to any previous instance
    if (m_extInstance) {
        m_menuWidget->clear();
        m_panelContainer->setPanel(new QWidget);
        m_statusWidget->setInstance(0);
    }

    // set the new instance
    m_extInstance = instance;

    // connect it if not null
    if (m_extInstance) {
        // menu: add all entries by the plugged modules
        ModuleMenuEntries entries = m_extInstance->moduleController()->menuEntries();
        foreach (const ModuleMenuEntry &entry, entries) {
            if ((entry.moduleUid & 0xFF000000) || (entry.panelId & 0xFFFFFF00)) {
                qWarning("Window::setInstance: moduleUid (%d) or panelId (%d) not valid", entry.moduleUid, entry.panelId);
                continue;
            }
            quint32 compoId = (entry.moduleUid << 8) + entry.panelId;
            m_menuWidget->addItem(entry.path, compoId, entry.icon);
        }

        // link the taskbar
        m_statusWidget->setInstance(m_extInstance);

        // show information about the current instance
        showPanel(InfoModule::Uid, 0);
    }
}

void Window::slotActivateMenu(int moduleUid, int panelId)
{
    if ((moduleUid & 0xFF000000) || (panelId & 0xFFFFFF00)) {
        qWarning("Window::slotActivateMenu: moduleUid (%d) or panelId (%d) not valid", moduleUid, panelId);
        return;
    }
    quint32 compoId = (moduleUid << 8) + panelId;
    m_menuWidget->setCurrentPath(compoId);
}

void Window::slotMenuChanged(const QStringList &/*path*/, const QVariant &data)
{
    // sanity check on the menu code
    quint32 compoId = data.toInt();
    if (!compoId) {
        qWarning("Window::slotMenuChanged: invalid module/panel ids, skipping panel creation");
        return;
    }

    // create the panel of a module
    int moduleUid = compoId >> 8;
    int panelId = compoId & 0xFF;
    showPanel(moduleUid, panelId);
}

void Window::showPanel(int moduleUid, int panelId)
{
    if (!m_extInstance) {
        qWarning("Window::showPanel: requested panel %d:%d with a null instance", moduleUid, panelId);
        m_panelContainer->setPanel(new QWidget);
        return;
    }

    // ask for panel creation
    AbstractPanel * panel = m_extInstance->moduleController()->createPanel(moduleUid, panelId);
    if (!panel) {
        qWarning("Window::showPanel: can't create panel %d for module %d", panelId, moduleUid);
        m_panelContainer->setPanel(new QWidget);
        return;
    }

    // set the panel
    m_panelContainer->setPanel(panel);
}

} // namespace Internal
} // namespace Inspector
