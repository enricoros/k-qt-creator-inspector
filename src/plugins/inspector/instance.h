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

#ifndef INSPECTORINSTANCE_H
#define INSPECTORINSTANCE_H

#include <QObject>
#include <QList>
#include <QVariantList>
#include "instancemodel.h"
#include "tasksmodel.h"

namespace Inspector {


namespace Internal {

class CommServer;
class ModuleController;

}

class Q_DECL_EXPORT Instance : public QObject
{
    Q_OBJECT

public:
    Instance(QObject *parent = 0);
    ~Instance();

    // data models
    InstanceModel *instanceModel() const;
    Internal::TasksModel *tasksModel() const;

    // probe communication server
    Internal::CommServer *commServer() const;

    // controls loaded modules
    Internal::ModuleController *moduleController() const;

    void makeVisible(int moduleUid, int viewId);

signals:
    void requestDisplay(int moduleUid, int viewId);

private:
    InstanceModel *m_instanceModel;
    Internal::TasksModel *m_tasksModel;
    Internal::CommServer *m_commServer;
    Internal::ModuleController *m_moduleController;
};

} // namespace Inspector

#endif // INSPECTORINSTANCE_H
