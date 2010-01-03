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

#include "infomodule.h"
#include "infoview.h"

using namespace Inspector::Internal;

InfoModule::InfoModule(Inspector::Instance *instance, QObject *parent)
  : AbstractModule(instance, parent)
{
}

InfoModule::~InfoModule()
{
    qDeleteAll(m_views);
}

QString InfoModule::name() const
{
    return tr("Information (0.8)");
}

ModuleMenuEntries InfoModule::menuEntries() const
{
    ModuleMenuEntries entries;
    entries.append(ModuleMenuEntry(QStringList() << tr("Status"), Uid, 0, QIcon(":/inspector/images/menu-status.png")));
    return entries;
}

AbstractView *InfoModule::createView(int viewId)
{
    if (viewId != 0) {
        qWarning("InfoModule::createView: unhandled view %d", viewId);
        return 0;
    }
    AbstractView *view = new InfoView(this);
    connect(view, SIGNAL(destroyed()), this, SLOT(slotViewDestroyed()));
    m_views.append(view);
    return view;
}

void InfoModule::slotActivate()
{
    qWarning("InfoModule::slotActivate: ACTIVATED");
}

void InfoModule::slotDeactivate()
{
    qWarning("InfoModule::slotDeactivate: DEACTIVATED");
    emit deactivated();
}

void InfoModule::slotLock()
{
    foreach (AbstractView *view, m_views)
        view->setEnabled(false);
}

void InfoModule::slotUnlock()
{
    foreach (AbstractView *view, m_views)
        view->setEnabled(true);
}

void InfoModule::slotViewDestroyed()
{
    AbstractView *view = static_cast<AbstractView *>(sender());
    m_views.removeAll(view);
}
