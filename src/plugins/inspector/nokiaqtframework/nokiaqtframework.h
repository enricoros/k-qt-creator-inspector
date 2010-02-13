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

#ifndef NOKIAQTFRAMEWORK_H
#define NOKIAQTFRAMEWORK_H

#include "iframework.h"
#include <QtCore/QVariantList>

namespace Inspector {
namespace Internal {

class LocalCommServer;
class NokiaQtFrameworkFactory;
class NokiaQtInspectionModel;
class SharedDebugger;


class NokiaQtFramework : public IFramework
{
    Q_OBJECT

public:
    ~NokiaQtFramework();

    // probe communication server
    LocalCommServer *commServer() const;

    // ::IFramework
    bool startAttachToPid(quint64 pid);
    bool startRunConfiguration(ProjectExplorer::RunConfiguration *rc);
    int infoModuleUid() const;

    // ### move this
    void callProbeFunction(const QString &name, const QVariantList &args);

private:
    friend class NokiaQtFrameworkFactory;
    NokiaQtFramework(NokiaQtInspectionModel *, SharedDebugger *, QObject *parent = 0);
    LocalCommServer *m_commServer;
    SharedDebugger *m_sharedDebugger;
    NokiaQtInspectionModel *m_model;
};


class NokiaQtFrameworkFactory : public IFrameworkFactory
{
    Q_OBJECT

public:
    // ::IFrameworkFactory
    QString displayName() const;
    QIcon icon() const;
    bool isConfigurable() const;
    void configure();
    bool available() const;
    IFramework *createFramework();
};

} // namespace Internal
} // namespace Inspector

#endif // NOKIAQTFRAMEWORK_H
