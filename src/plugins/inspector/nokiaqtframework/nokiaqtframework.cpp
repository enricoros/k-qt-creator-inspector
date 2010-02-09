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

#include "nokiaqtframework.h"
#include "localcommserver.h"
#include "blueprint/blueprintmodule.h"
#include "info/infomodule.h"
#include "painting/paintingmodule.h"
#include "warnings/warningsmodule.h"
#include <QtGui/QMessageBox>

using namespace Inspector::Internal;

//
// NokiaQtFramework
//
NokiaQtFramework::NokiaQtFramework(Instance *instance, QObject *parent)
  : IFramework(instance, parent)
{
    m_commServer = new LocalCommServer(instanceModel());

    addModule(new InfoModule(this));
    addModule(new PaintingModule(this));
    addModule(new WarningsModule(this));
    addModule(new BlueprintModule(this));
}

NokiaQtFramework::~NokiaQtFramework()
{
    delete m_commServer;
}

LocalCommServer *NokiaQtFramework::commServer() const
{
    return m_commServer;
}

void NokiaQtFramework::callProbeFunction(const QString &name, const QVariantList &args)
{
    qWarning("NokiaQtFramework::callProbeFunction: not implemented");
}

int NokiaQtFramework::infoModuleUid() const
{
    return InfoModule::Uid;
}

//
// NokiaQtFrameworkFactory
//
NokiaQtFrameworkFactory::NokiaQtFrameworkFactory()
{
}

QString NokiaQtFrameworkFactory::displayName() const
{
    return tr("Qt by Nokia");
}

QIcon NokiaQtFrameworkFactory::icon() const
{
    return QIcon(":/core/images/qtcreator_logo_32.png");
}

bool NokiaQtFrameworkFactory::isConfigurable() const
{
    return true;
}

void NokiaQtFrameworkFactory::configure()
{
    QMessageBox::information(0, tr("Configure Nokia Qt Framework"),
        tr("Configuration not implemented, please try again later."));
}
