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

#ifndef INSPECTORPLUGIN_H
#define INSPECTORPLUGIN_H

#include <QtCore/QList>
#include <extensionsystem/iplugin.h>
#include <coreplugin/icorelistener.h>

namespace Inspector {
namespace Internal {

class InspectorContainer;
class Inspection;
class SharedDebugger;

// constants
const char * const MODE_INSPECTOR       = "Inspect";
const int          P_MODE_INSPECTOR     = 5;

// contexts
const char * const C_INSPECTOR          = "Inspector";
const char * const C_INSPECTOR_RUNNING  = "Inspector.Running";

/**
    \brief QtCreator plugin for Runtime Inspection
    See "Intuitive Modern Software Metrics: design and implementation" by Enrico Ros
*/
class InspectorPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT

public:
    InspectorPlugin();
    ~InspectorPlugin();

    static InspectorPlugin *instance();

    SharedDebugger *sharedDebugger() const;
    QWidget *containerWindow() const;

    QList<Inspection *> inspections() const;
    void addInspection(Inspection *);
    void deleteInspection(Inspection *);
    void closeInspections();

    // ::ExtensionSystem::IPlugin
    bool initialize(const QStringList &arguments, QString *error_message);
    void extensionsInitialized();

signals:
    void inspectionAdded(Inspection *);
    void inspectionRemoved(Inspection *);

private slots:
    void slotSetPluginEnabled(bool enabled);
    void slotDisplayWindow();

private:
    void parseArguments(const QStringList & arguments);
    static InspectorPlugin *s_pluginInstance;
    SharedDebugger *m_sharedDebugger;
    InspectorContainer *m_container;
    QList<Inspection *> m_inspections;
    int m_runningContextId;
};

/**
  \brief Filters application close events
*/
class InspectorCoreListener : public Core::ICoreListener
{
    Q_OBJECT
public:
    explicit InspectorCoreListener(QObject *parent = 0);
    bool coreAboutToClose();
};

} // namespace Internal
} // namespace Inspector

#endif // INSPECTORPLUGIN_H
