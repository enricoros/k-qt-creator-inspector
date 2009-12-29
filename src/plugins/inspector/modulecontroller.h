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

#ifndef MODULECONTROLLER_H
#define MODULECONTROLLER_H

#include <QObject>
#include <QList>
#include "abstractmodule.h"

namespace Inspector {

class Instance;

namespace Internal {

/**
  Features to add:
   - model to store all the activations like "active modules", commands log, past modules, etc...
**/
class ModuleController : public QObject
{
    Q_OBJECT

public:
    ModuleController(Inspector::Instance *instance);
    ~ModuleController();

    void addModule(AbstractModule *);
    void removeModule(AbstractModule *);

    // operate on modules
    ModuleMenuEntries menuEntries() const;
    QStringList moduleNames() const;
    AbstractView *createView(int moduleUid, int viewId) const;

signals:
    void modulesChanged();

private:
    AbstractModule * moduleForUid(int moduleUid) const;
    Inspector::Instance *m_instance;
    QList<AbstractModule *> m_modules;
    QList<AbstractModule *> m_activeModules;

private slots:
    void slotModuleActivationRequested();
    void slotModuleDeactivated();
    void slotModuleDestroyed();
};

} // namespace Internal
} // namespace Inspector

#endif // MODULECONTROLLER_H
