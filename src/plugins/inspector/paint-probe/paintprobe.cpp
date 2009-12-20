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

#include "paintprobe.h"
#include "painttemperatureview.h"

using namespace Inspector::Internal;

PaintProbe::PaintProbe(QObject *parent)
  : AbstractProbe(parent)
{
}

PaintProbe::~PaintProbe()
{
    qDeleteAll(m_views);
}

QString PaintProbe::name() const
{
    return tr("Painting Tests");
}

ProbeMenuEntries PaintProbe::menuEntries() const
{
    ProbeMenuEntries entries;
    entries.append(ProbeMenuEntry(QStringList() << tr("Painting") << tr("Temperature"), Uid, 1));
    entries.append(ProbeMenuEntry(QStringList() << tr("Painting") << tr("Pixel Energy"), Uid, 2));
    return entries;
}

AbstractView *PaintProbe::createView(int viewId)
{
    AbstractView *view = 0;
    if (viewId == 1)
        view = new PaintTemperatureView;
    else
        qWarning("PaintProbe::createView: unknown view %d", viewId);
    if (view) {
        connect(view, SIGNAL(destroyed()), this, SLOT(slotViewDestroyed()));
        m_views.append(view);
    }
    return view;
}

void PaintProbe::slotActivate()
{
    qWarning("PaintProbe::slotActivate: ACTIVATED");
}

void PaintProbe::slotDeactivate()
{
    qWarning("PaintProbe::slotDeactivate: DEACTIVATED");
    emit deactivated();
}

void PaintProbe::slotLock()
{
    foreach (AbstractView * view, m_views)
        view->setEnabled(false);
}

void PaintProbe::slotUnlock()
{
    foreach (AbstractView * view, m_views)
        view->setEnabled(true);
}

void PaintProbe::slotViewDestroyed()
{
    AbstractView *view = static_cast<AbstractView *>(sender());
    m_views.removeAll(view);
}
