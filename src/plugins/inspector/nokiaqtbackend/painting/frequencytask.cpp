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

#include "frequencytask.h"
#include "../localcommserver.h"
#include "../nokiaqtbackend.h"

// for demarshalling communication with The Probe (FIXME!)
#include "../../../../../share/qtcreator/gdbmacros/perfunction.h"
#include "../../../../../share/qtcreator/gdbmacros/probedata.h"

using namespace Inspector::Internal;

FrequencyTask::FrequencyTask(NokiaQtBackend *backend, QObject *parent)
  : IBackendTask(backend, parent)
  , m_nqBackend(backend)
{
    emit requestActivation();
}

QString FrequencyTask::displayName() const
{
    return tr("Paint Frequency");
}

void FrequencyTask::activateTask()
{
    connect(m_nqBackend->commServer(), SIGNAL(incomingData(quint32,quint32,QByteArray*)),
            this, SLOT(slotProcessIncomingData(quint32,quint32,QByteArray*)));

    m_nqBackend->callProbeFunction("qPaintingFrequencyAnalysis", QVariantList() << true);
}

void FrequencyTask::deactivateTask()
{
    disconnect(m_nqBackend->commServer(), 0, this, 0);
    m_nqBackend->callProbeFunction("qPaintingFrequencyAnalysis", QVariantList() << false);
    emit finished();
}

void FrequencyTask::slotProcessIncomingData(quint32 channel, quint32 code1, QByteArray *data)
{
    // only filter comm by this Uid (NOTE: sync the probe impl)
    if (channel != Inspector::Probe::Channel_Painting)
        return;

    switch (code1) {
    case 1:     // begin
        break;
    case 2:     // end
        deactivateTask();
        break;
    case 6: {   // area
        Inspector::Probe::AreaData area;
        LocalCommServer::decodeArea(data, &area);
        qWarning() << "got area" << area.absoluteRect;
        } break;
    case 7: {   // area clear
        qWarning("area clear");
        } break;
    default:
        qWarning("FrequencyTask::slotProcessIncomingData: unhandled code1 %d", code1);
        break;
    }
}
