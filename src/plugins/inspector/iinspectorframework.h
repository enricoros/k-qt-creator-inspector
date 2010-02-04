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

#ifndef IINSPECTORFRAMEWORK_H
#define IINSPECTORFRAMEWORK_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include "iframeworkmodule.h"
class QStandardItem;

namespace Inspector {
namespace Internal {

/**
  \brief Handles everything within a framework
*/
class IInspectorFramework : public QObject
{
    Q_OBJECT

public:
    IInspectorFramework(Instance *instance, QObject *parent = 0);
    virtual ~IInspectorFramework();

    // operate on modules
    ModuleMenuEntries menuEntries() const;
    QStringList moduleNames() const;
    AbstractPanel *createPanel(int moduleUid, int panelId) const;

    // to be reimplemented by subclasses
    virtual int infoModuleUid() const = 0;

signals:
    void modulesChanged();

protected:
    void addModule(IFrameworkModule *);
    void removeModule(IFrameworkModule *);
    IFrameworkModule * moduleForUid(int moduleUid) const;

    Instance *m_instance;
    QList<IFrameworkModule *> m_modules;
    QList<IFrameworkModule *> m_activeModules;

private slots:
    void slotModuleActivationRequested(const QString &text);
    void slotModuleDeactivated();
    void slotModuleDestroyed();
    void slotModelItemChanged(QStandardItem*);
};


/**
  \brief Describe, create (eventually restore) frameworks
 */
class IInspectorFrameworkFactory : public QObject
{
    Q_OBJECT

public:
    IInspectorFrameworkFactory()
    { }
    virtual ~IInspectorFrameworkFactory()
    { }

    //IInspectorFramework *create(Instance *instance) const = 0;

    virtual QString displayName() const = 0;
    virtual QIcon icon() const = 0;
    virtual bool isConfigurable() const { return false; }

public slots:
    virtual void configure() { }
};

} // namespace Internal
} // namespace Inspector

#endif // IINSPECTORFRAMEWORK_H