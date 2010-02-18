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

#include "nokiaqtframework.h"
#include "inspectorplugin.h"
#include "localcommserver.h"
#include "nokiaqtinspectionmodel.h"
#include "probeinjectingdebugger.h"
#include "shareddebugger.h"

#include "blueprint/blueprintmodule.h"
#include "info/infomodule.h"
#include "painting/paintingmodule.h"
#include "warnings/warningsmodule.h"

#include <QtGui/QMessageBox>

using namespace Inspector::Internal;

//
// NokiaQtFramework
//
NokiaQtFramework::NokiaQtFramework(NokiaQtInspectionModel *model, ProbeInjectingDebugger *debugger, QObject *parent)
  : IFramework(model, parent)
  , m_piDebugger(debugger)
  , m_model(model)
{
    m_commServer = new LocalCommServer(m_model);

    connect(m_piDebugger, SIGNAL(targetRunningChanged(bool)),
            this, SLOT(slotTargetRunningChanged(bool)));
    connect(m_piDebugger, SIGNAL(targetConnectedChanged(bool)),
            this, SLOT(slotTargetConnectedChanged(bool)));

    addModule(new InfoModule(this));
    addModule(new PaintingModule(this));
    addModule(new WarningsModule(this));
    addModule(new BlueprintModule(this));
}

NokiaQtFramework::~NokiaQtFramework()
{
    InspectorPlugin::instance()->sharedDebugger()->releaseProbeInjectingDebugger();
    delete m_commServer;
}

LocalCommServer *NokiaQtFramework::commServer() const
{
    return m_commServer;
}

bool NokiaQtFramework::startInspection(const InspectionTarget &target)
{
    QString localServerName = m_model->localServerName();
    if (localServerName.isEmpty()) {
        qWarning("NokiaQtFramework::startInspection: local server is not listening, won't start the inspection");
        return false;
    }
    return m_piDebugger->setInspectionTarget(target, localServerName);
}

bool NokiaQtFramework::targetIsConnected() const
{
    return m_piDebugger->targetConnected();
}

int NokiaQtFramework::infoModuleUid() const
{
    return InfoModule::Uid;
}

void NokiaQtFramework::callProbeFunction(const QString &name, const QVariantList &args)
{
    m_piDebugger->callProbeFunction(name, args);
}

void NokiaQtFramework::slotTargetRunningChanged(bool running)
{
    Q_UNUSED(running);
}

void NokiaQtFramework::slotTargetConnectedChanged(bool connected)
{
    if (connected) {
        emit targetConnected();
    } else {
        setModuleActivationEnabled(false);
        emit targetDisconnected();
    }
}


//
// NokiaQtFrameworkFactory
//
QString NokiaQtFrameworkFactory::displayName() const
{
    return tr("Qt by Nokia");
}

QIcon NokiaQtFrameworkFactory::icon() const
{
    return QIcon(":/inspector/nokiaqtframework.png");
}

bool NokiaQtFrameworkFactory::isConfigurable() const
{
    return true;
}

void NokiaQtFrameworkFactory::configure()
{
    QMessageBox::information(0, tr("Configure Nokia Qt Framework"),
        tr("Configuration not implemented, please try again later."));
}

bool NokiaQtFrameworkFactory::available(const InspectionTarget &target) const
{
    // bad: other than StartLocalRunConfiguratio or AttachToPid
    if (target.type != InspectionTarget::StartLocalRunConfiguration &&
        target.type != InspectionTarget::AttachToPid)
        return false;

    // bad: StartLocalRunConfiguration with a null rc
    if (target.type == InspectionTarget::StartLocalRunConfiguration &&
        !target.runConfiguration)
        return false;

    // good: inspectingdebugger free
    return InspectorPlugin::instance()->sharedDebugger()->acquirable();
}

IFramework *NokiaQtFrameworkFactory::createFramework(const InspectionTarget &target)
{
    ProbeInjectingDebugger *debugger = InspectorPlugin::instance()->sharedDebugger()->acquireProbeInjectingDebugger();
    if (!debugger) {
        qWarning("NokiaQtFrameworkFactory::createFramework: can't acquire the ProbeInjectingDebugger");
        return 0;
    }

    NokiaQtInspectionModel *model = new NokiaQtInspectionModel;
    model->setTargetName(target.displayName);
    model->setFrameworkName(displayName());

    return new NokiaQtFramework(model, debugger);
}
