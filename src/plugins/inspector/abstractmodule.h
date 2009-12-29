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

#ifndef ABSTRACTMODULE_H
#define ABSTRACTMODULE_H

#include <QObject>
#include <QIcon>
#include <QList>

namespace Inspector {
namespace Internal {
class AbstractView;
class ModuleController;

struct ModuleMenuEntry {
    QStringList path;
    QIcon icon;
    int moduleUid;
    int viewId;

    ModuleMenuEntry(const QStringList &path, int moduleUid, int viewId, const QIcon &icon = QIcon())
        : path(path), icon(icon), moduleUid(moduleUid), viewId(viewId) {}
};

typedef QList<ModuleMenuEntry> ModuleMenuEntries;

class AbstractModule : public QObject
{
    Q_OBJECT

public:
    AbstractModule(QObject *parent = 0);
    virtual ~AbstractModule();

    // describe the module
    virtual int uid() const = 0;
    virtual QString name() const = 0;
    virtual ModuleMenuEntries menuEntries() const = 0;
    //virtual QList<int> cmdClasses() const = 0;
    //virtual * createCommSession(int cmdClass) = 0;
    virtual AbstractView * createView(int viewId) = 0;

signals:
    // requests ModuleContoller to activate this module
    void requestActivation();
    // tells ModuleController that this module is idle again
    void deactivated();

protected slots:
    // reimplement to be notified when the state changes
    virtual void slotActivate();
    virtual void slotDeactivate();
    virtual void slotLock();
    virtual void slotUnlock();

private:
    // used by ModuleController for state transitions
    friend class ModuleController;
    void controlActivate();
    void controlDeactivate();
    void controlRefuse();
    void controlWait();
    struct AbstractModulePrivate * d;
};

} // namespace Internal
} // namespace Inspector

#endif // ABSTRACTMODULE_H
