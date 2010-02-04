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

#include "blueprintmodule.h"
#include "abstractpanel.h"
#include "instance.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>

namespace Inspector {
namespace Internal {
  
class BlueprintPanel : public AbstractPanel {
public:
    BlueprintPanel(AbstractModule *parentModule)
      : AbstractPanel(parentModule)
    {    
        QPixmap pixmap(":/inspector/images/blueprint.png");
        QLabel *label = new QLabel;
        label->setFixedSize(pixmap.size());
        label->setPixmap(pixmap);

        QHBoxLayout *hLay = new QHBoxLayout(this);
        hLay->setMargin(0);
        hLay->addWidget(label);
    }
};

BlueprintModule::BlueprintModule(Instance *instance, QObject *parent)
  : AbstractModule(instance, parent)
{
}

QString BlueprintModule::name() const
{
    return tr("Blueprint (0.001)");
}

ModuleMenuEntries BlueprintModule::menuEntries() const
{
    ModuleMenuEntries entries;
    entries.append(ModuleMenuEntry(QStringList() << tr("Blueprint"), Uid, 1));
    return entries;
}

AbstractPanel *BlueprintModule::createPanel(int panelId)
{
    if (panelId != 1) {
        qWarning("BlueprintModule::createPanel: unknown panel %d", panelId);
        return 0;
    }
    return new BlueprintPanel(this);
}

} // namespace Internal
} // namespace Inspector
