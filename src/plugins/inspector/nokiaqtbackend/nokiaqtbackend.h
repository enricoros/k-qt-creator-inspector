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

#ifndef NOKIAQTBACKEND_H
#define NOKIAQTBACKEND_H

#include "ibackend.h"
#include "ibackendtask.h"
#include <QtCore/QVariantList>

namespace Inspector {
namespace Internal {

class LocalCommServer;
class NokiaQtBackendFactory;
class NokiaQtInspectionModel;
class ProbeInjectingDebugger;


class NokiaQtBackend : public IBackend
{
    Q_OBJECT

    friend class NokiaQtBackendFactory;
    NokiaQtBackend(NokiaQtInspectionModel *, ProbeInjectingDebugger *, QObject *parent = 0);
public:
    ~NokiaQtBackend();

    // probe communication server
    LocalCommServer *commServer() const;

    // ::IBackend
    bool startInspection(const InspectionTarget &);
    bool isTargetConnected() const;
    int defaultModuleUid() const;

    // ### move this
    void callProbeFunction(const QString &name, const QVariantList &args);

private slots:
    void slotTargetConnectedChanged(bool connected);

private:
    LocalCommServer *m_commServer;
    ProbeInjectingDebugger *m_piDebugger;
    NokiaQtInspectionModel *m_model;
};


class NokiaQtBackendFactory : public IBackendFactory
{
    Q_OBJECT

public:
    // ::IBackendFactory
    QString displayName() const;
    QIcon icon() const;
    bool isConfigurable() const;
    void configure();

    bool available(const InspectionTarget &) const;
    IBackend *createBackend(const InspectionTarget &);
};


class NokiaQtSimpleCallTask : public IBackendTask
{
public:
    NokiaQtSimpleCallTask(const QString &displayName, const QString &functionName,
                          const QVariantList &functionArgs, NokiaQtBackend *);

    QString displayName() const;

protected:
    void activateTask();

private:
    NokiaQtBackend *m_backend;
    QString m_displayName;
    QString m_functionName;
    QVariantList m_functionArgs;
};

} // namespace Internal
} // namespace Inspector

#endif // NOKIAQTBACKEND_H
