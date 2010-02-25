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

#include "thermaltask.h"
#include "thermalmodel.h"
#include "../localcommserver.h"
#include "../nokiaqtframework.h"

// for demarshalling communication with The Probe (FIXME!)
#include "../../../../share/qtcreator/gdbmacros/perfunction.h"

using namespace Inspector::Internal;

ThermalTask::ThermalTask(NokiaQtFramework *framework, ThermalModel *model,
                         const QVariantList &options, const QString &testTitle, QObject *parent)
  : IFrameworkTask(framework, parent)
  , m_framework(framework)
  , m_model(model)
  , m_options(options)
  , m_testTitle(testTitle)
{
    emit requestActivation();
}

QString ThermalTask::displayName() const
{
    return tr("Thermal");
}

void ThermalTask::activateTask()
{
    connect(m_framework->commServer(), SIGNAL(incomingData(quint32,quint32,QByteArray*)),
            this, SLOT(slotProcessIncomingData(quint32,quint32,QByteArray*)));

    m_optionsString = tr("parameters: ");
    bool skipFirstComma = true;
    foreach (const QVariant &option, m_options) {
        QString s = option.toString();
        if (s.isEmpty())
            continue;
        if (skipFirstComma)
            skipFirstComma = false;
        else
            m_optionsString += ", ";
        m_optionsString += s;
    }
    m_startDate = QDateTime::currentDateTime();

    m_framework->callProbeFunction("qThermalAnalysis", m_options);
}

void ThermalTask::deactivateTask()
{
    disconnect(m_framework->commServer(), 0, this, 0);
    m_model->setPtProgress(100);
    emit finished();
}

void ThermalTask::slotProcessIncomingData(quint32 channel, quint32 code1, QByteArray *data)
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
    case 3: {   // percent
        int percent = qBound(0, QString(*data).toInt(), 100);
        m_model->setPtProgress(percent);
        setProgress(percent);
        } break;
    case 4:     // base image
        LocalCommServer::decodeImage(data, &m_lastImage);
        break;
    case 5: {   // mesh data
        Inspector::Probe::RegularMeshRealData mesh;
        LocalCommServer::decodeMesh(data, &mesh);
        int duration = m_startDate.secsTo(QDateTime::currentDateTime());
        m_model->addResult(m_startDate, duration, m_testTitle, m_optionsString, m_lastImage, mesh);
        } break;
    default:
        qWarning("ThermalTask::slotProcessIncomingData: unhandled code1 %d", code1);
        break;
    }
}
