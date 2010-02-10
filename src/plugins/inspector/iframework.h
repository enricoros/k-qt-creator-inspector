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

#ifndef IFRAMEWORK_H
#define IFRAMEWORK_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include "iframeworkmodule.h"
class QStandardItem;

namespace ProjectExplorer {
class RunConfiguration;
}

namespace Inspector {
namespace Internal {

class Instance;
class InstanceModel;

/**
  \brief Handles everything within a framework
*/
class IFramework : public QObject
{
    Q_OBJECT

public:
    IFramework(Instance *instance, QObject *parent = 0);
    virtual ~IFramework();

    // operate on modules
    ModuleMenuEntries menuEntries() const;
    QStringList moduleNames() const;
    AbstractPanel *createPanel(int moduleUid, int panelId) const;

    // to be reimplemented by subclasses
    virtual int infoModuleUid() const = 0;
    virtual bool startRunConfiguration(ProjectExplorer::RunConfiguration *) { return false; }

    // ### is it ok to put this here?
    InstanceModel *instanceModel() const;

signals:
    void modulesChanged();
    void requestPanelDisplay(int moduleUid, int panelId);

protected:
    void addModule(IFrameworkModule *);
    void removeModule(IFrameworkModule *);
    IFrameworkModule * moduleForUid(int moduleUid) const;

    Instance *m_instance;
    QList<IFrameworkModule *> m_modules;
    QList<IFrameworkModule *> m_activeModules;

private slots:
    void slotModulePanelDisplayRequested(int panelId);
    void slotModuleActivationRequested(const QString &text);
    void slotModuleDeactivated();
    void slotModuleDestroyed();
    void slotModelItemChanged(QStandardItem*);
};


/**
  \brief Describe, create (eventually restore) frameworks
 */
class IFrameworkFactory : public QObject
{
    Q_OBJECT

public:
    IFrameworkFactory()
    { }
    virtual ~IFrameworkFactory()
    { }

    virtual QString displayName() const = 0;
    virtual QIcon icon() const = 0;
    virtual bool isConfigurable() const { return false; }

    virtual bool available() const { return true; }
    virtual IFramework *createFramework(Instance *) = 0;

public slots:
    virtual void configure() { }
};

} // namespace Internal
} // namespace Inspector

#endif // IFRAMEWORK_H
