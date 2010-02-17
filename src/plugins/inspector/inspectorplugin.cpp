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

#include "inspectorplugin.h"
#include "inspectorcontainer.h"
#include "inspection.h"
#include "shareddebugger.h"
#include "nokiaqtframework/nokiaqtframework.h"
#include "nvidiacudaframework/nvidiacudaframework.h"
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/basemode.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <debugger/debuggerconstants.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <QtCore/QDebug>
#include <QtCore/QtPlugin>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

using namespace Inspector::Internal;

InspectorPlugin *Inspector::Internal::InspectorPlugin::s_pluginInstance = 0;

InspectorPlugin::InspectorPlugin()
  : ExtensionSystem::IPlugin()
  , m_sharedDebugger(0)
  , m_container(0)
{
    // reference the plugin (for static accessors)
    s_pluginInstance = this;
}

InspectorPlugin::~InspectorPlugin()
{
    // delete inspections
    while (!m_inspections.isEmpty())
        deleteInspection(m_inspections.last());

    // goodbye plugin
    s_pluginInstance = 0;

    // delete the debugger
    delete m_sharedDebugger;

    // objects registered with 'addAutoReleasedObject' will be removed automatically, like:
    // m_window is deleted by the plugin system
    m_container = 0;
}

InspectorPlugin *InspectorPlugin::instance()
{
    return s_pluginInstance;
}

SharedDebugger *InspectorPlugin::sharedDebugger()
{
    return m_sharedDebugger;
}

QList<Inspection *> InspectorPlugin::inspections() const
{
    return m_inspections;
}

void InspectorPlugin::addInspection(Inspection * inspection)
{
    if (m_inspections.contains(inspection)) {
        qWarning("InspectorPlugin::addInspection: inspection already present");
        return;
    }

    m_inspections.append(inspection);
    emit inspectionAdded(inspection);
}

void InspectorPlugin::deleteInspection(Inspection * inspection)
{
    if (!m_inspections.contains(inspection)) {
        qWarning("InspectorPlugin::deleteInspection: inspection is not present");
        return;
    }

    m_inspections.removeAll(inspection);
    emit inspectionRemoved(inspection);
    inspection->deleteLater();
}

bool InspectorPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(error_message)

    // check arguments
    parseArguments(arguments);

    // get core objects
    Core::ICore *core = Core::ICore::instance();
    QList<int> ourContext;
    ourContext << Core::Constants::C_GLOBAL_ID;
    ourContext << core->uniqueIDManager()->uniqueIdentifier(Debugger::Constants::GDBRUNNING);

    addAutoReleasedObject(new NokiaQtFrameworkFactory());
    addAutoReleasedObject(new NvidiaCudaFrameworkFactory());

    m_sharedDebugger = new SharedDebugger;

    m_container = new InspectorContainer;
    connect(m_container, SIGNAL(requestWindowDisplay()),
            this, SLOT(slotDisplayWindow()));

    // create the Mode, that registers the widget too
    Core::BaseMode * inspectorMode = new Core::BaseMode;
    inspectorMode->setDisplayName(tr("Inspect"));
    inspectorMode->setId(QLatin1String(Inspector::Internal::MODE_INSPECTOR));
    inspectorMode->setIcon(QIcon(":/inspector/images/inspector-icon-32.png"));
    inspectorMode->setPriority(Inspector::Internal::P_MODE_INSPECTOR);
    inspectorMode->setWidget(m_container);
    inspectorMode->setContext(ourContext);
    addAutoReleasedObject(inspectorMode);

    // create the Menu and add it to the Debug menu
    Core::ActionManager *am = core->actionManager();

    QAction *showAction = new QAction(tr("Show Inspector"), this);
    connect(showAction, SIGNAL(triggered()), this, SLOT(slotDisplayWindow()));
    am->registerAction(showAction, "Inspector.ShowInspection", ourContext);

    return true;
}

void InspectorPlugin::extensionsInitialized()
{
}

void InspectorPlugin::slotDisplayWindow()
{
    // switch creator to the Inspector Mode
    Core::ICore::instance()->modeManager()->activateMode(Inspector::Internal::MODE_INSPECTOR);
}

void InspectorPlugin::parseArguments(const QStringList &arguments)
{
    Q_UNUSED(arguments);
}

Q_EXPORT_PLUGIN(InspectorPlugin)
