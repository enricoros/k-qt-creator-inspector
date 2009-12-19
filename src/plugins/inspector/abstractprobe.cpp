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

#include "abstractprobe.h"

#include <QAbstractTransition>
#include <QStateMachine>
#include <QState>

using namespace Inspector::Internal;

/// Custom Events and Transition for the state machine
class TestEvent : public QEvent
{
public:
    enum OpType { Activate, Deactivate, Refuse, Wait };
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

struct Inspector::Internal::AbstractProbePrivate {
    QStateMachine stateMachine;
};

AbstractProbe::AbstractProbe(QObject *parent)
  : QObject(parent)
  , d(new AbstractProbePrivate)
{
    // 1. state machine configuration

    // 1.1 create states
    QState * sIdle = new QState;
    QState * sWait = new QState;
    connect(sWait, SIGNAL(entered()), this, SLOT(slotLock()));
    connect(sWait, SIGNAL(exited()), this, SLOT(slotUnlock()));
    QState * sActive = new QState;
    connect(sActive, SIGNAL(entered()), this, SLOT(slotActivate()));
    QState * sDeactivate = new QState;
    connect(sDeactivate, SIGNAL(entered()), this, SLOT(slotDeactivate()));

    // 1.2 configure transitions
    sIdle->addTransition(new TestTransition(TestEvent::Wait, sWait));
    sIdle->addTransition(new TestTransition(TestEvent::Activate, sActive));
    sWait->addTransition(new TestTransition(TestEvent::Refuse, sIdle));
    sWait->addTransition(new TestTransition(TestEvent::Activate, sActive));
    sActive->addTransition(new TestTransition(TestEvent::Deactivate, sDeactivate));
    sDeactivate->addTransition(this, SIGNAL(deactivated()), sIdle);

    // 1.3 add states to the machine
    d->stateMachine.addState(sIdle);
    d->stateMachine.addState(sWait);
    d->stateMachine.addState(sActive);
    d->stateMachine.addState(sDeactivate);
    d->stateMachine.setInitialState(sIdle);
}

AbstractProbe::~AbstractProbe()
{
    delete d;
}

void AbstractProbe::slotActivate()
{
}

void AbstractProbe::slotDeactivate()
{
    emit deactivated();
}

void AbstractProbe::slotLock()
{
}

void AbstractProbe::slotUnlock()
{
}

void AbstractProbe::controlActivate()
{
    d->stateMachine.postEvent(new TestEvent(TestEvent::Activate));
}

void AbstractProbe::controlDeactivate()
{
    d->stateMachine.postEvent(new TestEvent(TestEvent::Deactivate));
}

void AbstractProbe::controlRefuse()
{
    d->stateMachine.postEvent(new TestEvent(TestEvent::Refuse));
}

void AbstractProbe::controlWait()
{
    d->stateMachine.postEvent(new TestEvent(TestEvent::Wait));
}
