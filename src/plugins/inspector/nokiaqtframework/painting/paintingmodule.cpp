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

#include "paintingmodule.h"
#include "paintingpanel.h"
#include "frequencypanel.h"
#include "frequencytask.h"
#include "thermalmodel.h"
#include "thermalpanel.h"
#include "thermaltask.h"
#include "../nokiaqtframework.h"

using namespace Inspector::Internal;

//
// PaintingModule
//
PaintingModule::PaintingModule(NokiaQtFramework *framework, QObject *parent)
  : IFrameworkModule(framework, parent)
  , m_framework(framework)
  , m_showExposedAreas(false) // mirror initial state in perfunction.cpp
{
    m_thermalModel = new ThermalModel;
}

PaintingModule::~PaintingModule()
{
    delete m_thermalModel;
}

ThermalModel *PaintingModule::thermalModel() const
{
    return m_thermalModel;
}

QString PaintingModule::name() const
{
    return tr("Painting Measures (0.6)");
}

ModuleMenuEntries PaintingModule::menuEntries() const
{
    ModuleMenuEntries entries;
    entries.append(ModuleMenuEntry(QStringList() << tr("Painting") << tr("Thermal Analysis"), UID_MODULE_PAINTING, 1, QIcon(":/inspector/painting/menu-thermal.png")));
    entries.append(ModuleMenuEntry(QStringList() << tr("Painting") << tr("Frequency Analysis"), UID_MODULE_PAINTING, 2));
    entries.append(ModuleMenuEntry(QStringList() << tr("Painting") << tr("Debugging Tools"), UID_MODULE_PAINTING, 3));
    return entries;
}

AbstractPanel *PaintingModule::createPanel(int panelId)
{
    if (panelId == 1)
        return new ThermalPanel(this);
    else if (panelId == 2)
        return new FrequencyPanel(this);
    else if (panelId == 3)
        return new PaintingPanel(this);
    qWarning("PaintingModule::createPanel: unknown panel %d", panelId);
    return 0;
}

void PaintingModule::startFrequencyTest()
{
    new FrequencyTask(m_framework);
}

void PaintingModule::startThermalTest(const QString &title, const QVariantList &options)
{
    new ThermalTask(m_framework, m_thermalModel, options, title);
}

void PaintingModule::setShowExposedAreas(bool show)
{
    if (show != m_showExposedAreas) {
        m_showExposedAreas = show;
        QString name = m_showExposedAreas ? tr("Enabling Debug") : tr("Disabling Debug");
        new NokiaQtSimpleCallTask(name, "qPaintingShowExposedAreas",
                                  QVariantList() << (bool)m_showExposedAreas, m_framework);
    }
}

void PaintingModule::setGuiStyle(const QString &styleName)
{
    QString name = tr("Setting style %1").arg(styleName);
    new NokiaQtSimpleCallTask(name, "qPaintingSetGuiStyle",
                              QVariantList() << styleName, m_framework);
}
