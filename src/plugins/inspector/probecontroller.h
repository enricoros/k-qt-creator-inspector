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

#ifndef PROBECONTROLLER_H
#define PROBECONTROLLER_H

#include <QObject>
#include <QList>
#include "abstractprobe.h"

namespace Inspector {
namespace Internal {

/**
  Features to add:
   - model to store all the activations like "active probes", commands log, past probes, etc...
**/
class ProbeController : public QObject
{
    Q_OBJECT

public:
    ProbeController(QObject *parent = 0);
    ~ProbeController();

    void addProbe(AbstractProbe *);
    void removeProbe(AbstractProbe *);

    // operate on probes
    ProbeMenuEntries menuEntries() const;
    QStringList probeNames() const;
    AbstractView *createView(int probeUid, int viewId) const;

    // TEMP
    void activatePTProbe();

signals:
    void probesChanged();

private:
    AbstractProbe * probeForUid(int probeId) const;
    QList<AbstractProbe *> m_probes;
    QList<AbstractProbe *> m_activeProbes;

private slots:
    void slotProbeActivationRequested();
    void slotProbeDeactivated();
    void slotProbeDestroyed();
};

} // namespace Internal
} // namespace Inspector

#endif // PROBECONTROLLER_H
