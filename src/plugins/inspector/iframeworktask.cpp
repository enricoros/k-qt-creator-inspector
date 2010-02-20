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

#include "iframeworktask.h"
#include "iframework.h"

#include <QtCore/QAbstractTransition>
#include <QtCore/QStateMachine>
#include <QtCore/QState>

using namespace Inspector::Internal;

namespace Inspector {
namespace Internal {

//
// Custom Events and Transition test for the StateMachine
//
class TestEvent : public QEvent
{
public:
    enum OpType { Activate, Deactivate, Refuse, Wait };
    TestEvent(OpType opType)
      : QEvent(QEvent::Type(QEvent::User + 1))
      , opType(opType)
    {
    }
    OpType opType;
};

class TestTransition : public QAbstractTransition
{
public:
    TestTransition(TestEvent::OpType opType, QAbstractState * target)
      : m_opType(opType)
    {
        setTargetState(target);
    }

protected:
    virtual bool eventTest(QEvent *e)
    {
        if (e->type() != QEvent::Type(QEvent::User + 1))
            return false;
        return m_opType == static_cast<TestEvent*>(e)->opType;
    }
    virtual void onTransition(QEvent *)
    {
    }

private:
    TestEvent::OpType m_opType;
};

//
// private structure
//
class IFrameworkTaskPrivate
{
public:
    IFramework *framework;
    QStateMachine stateMachine;
    quint32 taskId;
};

} // namespace Internal
} // namespace Inspector

/**
  Note about software's workflow:
    new WarningsTask()
     1. ::IFrameworkTask()
      * creates ans starts Task StateMachine
      * registers the task in the Framework
        * queues connection of task's requestActivation and finished
     2. emits requestActivation

    IFramework::slotTaskActivationRequested()
      * updates TaskModel
      * upadtes task state (likely an Activation)

    WarningsTask::slotActivate()
     slotActivate gets called
*/

//
// IFrameworkTask
//
IFrameworkTask::IFrameworkTask(IFramework *framework, QObject *parent)
  : QObject(parent)
  , d(new IFrameworkTaskPrivate)
{
    static quint32 s_baseTaskId = 1;
    d->framework = framework;
    d->taskId = s_baseTaskId++;

    // StateMachine configuration

    // 1. create states
    QState *sIdle = new QState;
    QState *sWait = new QState;
    connect(sWait, SIGNAL(entered()), this, SLOT(lockTask()));
    connect(sWait, SIGNAL(exited()), this, SLOT(unlockTask()));
    QState *sActive = new QState;
    connect(sActive, SIGNAL(entered()), this, SLOT(activateTask()));
    QState *sDeactivate = new QState;
    connect(sDeactivate, SIGNAL(entered()), this, SLOT(deactivateTask()));

    // 2. configure transitions
    sIdle->addTransition(new TestTransition(TestEvent::Wait, sWait));
    sIdle->addTransition(new TestTransition(TestEvent::Activate, sActive));
    sIdle->addTransition(new TestTransition(TestEvent::Refuse, sDeactivate));
    sWait->addTransition(new TestTransition(TestEvent::Refuse, sIdle));
    sWait->addTransition(new TestTransition(TestEvent::Activate, sActive));
    sActive->addTransition(new TestTransition(TestEvent::Deactivate, sDeactivate));
    sDeactivate->addTransition(this, SIGNAL(finished()), sIdle);

    // 3. add states to the machine
    d->stateMachine.addState(sIdle);
    d->stateMachine.addState(sWait);
    d->stateMachine.addState(sActive);
    d->stateMachine.addState(sDeactivate);
    d->stateMachine.setInitialState(sIdle);
    d->stateMachine.start();

    // Task registration
    d->framework->registerTask(this);
}

IFrameworkTask::~IFrameworkTask()
{
    // unregisters this from the IFramework
    d->framework->unregisterTask(this);
    delete d;
}

quint32 IFrameworkTask::taskUid() const
{
    return d->taskId;
}

void IFrameworkTask::controlActivate()
{
    d->stateMachine.postEvent(new TestEvent(TestEvent::Activate));
}

void IFrameworkTask::controlDeactivate()
{
    d->stateMachine.postEvent(new TestEvent(TestEvent::Deactivate));
}

void IFrameworkTask::controlRefuse()
{
    d->stateMachine.postEvent(new TestEvent(TestEvent::Refuse));
}

void IFrameworkTask::controlWait()
{
    d->stateMachine.postEvent(new TestEvent(TestEvent::Wait));
}
