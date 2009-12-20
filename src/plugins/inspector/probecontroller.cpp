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

#include "probecontroller.h"

#include "abstractprobe.h"

using namespace Inspector::Internal;

ProbeController::ProbeController(QObject *parent)
  : QObject(parent)
{
}

ProbeController::~ProbeController()
{
    // delete all the probes (bypassing the 'destroyed' hook)
    QList<AbstractProbe *> listCopy = m_probes;
    m_probes.clear();
    qDeleteAll(listCopy);
}

void ProbeController::addProbe(AbstractProbe * probe)
{
    if (!probe) {
        qWarning("ProbeController::addProbe: skipping null probe");
        return;
    }
    // register the Probe
    connect(probe, SIGNAL(requestActivation()), this, SLOT(slotProbeActivationRequested()));
    connect(probe, SIGNAL(deactivated()), this, SLOT(slotProbeDeactivated()));
    connect(probe, SIGNAL(destroyed()), this, SLOT(slotProbeDestroyed()));
    m_probes.append(probe);
    emit probesChanged();
}

void ProbeController::removeProbe(AbstractProbe * probe)
{
    if (!probe) {
        qWarning("ProbeController::removeProbe: skipping null probe");
        return;
    }
    // unregister the Probe
    disconnect(probe, 0, this, 0);
    m_probes.removeAll(probe);
    m_activeProbes.removeAll(probe);
    emit probesChanged();
}

ProbeMenuEntries ProbeController::menuEntries() const
{
    ProbeMenuEntries entries;
    foreach (AbstractProbe *probe, m_probes)
        entries.append(probe->menuEntries());
    return entries;
}

QStringList ProbeController::probeNames() const
{
    QStringList names;
    foreach (AbstractProbe *probe, m_probes)
        names.append(probe->name());
    return names;
}

QWidget *ProbeController::createView(int probeUid, int viewId) const
{
    AbstractProbe * probe = probeForUid(probeUid);
    if (!probe) {
        qWarning("ProbeController::createView: unknown probe Uid %d", probeUid);
        return 0;
    }
    return probe->createView(viewId);
}

void ProbeController::activatePTProbe()
{
    qWarning("ProbeController::activatePTProbe: TODO");
}

AbstractProbe *ProbeController::probeForUid(int probeUid) const
{
    foreach (AbstractProbe *probe, m_probes)
        if (probe->uid() == probeUid)
            return probe;
    return 0;
}

void ProbeController::slotProbeActivationRequested()
{
    AbstractProbe * probe = static_cast<AbstractProbe *>(sender());
    probe->controlActivate();
    if (!m_activeProbes.contains(probe))
        m_activeProbes.append(probe);
}

void ProbeController::slotProbeDeactivated()
{
    AbstractProbe * probe = static_cast<AbstractProbe *>(sender());
    m_activeProbes.removeAll(probe);
}

void ProbeController::slotProbeDestroyed()
{
    AbstractProbe * probe = static_cast<AbstractProbe *>(sender());
    if (m_probes.contains(probe))
        removeProbe(probe);
}
