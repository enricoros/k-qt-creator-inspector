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
   - model to store all the activations like "active tests", commands log, past tests, etc...
**/
class ProbeController : public QObject
{
    Q_OBJECT

public:
    ProbeController(QObject *parent = 0);
    ~ProbeController();

    void addTest(AbstractTest *);
    void removeTest(AbstractTest *);

    TestMenu mergedMenu() const;

private:
    void mergeToMenu(const TestMenu & menu);
    void unmergeFromMenu(const TestMenu & menu);
    QList<AbstractTest *> m_activeTests;
    QList<AbstractTest *> m_tests;
    TestMenu m_menu;

private slots:
    void slotTestActivationRequested();
    void slotTestDeactivated();
    void slotTestDestroyed();
};

} // namespace Internal
} // namespace Inspector

#endif // PROBECONTROLLER_H
