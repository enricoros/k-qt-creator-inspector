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
#include "abstractview.h"
#include "combotreewidget.h"
#include "instance.h"
#include "modulecontroller.h"
#include "taskbarwidget.h"
#include "viewcontainerwidget.h"
#include "module-info/infomodule.h"
#include <QVBoxLayout>

namespace Inspector {
namespace Internal {

Window::Window(QWidget *parent)
  : QWidget(parent)
  , m_extInstance(0)
  , m_menuWidget(0)
  , m_viewWidget(0)
  , m_taskbarWidget(0)
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_menuWidget = new ComboTreeWidget(this);
    connect(m_menuWidget, SIGNAL(pathSelected(QStringList,QVariant)), this, SLOT(slotMenuChanged(QStringList,QVariant)));
    layout->addWidget(m_menuWidget);

    m_viewWidget = new ViewContainerWidget(this);
    layout->addWidget(m_viewWidget);

    m_taskbarWidget = new TaskbarWidget(this);
    layout->addWidget(m_taskbarWidget);
}

void Window::setInstance(Inspector::Instance *instance)
{
    // remove references to any previous instance
    if (m_extInstance) {
        m_menuWidget->clear();
        m_viewWidget->setWidget(new QWidget);
        m_taskbarWidget->clear();
    }

    // set the new instance
    m_extInstance = instance;

    // connect it if not null
    if (m_extInstance) {
        // menu: add all entries by the plugged modules
        ModuleMenuEntries entries = m_extInstance->moduleController()->menuEntries();
        foreach (const ModuleMenuEntry &entry, entries) {
            if ((entry.moduleUid & 0xFF000000) || (entry.viewId & 0xFFFFFF00)) {
                qWarning("Window::setInstance: moduleUid (%d) or viewId (%d) not valid", entry.moduleUid, entry.viewId);
                continue;
            }
            quint32 compoId = (entry.moduleUid << 8) + entry.viewId;
            m_menuWidget->addItem(entry.path, compoId, entry.icon);
        }

        // show information about the current instance
        showView(InfoModule::Uid, 0);

        // TODO link the taskbar
        m_taskbarWidget->clear();
    }
}

void Window::slotMenuChanged(const QStringList &/*path*/, const QVariant &data)
{
    // sanity check on the menu code
    quint32 compoId = data.toInt();
    if (!compoId) {
        qWarning("Window::slotMenuChanged: invalid module/view ids, skipping view creation");
        return;
    }

    // create the view of a module
    int moduleUid = compoId >> 8;
    int viewId = compoId & 0xFF;
    showView(moduleUid, viewId);
}

void Window::showView(int moduleUid, int viewId)
{
    if (!m_extInstance) {
        qWarning("Window::showView: requested view %d:%d with a null instance", moduleUid, viewId);
        m_viewWidget->setWidget(new QWidget);
        return;
    }

    // ask for view creation
    AbstractView * view = m_extInstance->moduleController()->createView(moduleUid, viewId);
    if (!view) {
        qWarning("Window::showView: can't create view %d for module %d", viewId, moduleUid);
        m_viewWidget->setWidget(new QWidget);
        return;
    }

    // set the view
    m_viewWidget->setWidget(view);
}

} // namespace Internal
} // namespace Inspector
