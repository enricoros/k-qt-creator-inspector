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

#include <extensionsystem/iplugin.h>

class QAction;

namespace Inspector {

class InspectorInstance;

/// main accessor when using this plugin from the outside. we suppose a single debuggee for now.
Q_DECL_EXPORT InspectorInstance * defaultInstance();

namespace Internal {

class InspectorFrame;

// constants
const char * const MODE_INSPECTOR   = "Probe";
const int          P_MODE_INSPECTOR = 5;

/**
    \brief QtCreator plugin that exposes a framework for runtime probing
*/
class InspectorPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT

public:
    InspectorPlugin();
    ~InspectorPlugin();

    // single debuggee assumption lies here: we use a single instance from the outside
    static Inspector::InspectorInstance * defaultInstance();

    // ::ExtensionSystem::IPlugin
    bool initialize(const QStringList &arguments, QString *error_message);
    void extensionsInitialized();

private slots:
    void slotDisplayInstance();
    void slotDebugPaintToggled(bool checked);
    void slotTempPaintingTemperature();
    void slotSetEnabled(bool enabled);

private:
    void parseArguments(const QStringList & arguments);
    static InspectorPlugin *s_pluginInstance;
    QList<InspectorInstance *> m_instances;
    Internal::InspectorFrame *m_window;
    bool m_pluginEnabled;
};

} // namespace Internal
} // namespace Inspector

#endif // INSPECTORPLUGIN_H
