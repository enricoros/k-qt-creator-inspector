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

#include "abstracttest.h"

#include <QSignalTransition>
#include <QState>

using namespace Performance::Internal;

/// Custom Events and Transition for the state machine
class TestEvent : public QEvent
{
public:
    enum OpType { Go, Wait, Deny, Close };
    TestEvent(OpType opType) : QEvent(QEvent::Type(QEvent::User+1)), opType(opType) {}
    OpType opType;
};

class TestTransition : public QAbstractTransition
{
public:
    TestTransition(TestEvent::OpType opType, QAbstractState * target) : m_opType(opType) { setTargetState(target); }

protected:
    virtual bool eventTest(QEvent *e)
    {
        if (e->type() != QEvent::Type(QEvent::User+1))
            return false;
        return m_opType == static_cast<TestEvent*>(e)->opType;
    }
    virtual void onTransition(QEvent *) {}

private:
    TestEvent::OpType m_opType;
};


AbstractTest::AbstractTest(QObject *parent)
  : QObject(parent)
{
    // 1. state machine configuration

    // 1.1 create states
    QState * sIdle = new QState;
    QState * sWait = new QState;
    connect(sWait, SIGNAL(entered()), this, SLOT(slotWaitEntered()));
    connect(sWait, SIGNAL(exited()), this, SLOT(slotWaitExited()));
    QState * sActive = new QState;
    connect(sActive, SIGNAL(entered()), this, SLOT(slotActivate()));
    QState * sClosing = new QState;
    connect(sClosing, SIGNAL(entered()), this, SLOT(slotClose()));

    // 1.2 configure transitions
    sIdle->addTransition(new TestTransition(TestEvent::Wait, sWait));
    sIdle->addTransition(new TestTransition(TestEvent::Go, sActive));
    sWait->addTransition(new TestTransition(TestEvent::Deny, sIdle));
    sWait->addTransition(new TestTransition(TestEvent::Go, sActive));
    sActive->addTransition(new TestTransition(TestEvent::Close, sClosing));
    sClosing->addTransition(new QSignalTransition(this, SIGNAL(deactivated())));

    // 1.3 add states to the machine
    m_stateMachine.addState(sIdle);
    m_stateMachine.addState(sWait);
    m_stateMachine.addState(sActive);
    m_stateMachine.addState(sClosing);
    m_stateMachine.setInitialState(sIdle);
}

void AbstractTest::slotWaitEntered()
{
}

void AbstractTest::slotWaitExited()
{
}

void AbstractTest::slotActivate()
{
}

void AbstractTest::slotClose()
{
    emit deactivated();
}

void AbstractTest::controlGo()
{
    m_stateMachine.postEvent(new TestEvent(TestEvent::Go));
}

void AbstractTest::controlWait()
{
    m_stateMachine.postEvent(new TestEvent(TestEvent::Wait));
}

void AbstractTest::controlDenied()
{
    m_stateMachine.postEvent(new TestEvent(TestEvent::Deny));
}

void AbstractTest::controlClose()
{
    m_stateMachine.postEvent(new TestEvent(TestEvent::Close));
}
