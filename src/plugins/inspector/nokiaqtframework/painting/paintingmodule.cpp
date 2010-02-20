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

#include "paintingmodule.h"
#include "paintingmodel.h"
#include "temperaturepanel.h"
#include "../datautils.h"
#include "../localcommserver.h"
#include "../nokiaqtframework.h"

using namespace Inspector::Internal;

//
// PaintingModule
//
PaintingModule::PaintingModule(NokiaQtFramework *framework, QObject *parent)
  : IFrameworkModule(framework, parent)
  , m_framework(framework)
{
    m_model = new PaintingModel;
}

PaintingModule::~PaintingModule()
{
    delete m_model;
}

PaintingModel *PaintingModule::model() const
{
    return m_model;
}

QString PaintingModule::name() const
{
    return tr("Painting Measures (0.6)");
}

ModuleMenuEntries PaintingModule::menuEntries() const
{
    ModuleMenuEntries entries;
    entries.append(ModuleMenuEntry(QStringList() << tr("Painting") << tr("Temperature"), UID_MODULE_PAINTING, 1, QIcon(":/inspector/painting/menu-temperature.png")));
    entries.append(ModuleMenuEntry(QStringList() << tr("Painting") << tr("Pixel Energy"), UID_MODULE_PAINTING, 2, QIcon(":/inspector/painting/menu-energy.png")));
    return entries;
}

AbstractPanel *PaintingModule::createPanel(int panelId)
{
    if (panelId == 1)
        return new TemperaturePanel(this);
    qWarning("PaintingModule::createPanel: unknown panel %d", panelId);
    return 0;
}

void PaintingModule::startTemperatureTest(const QString &title, const QVariantList &options)
{
    new PaintingTemperatureTask(m_framework, m_model, options, title);
}

//
// PaintingTemperatureTask
//
PaintingTemperatureTask::PaintingTemperatureTask(NokiaQtFramework *framework, PaintingModel *model,
                                                 const QVariantList &options, const QString &testTitle, QObject *parent)
  : IFrameworkTask(framework, parent)
  , m_framework(framework)
  , m_model(model)
  , m_options(options)
  , m_testTitle(testTitle)
{
    emit requestActivation();
}

QString PaintingTemperatureTask::displayName() const
{
    return tr("Painting Temperature");
}

void PaintingTemperatureTask::activateTask()
{
    connect(m_framework->commServer(), SIGNAL(incomingData(quint32,quint32,QByteArray*)),
            this, SLOT(slotProcessIncomingData(quint32,quint32,QByteArray*)));

    m_framework->callProbeFunction("qWindowTemperature", m_options);
}

void PaintingTemperatureTask::deactivateTask()
{
    disconnect(m_framework->commServer(), 0, this, 0);
    m_model->setPtProgress(100);
    emit finished();
}

void PaintingTemperatureTask::slotProcessIncomingData(quint32 channel, quint32 code1, QByteArray *data)
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
        DataUtils::paintMeshOverImage(&m_lastImage, &mesh, false);
        m_model->addPtResult(QDateTime::currentDateTime(), 1.0, m_testTitle, "options", QPixmap::fromImage(m_lastImage));
        } break;
    default:
        qWarning("PaintingModule::slotProcessIncomingData: unhandled code1 %d", code1);
        break;
    }
}
