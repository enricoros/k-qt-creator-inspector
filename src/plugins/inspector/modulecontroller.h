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
class QStandardItem;

namespace Inspector {
namespace Internal {

class TasksModel;

/**
  Features to add:
   - model to store all the activations like "active modules", commands log, past modules, etc...
**/
class ModuleController : public QObject
{
    Q_OBJECT

public:
    ModuleController(TasksModel *, QObject *parent = 0);
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
    TasksModel *m_tasksModel;
    QList<AbstractModule *> m_modules;
    QList<AbstractModule *> m_activeModules;

private slots:
    void slotModuleActivationRequested(const QString &text);
    void slotModuleDeactivated();
    void slotModuleDestroyed();
    void slotModelItemChanged(QStandardItem*);
};

} // namespace Internal
} // namespace Inspector

#endif // MODULECONTROLLER_H
