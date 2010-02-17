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

#ifndef IFRAMEWORKMODULE_H
#define IFRAMEWORKMODULE_H

#include <QObject>
#include <QIcon>
#include <QList>

namespace Inspector {
namespace Internal {

class AbstractPanel;
class IFramework;

/**
  \brief Describes the items to put in the menus by one module
*/
struct ModuleMenuEntry {
    QStringList path;
    QIcon icon;
    int moduleUid;
    int panelId;

    ModuleMenuEntry(const QStringList &path, int moduleUid, int panelId, const QIcon &icon = QIcon())
        : path(path), icon(icon), moduleUid(moduleUid), panelId(panelId) {}
};

typedef QList<ModuleMenuEntry> ModuleMenuEntries;

/**
  \brief Encapsulates functionalities relative to a probing context

  Handles a certain type of tests, encapsulates communication, database,
  panels and activation logic.
*/
class IFrameworkModule : public QObject
{
    Q_OBJECT

public:
    IFrameworkModule(IFramework *, QObject *parent = 0);
    virtual ~IFrameworkModule();

    // describe the module
    virtual int uid() const = 0;
    virtual QString name() const = 0;
    virtual ModuleMenuEntries menuEntries() const;
    virtual AbstractPanel *createPanel(int panelId);
    //virtual QList<int> cmdClasses() const = 0;
    //virtual * createCommSession(int cmdClass) = 0;

    //virtual void frameworkStarted();

    // useful references
    IFramework *parentFramework() const;

signals:
    // requests IFramework to activate this module
    void requestActivation(const QString &text);
    // tells IFramework that this module is idle again
    void deactivated();
    // requests display of a panesl
    void requestPanelDisplay(int panelId);

protected slots:
    // reimplement to be notified when the state changes
    virtual void slotActivate();
    virtual void slotDeactivate();
    virtual void slotLock();
    virtual void slotUnlock();

private:
    // used by IFramework for state transitions
    friend class IFramework;
    void controlActivate();
    void controlDeactivate();
    void controlRefuse();
    void controlWait();
    struct IFrameworkModulePrivate *d;
};

} // namespace Internal
} // namespace Inspector

#endif // IFRAMEWORKMODULE_H
