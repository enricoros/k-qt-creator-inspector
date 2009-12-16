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

#include "painttest.h"

#include "ptview.h"

#include <QAbstractTransition>
#include <QStateMachine>
#include <QState>

using namespace Performance::Internal;


PaintTest::PaintTest(QObject *parent)
  : AbstractTest(parent)
{
}

PaintTest::~PaintTest()
{
    qDeleteAll(m_views);
}

QString PaintTest::name() const
{
    return tr("Paint Temperature Test");
}

TestMenu PaintTest::menu() const
{
    TestMenu menu;
    TestMenuItem root(tr("Painting"), true, Uid, 0);
    root.children.append(TestMenuItem(tr("Temperature"), true, Uid, 1));
    root.children.append(TestMenuItem(tr("Pixel Energy"), true, Uid, 2));
    menu.append(root);
    return menu;
}

QWidget * PaintTest::createView(int viewId)
{
    Q_UNUSED(viewId);
    PaintTemperatureView * ptView = new PaintTemperatureView;
    return ptView;
}

void PaintTest::slotActivate()
{
}

void PaintTest::slotDeactivate()
{
    emit deactivated();
}

void PaintTest::slotLock()
{
    foreach (PaintTemperatureView * view, m_views)
        view->setEnabled(false);
}

void PaintTest::slotUnlock()
{
    foreach (PaintTemperatureView * view, m_views)
        view->setEnabled(true);
}
