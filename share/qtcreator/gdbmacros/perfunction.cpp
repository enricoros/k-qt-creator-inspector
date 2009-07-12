/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
** contact the sales department at http://www.qtsoftware.com/contact.
**
**************************************************************************/

#include "perfunction.h"

#include <QCoreApplication>
#include <QObject>

#define PP_NAME "QtCreator Performance Plugin"

/**
   This file shamelessyly hijacks the event dispatching callbacks and the
   Signal/Slots callbacks for Qt 4.5+ Programs.

   Conformance checking (symbols to be hijacked) is done in the DebuggerPlugin,
   here we suppose everything works right (symbols are found, etc.).

   There are 2 types of callbacks we use:
    - the EVENT callbacks %MORE HERE%
    - the SIGNAL/SLOTS callbacks %MORE HERE%

   All the data analysis (distribution, frequency, patterns) is done on the GUI
   side. Here we only collect the data and send it out.
*/

/// Mirrored from 'qobject_p.h'
struct QSignalSpyCallbackSet
{
    typedef void (*BeginCallback)(QObject *caller, int method_index, void **argv);
    typedef void (*EndCallback)(QObject *caller, int method_index);
    BeginCallback signal_begin_callback,
                    slot_begin_callback;
    EndCallback signal_end_callback,
                slot_end_callback;
};
void Q_CORE_EXPORT qt_register_signal_spy_callbacks(const QSignalSpyCallbackSet &callback_set);

/** Mirrored from 'qglobal.cpp'
typedef bool (*qInternalCallback)(void **);

class Q_CORE_EXPORT QInternal {
public:
    enum PaintDeviceFlags {
        ...
    };
    enum RelayoutType {
        ...
    };
    enum Callback {
        ConnectCallback,
        DisconnectCallback,
        AdoptCurrentThread,
        EventNotifyCallback,
        LastCallback
    };
    enum InternalFunction {
        ...
    };
    enum DockPosition {
        ...
    };
    static bool registerCallback(Callback, qInternalCallback);
    static bool unregisterCallback(Callback, qInternalCallback);
    static bool activateCallbacks(Callback, void **);
    static bool callFunction(InternalFunction func, void **);
};
struct QInternal_CallBackTable {
    QVector<QList<qInternalCallback> > callbacks;
};
Q_GLOBAL_STATIC(QInternal_CallBackTable, global_callback_table)

Connect Activation, qobject.cpp:
        const void *cbdata[] = { sender, signal, receiver, method, &type };
        if (QInternal::activateCallbacks(QInternal::ConnectCallback, (void **) cbdata))
            return true;

Disconnect Activation, qobject.cpp:
        const void *cbdata[] = { sender, signal, receiver, method };
        if (QInternal::activateCallbacks(QInternal::DisconnectCallback, (void **) cbdata))
            return true;

EventNotify Activation, qcoreapplication.cpp:
        bool result = false;
        void *cbdata[] = { receiver, event, &result };
        if (QInternal::activateCallbacks(QInternal::EventNotifyCallback, cbdata)) {
            return result;
        }
*/
#include <QPainter>
#include <QPaintEvent>
#include <QWidget>
#include <QFont>
/*static bool paintInterceptorCallback(void **data)
{
    QEvent *event = reinterpret_cast<QEvent*>(data[1]);
    if (event->type() == QEvent::Paint) {
        QObject *receiver = reinterpret_cast<QObject*>(data[0]);
        static int stackDepth = 0;
        ++stackDepth;
        static int i = 0;
        int localI = ++i;
        qWarning("<ICEpted Paint %d (%d) to %s>", localI, stackDepth, receiver->metaObject()->className() ? receiver->metaObject()->className() : "null");
        QCoreApplication::instance()->notify(receiver, event);
        if (QWidget * widget = dynamic_cast<QWidget *>(receiver)) {
            QPainter p(widget);
            int hue = qrand() % 360;
            p.setBrush(QColor::fromHsv(hue, 255, 255, 128));
            p.drawRect(static_cast<QPaintEvent *>(event)->rect().adjusted(0, 0, -1, -1));
            p.setFont(QFont("Arial",8));
            p.drawText(static_cast<QPaintEvent *>(event)->rect().topLeft() + QPoint(2,10), QString::number(localI));
        }
        qWarning("</ICEpted Paint %d>", localI);
        bool *result = reinterpret_cast<bool*>(data[2]);
        *result = true;
        --stackDepth;
        return true;
    }
    return false;
}
*/

