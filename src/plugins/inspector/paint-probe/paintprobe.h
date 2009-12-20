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

#ifndef PAINTPROBE_H
#define PAINTPROBE_H

#include "abstractprobe.h"

namespace Inspector {
namespace Internal {

class PaintProbe : public AbstractProbe
{
    Q_OBJECT

public:
    PaintProbe(QObject *parent = 0);
    ~PaintProbe();

    // ::AbstractProbe
    enum { Uid = 0x01 };
    int uid() const { return Uid; }
    QString name() const;
    ProbeMenuEntries menuEntries() const;
    AbstractView *createView(int viewId);
    void slotActivate();
    void slotDeactivate();
    void slotLock();
    void slotUnlock();

private:
    QList<AbstractView *> m_views;

private slots:
    void slotViewDestroyed();
};

} // namespace Internal
} // namespace Inspector

#endif // PAINTPROBE_H
