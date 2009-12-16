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

#include "testcontrol.h"

#include "abstracttest.h"

using namespace Performance::Internal;

TestControl::TestControl(QObject *parent)
  : QObject(parent)
{
}

TestControl::~TestControl()
{
    // delete all the tests (bypassing the 'destroyed' hook)
    QList<AbstractTest *> listCopy = m_tests;
    m_tests.clear();
    qDeleteAll(listCopy);
}

void TestControl::addTest(AbstractTest * test)
{
    if (!test) {
        qWarning("TestControl::addTest: skipping 0 pointer");
        return;
    }
    // register the Test
    connect(test, SIGNAL(requestActivation()), this, SLOT(slotTestActivationRequested()));
    connect(test, SIGNAL(deactivated()), this, SLOT(slotTestDeactivated()));
    connect(test, SIGNAL(destroyed()), this, SLOT(slotTestDestroyed()));
    mergeToMenu(test->menu());
    m_tests.append(test);
}

void TestControl::removeTest(AbstractTest * test)
{
    if (!test) {
        qWarning("TestControl::removeTest: skipping 0 pointer");
        return;
    }
    // unregister the Test
    unmergeFromMenu(test->menu());
    disconnect(test, 0, this, 0);
    m_tests.removeAll(test);
    m_activeTests.removeAll(test);
}

TestMenu TestControl::mergedMenu() const
{
    return m_menu;
}

void TestControl::mergeToMenu(const TestMenu & menu)
{
    qWarning("TestControl::mergeToMenu: FIXME");
    m_menu.append(menu);
}

void TestControl::unmergeFromMenu(const TestMenu & /*menu*/)
{
    qWarning("TestControl::unmergeFromMenu: TODO");
}

void TestControl::slotTestActivationRequested()
{
    AbstractTest * test = static_cast<AbstractTest *>(sender());
    test->controlActivate();
    m_activeTests.append(test);
}

void TestControl::slotTestDeactivated()
{
    AbstractTest * test = static_cast<AbstractTest *>(sender());
    m_activeTests.removeAll(test);
}

void TestControl::slotTestDestroyed()
{
    AbstractTest * test = static_cast<AbstractTest *>(sender());
    if (m_tests.contains(test))
        removeTest(test);
}
