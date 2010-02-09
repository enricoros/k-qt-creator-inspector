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

#ifndef PAINTINGMODULE_H
#define PAINTINGMODULE_H

#include "iframeworkmodule.h"

namespace Inspector {
namespace Internal {

class NokiaQtFramework;
class PaintingModel;

class PaintingModule : public IFrameworkModule
{
    Q_OBJECT

public:
    PaintingModule(NokiaQtFramework *, QObject *parent = 0);
    ~PaintingModule();

    PaintingModel *model() const;

    // ::IFrameworkModule
    enum { Uid = 0x02 };
    int uid() const { return Uid; }
    QString name() const;
    ModuleMenuEntries menuEntries() const;
    AbstractPanel *createPanel(int panelId);

    void slotActivate();
    void slotDeactivate();
    void slotLock();
    void slotUnlock();

private:
    PaintingModel *m_model;
    QList<AbstractPanel *> m_panels;

private slots:
    void slotProcessIncomingData(quint32 channel, quint32 code1, QByteArray *data);
    void slotPanelDestroyed();
};

} // namespace Internal
} // namespace Inspector

#endif // PAINTINGMODULE_H
