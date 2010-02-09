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

#include "instance.h"
#include "iframework.h"

using namespace Inspector::Internal;

Instance::Instance(const QString &targetName, IFrameworkFactory *factory, QObject *parent)
  : QObject(parent)
{
    m_instanceModel = new InstanceModel(targetName, factory->displayName());
    m_tasksModel = new TasksModel;
    m_framework = factory->createFramework(this);
}

Instance::~Instance()
{
    delete m_framework;
    delete m_tasksModel;
    delete m_instanceModel;
}

InstanceModel *Instance::instanceModel() const
{
    return m_instanceModel;
}

TasksModel *Instance::tasksModel() const
{
    return m_tasksModel;
}

IFramework *Instance::framework() const
{
    return m_framework;
}