#include <QLocalSocket>
class PerfCommClient {
    public:
        PerfCommClient()
            : m_fencing(false)
        {
            m_sock = new QLocalSocket();
            m_sock->connectToServer("performance1");
            m_connected = m_sock->waitForConnected(10000);
            if (!m_connected)
                writeError("can't establish connection to the performance server");
        }

        ~PerfCommClient()
        {
            delete m_sock;
        }

        bool isConnected() const
        {
            return m_connected;
        }

        bool writeData(const QString & string)
        {
            if (m_fencing) {
                writeError("writeData during fence!");
                return false;
            }
            m_fencing = true;
            m_sock->write(string.toLatin1());
            m_sock->waitForBytesWritten(1000);
            m_fencing = false;
            return true;
        }

        void writeError(const QString & string)
        {
            qWarning(PP_NAME": %s", qPrintable(string));
        }

        inline bool fencing() const
        {
            return m_fencing;
        }

    private:
        QLocalSocket * m_sock;
        bool m_connected;
        bool m_fencing;
};
static PerfCommClient * commClient = 0;


#include <QTime>
static bool loadInterceptorCallback(void **data)
{
    QEvent *event = reinterpret_cast<QEvent*>(data[1]);
    if (!commClient->fencing() && event->type() >= QEvent::Timer && event->type() <= QEvent::User ) {
        static int stackDepth = 0;
        ++stackDepth;
        static int numE = 0;
        int localE = ++numE;
        QObject *receiver = reinterpret_cast<QObject*>(data[0]);
        bool *resultValue = reinterpret_cast<bool*>(data[2]);
        QTime time;
        time.start();
        *resultValue = QCoreApplication::instance()->notify(receiver, event);
        int elapsed = time.elapsed();
        if (elapsed > 200) {
            QString s = QString("Troppo lungo l'evento %1, tipo %2, durata %3, su %4").arg(localE).arg(event->type()).arg(elapsed).arg(receiver->metaObject()->className() ? receiver->metaObject()->className() : "null");
            commClient->writeData(s);
        }
        --stackDepth;
        return true;
    }
    return false;
}


#include <QMetaMethod>
static void signalBeginCallback(QObject *caller, int method_index, void **argv)
{
    qWarning("sinal BEGIN: %s", caller->metaObject()->method(method_index).signature());
}

static void signalEndCallback(QObject *caller, int method_index)
{
    qWarning("sinal END: %s", caller->metaObject()->method(method_index).signature());
}

static void slotBeginCallback(QObject *caller, int method_index, void **argv)
{
    qWarning("slot BEGIN: %s", caller->metaObject()->method(method_index).signature());
}

static void slotEndCallback(QObject *caller, int method_index)
{
    qWarning("slot END: %s", caller->metaObject()->method(method_index).signature());
}


// Entry Points of the Shared Library (loaded by the GDB plugin)
extern "C"
bool qPerfActivate()
{
    // 1. comm client
    if (commClient) {
        commClient->writeError("already active");
        return false;
    } else
        commClient = new PerfCommClient();

    // 2. signal spy callback
    QSignalSpyCallbackSet set = {0, slotBeginCallback, 0, slotEndCallback};
    qt_register_signal_spy_callbacks(set);

    // 3. events callback
    //QInternal::registerCallback(QInternal::EventNotifyCallback, paintInterceptorCallback);
    QInternal::registerCallback(QInternal::EventNotifyCallback, loadInterceptorCallback);

    qWarning(PP_NAME": Activated");
    commClient->writeData("Active");
    return true;
}

extern "C"
void qPerfDeactivate()
{
    qWarning(PP_NAME": Deactivated");

    // 3. events callback
    QInternal::unregisterCallback(QInternal::EventNotifyCallback, loadInterceptorCallback);

    // 2. signal spy callback
    QSignalSpyCallbackSet set = {0, 0, 0, 0};
    qt_register_signal_spy_callbacks(set);

    // 1. comm client
    delete commClient;
    commClient = 0;
}
