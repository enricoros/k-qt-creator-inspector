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

#ifndef IBACKEND_H
#define IBACKEND_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include "ibackendmodule.h"
#include "inspectiontarget.h"
class QStandardItem;

namespace Inspector {
namespace Internal {

class IInspectionModel;
class IBackendTask;
class TasksModel;

/**
  \brief Handles everything within a backend (Qt, gtk, etc..)
*/
class IBackend : public QObject
{
    Q_OBJECT

public:
    IBackend(IInspectionModel *, QObject *parent = 0);
    virtual ~IBackend();

    IInspectionModel *inspectionModel() const;
    TasksModel *tasksModel() const;

    // IBackend description
    virtual bool startInspection(const InspectionTarget &) = 0;
    virtual bool isTargetConnected() const = 0;
    virtual int defaultModuleUid() const = 0;

    // IBackendModule operations
    QStringList loadedModules() const;
    ModuleMenuEntries menuItems() const;
    AbstractPanel *createPanel(int moduleUid, int panelId) const;

signals:
    void targetConnected(bool connected);
    void requestPanelDisplay(int moduleUid, int panelId);

private:
    IInspectionModel *m_inspectionModel;

// Modules
signals:
    void moduleListChanged();

protected:
    void addModule(IBackendModule *);
    void takeModule(IBackendModule *);
    IBackendModule *moduleForUid(int moduleUid) const;

private slots:
    void slotModulePanelDisplayRequested(int panelId);
    void slotModuleDestroyed();

private:
    QList<IBackendModule *> m_modules;

// Tasks
protected:
    friend class IBackendTask;
    void registerTask(IBackendTask *);
    void unregisterTask(IBackendTask *);
    void setTaskActivationEnabled(bool);

private slots:
    void slotTaskActivationRequested();
    void slotTaskDeletionRequested();
    void slotTaskSetProgress(int percent);
    void slotTaskModelItemChanged(QStandardItem*);

private:
    TasksModel *m_tasksModel;
    QList<IBackendTask *> m_tasks;
    bool m_taskActivationEnabled;
};


/**
  \brief Describes, creates (and eventually restore) Backends
 */
class IBackendFactory : public QObject
{
    Q_OBJECT

public:
    IBackendFactory() { }
    virtual ~IBackendFactory() { }

    virtual QString displayName() const = 0;
    virtual QIcon icon() const = 0;
    virtual bool isConfigurable() const { return false; }

    virtual bool available(const InspectionTarget &) const { return true; }
    virtual IBackend *createBackend(const InspectionTarget &) = 0;

public slots:
    virtual void configure() { }
};

} // namespace Internal
} // namespace Inspector

#endif // IBACKEND_H
