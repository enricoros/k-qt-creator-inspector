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
#include "probedata.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QObject>
#include <QtCore/QMetaMethod>

#include <QtNetwork/QLocalSocket>

#include <QtGui/QApplication>
#include <QtGui/QFont>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QWidget>

#if 0
#include <QtCore/QTime>
#else
#include <sys/time.h>
#endif

#define IP_NAME "QtCreator Inspector Plugin"

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

// output to stderr
#if defined(__GNUC__) && !(defined(__CC_ARM) || defined(__ARMCC__))
#define PF_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
#define PF_PRETTY_FUNCTION __FUNCTION__
#endif
#define CONSOLE_PRINT(...) do {fprintf(stderr, "%s:%d %s: ", __FILE__, __LINE__, PF_PRETTY_FUNCTION); fprintf(stderr, __VA_ARGS__); fprintf(stderr,"\n");} while(0)

namespace Inspector {
namespace Probe {

class CommClient {
public:
    CommClient(const char * serverName)
        : m_fencing(false)
    {
        m_sock = new QLocalSocket();
        m_sock->connectToServer(serverName, QIODevice::WriteOnly | QIODevice::Unbuffered);
        m_connected = m_sock->waitForConnected(10000);
        if (!m_connected)
            printError("can't establish connection to the Inspector server");
    }

    ~CommClient()
    {
        delete m_sock;
    }

    bool isConnected() const
    {
        return m_connected;
    }

    bool sendCustom(quint32 channel, quint32 code1, const QByteArray &data = QByteArray())
    {
        return sendMarshalled(channel, code1, data);
    }

    bool sendInteger(quint32 channel, quint32 code1, int value)
    {
        return sendMarshalled(channel, code1, QString::number(value).toLatin1());
    }

    bool sendDouble(quint32 channel, quint32 code1, double value)
    {
        return sendMarshalled(channel, code1, QString::number(value).toLatin1());
    }

    bool sendImage(quint32 channel, quint32 code1, const QImage & image)
    {
        QByteArray imageData;
        QDataStream dataWriter(&imageData, QIODevice::WriteOnly);
        dataWriter << image.size();
        dataWriter << (quint32)image.format();
        dataWriter << QByteArray((const char *)image.bits(), image.numBytes());
        return sendMarshalled(channel, code1, imageData);
    }

    bool sendMesh(quint32 channel, quint32 code1, const RegularMeshRealData &mesh)
    {
        QByteArray meshData;
        QDataStream dataWriter(&meshData, QIODevice::WriteOnly);
        dataWriter << mesh.physicalSize;
        dataWriter << mesh.rows;
        dataWriter << mesh.columns;
        dataWriter << mesh.meanPatchSize;
        dataWriter << mesh.data;
        return sendMarshalled(channel, code1, meshData);
    }
/*
    bool sendMessage(const QString & string)
    {
        return sendMarshalled(Inspector::Probe::Channel_General, 0x01, string.toLatin1());
    }

    bool sendError(const QString & string)
    {
        return sendMarshalled(Inspector::Probe::Channel_General, 0x02, string.toLatin1());
    }
*/
    void printError(const QString & string) const
    {
        CONSOLE_PRINT("%s", qPrintable(string));
    }

    inline bool fencing() const
    {
        return m_fencing;
    }

private:
    inline bool sendMarshalled(quint32 channel, quint32 code1, const QByteArray &data)
    {
        // send the message
        if (m_fencing) {
            printError("writeData during fence!");
            return false;
        }
        m_fencing = true;
        const QByteArray marshalled = Inspector::Probe::marshallMessage(channel, code1, data);
        m_sock->write(marshalled);
        if (!m_sock->waitForBytesWritten(5000))
            printError("error in waitForBytesWritten!");
        m_sock->flush();
        m_fencing = false;
        return true;
    }

    QLocalSocket * m_sock;
    bool m_connected;
    bool m_fencing;
};

} // namespace Probe
} // namespace Inspector


// static plugin data
extern "C" {
static Inspector::Probe::CommClient * ipCommClient = 0;
static bool ipDebugPainting = false;
static bool ipThermalAnalysis = false;
}

static bool eventInterceptorCallback(void **data)
{
    QEvent *event = reinterpret_cast<QEvent*>(data[1]);
    if (ipCommClient && !ipCommClient->fencing()
        && event->type() >= QEvent::Timer && event->type() <= QEvent::User
        && !ipThermalAnalysis) {
        static int stackDepth = 0;
        ++stackDepth;
        static int numE = 0;
        quint32 localE = ++numE;
        QObject *receiver = reinterpret_cast<QObject*>(data[0]);
        bool *resultValue = reinterpret_cast<bool*>(data[2]);

        // invoke function and measure time
#if 0
        QTime time;
        time.start();
#else
        struct timeval tv1, tv2;
        gettimeofday(&tv1, 0);
#endif
        *resultValue = QCoreApplication::instance()->notify(receiver, event);
#if 0
        double elapsedMs = (double)time.elapsed();
#else
        gettimeofday(&tv2, 0);
        double elapsedMs = (double)(tv2.tv_sec - tv1.tv_sec) * 1000.0 + (double)(tv2.tv_usec - tv1.tv_usec) / 1000.0;
#endif

        // send out data
        // TODO: use a per-thread STACK for SIGNALS AND SLOTS SENDING HERE ?
        ipCommClient->sendDouble(Inspector::Probe::Channel_Events, 0, elapsedMs);

        // check for too long events
        if (elapsedMs > 200) {
            QByteArray eventData;
            QDataStream dataWriter(&eventData, QIODevice::WriteOnly);
            dataWriter << localE;
            dataWriter << (quint32)event->type();
            dataWriter << elapsedMs;
            dataWriter << (receiver->metaObject()->className() ? receiver->metaObject()->className() : "null");
            ipCommClient->sendCustom(Inspector::Probe::Channel_Events, 1, eventData);
        }

        // show painting, if
        if (ipDebugPainting && event->type() == QEvent::Paint) {
            if (QWidget * widget = dynamic_cast<QWidget *>(receiver)) {
                static int paintOpNumber = 0;
                QPainter p(widget);
                int hue = qrand() % 360;
                p.setBrush(QColor::fromHsv(hue, 255, 255, 128));
                p.drawRect(static_cast<QPaintEvent *>(event)->rect().adjusted(0, 0, -1, -1));
                p.setFont(QFont("Arial",8));
                p.drawText(static_cast<QPaintEvent *>(event)->rect().topLeft() + QPoint(2,10), QString::number(++paintOpNumber));
            }
        }

        // TODO: measure send+painting overhead

        --stackDepth;
        return true;
    }
    return false;
}
#if 0
static void signalBeginCallback(QObject *caller, int method_index, void **/*argv*/)
{
    CONSOLE_PRINT("signal BEGIN: %s", caller->metaObject()->method(method_index).signature());
}

static void signalEndCallback(QObject *caller, int method_index)
{
    CONSOLE_PRINT("signal END: %s", caller->metaObject()->method(method_index).signature());
}

static void slotBeginCallback(QObject *caller, int method_index, void **/*argv*/)
{
    CONSOLE_PRINT("slot BEGIN: %s", caller->metaObject()->method(method_index).signature());
}

static void slotEndCallback(QObject *caller, int method_index)
{
    CONSOLE_PRINT("slot END: %s", caller->metaObject()->method(method_index).signature());
}
#endif

// Entry Points of the Shared Library (loaded by the GDB plugin)
extern "C"
Q_DECL_EXPORT bool qInspectorActivate(const char * serverName, int activationFlags)
{
    // 1. comm client
    if (ipCommClient) {
        ipCommClient->printError("already active");
        return false;
    } else
        ipCommClient = new Inspector::Probe::CommClient(serverName);

    // 2. activation flags
    ipDebugPainting = activationFlags & Inspector::Probe::AF_PaintDebug;

    // 3. signal spy callback
    QSignalSpyCallbackSet set = {0, 0/*slotBeginCallback*/, 0, 0/*slotEndCallback*/};
    qt_register_signal_spy_callbacks(set);

    // 4. events callback
    QInternal::registerCallback(QInternal::EventNotifyCallback, eventInterceptorCallback);

    CONSOLE_PRINT(IP_NAME": Activated");
    ipCommClient->sendCustom(Inspector::Probe::Channel_General, 0x00);
    return true;
}

extern "C"
Q_DECL_EXPORT void qInspectorDeactivate()
{
    CONSOLE_PRINT(IP_NAME": Deactivated");

    // 4. events callback
    QInternal::unregisterCallback(QInternal::EventNotifyCallback, eventInterceptorCallback);

    // 3. signal spy callback
    QSignalSpyCallbackSet set = {0, 0, 0, 0};
    qt_register_signal_spy_callbacks(set);

    // 1. comm client
    delete ipCommClient;
    ipCommClient = 0;
}

struct __TimedRect {
    QRect rect;
    QList<double> times;
    double totalTime;
};

extern "C"
Q_DECL_EXPORT void qThermalAnalysis(int passes, int headDrops, int tailDrops,
    int innerPasses, int chunkWidth, int chunkHeight, bool consoleDebug)
{
    // sanity check
    if (!ipCommClient) {
        CONSOLE_PRINT("not connected to the Inspector");
        return;
    }

    // check for graphical environment
    QApplication * app = dynamic_cast<QApplication *>(QCoreApplication::instance());
    if (!app) {
        CONSOLE_PRINT("no QApplication");
        // TODO: add a code for the PaintProbe comm!
        //ipCommClient->sendError("No QApplication in this window");
        return;
    }

    // signal the presence of this operation
    ipThermalAnalysis = true;

    // tell that the operation has started
    ipCommClient->sendCustom(Inspector::Probe::Channel_Painting, 1);
    ipCommClient->sendInteger(Inspector::Probe::Channel_Painting, 3, 1);

    // vars
    struct timeval tv1, tv2;
    if (consoleDebug)
        CONSOLE_PRINT("start, %d passes (drop %dh %dt), %d inner, chunk %dx%d", passes, headDrops, tailDrops, innerPasses, chunkWidth, chunkHeight);

    foreach (QWidget * widget, app->topLevelWidgets()) {
        if (!widget || !widget->isVisible() || widget->width() < 50 || widget->height() < 50)
            continue;

        // per-widget params and rect subdivision
        const int wW = widget->width();
        const int wH = widget->height();
        const int wCols = widget->width() / chunkWidth;
        const int wRows = widget->height() / chunkHeight;
        const int wRects = wCols * wRows;
        qreal meanPatchSize = 0;
        QVector<__TimedRect> wTimedRects;
        {
            int y1 = 0;
            for (int row = 0; row < wRows; row++) {
                int y2 = (wH * (row + 1)) / wRows;
                int x1 = 0;
                for (int col = 0; col < wCols; col++) {
                    int x2 = (wW * (col + 1)) / wCols;
                    __TimedRect tRect;
                    tRect.rect = QRect(x1, y1, x2 - x1, y2 - y1);
                    wTimedRects.append(tRect);
                    meanPatchSize += tRect.rect.width() * tRect.rect.height();
                    x1 = x2;
                }
                y1 = y2;
            }
            meanPatchSize /= (double)wRects;
        }
        if (consoleDebug)
            CONSOLE_PRINT("done subdivision");

        // do the test over the widget
        QImage baseImage(wW, wH, QImage::Format_ARGB32);
        baseImage.fill(0);
        widget->render(&baseImage);
        if (consoleDebug)
            CONSOLE_PRINT("send snapshot");

        ipCommClient->sendImage(Inspector::Probe::Channel_Painting, 4, baseImage);

        QImage testImage(wW, wH, QImage::Format_ARGB32);
        testImage.fill(0);
        if (consoleDebug)
            CONSOLE_PRINT("snapshot sent, inited target image");

        // test the rects
        const int percTotal = passes * wRects;
        const int percStep = percTotal > 100 ? percTotal / 100 : 1;
        int percCycle = 0;
        int percProgress = 0;
        for (int pass = 0; pass < passes; pass++) {
            CONSOLE_PRINT("pass %d", pass);
            for (int i = 0; i < wRects; i++) {

                //QImage testImage(testRect.size(), QImage::Format_ARGB32);
                //usleep(1000);

                //if (consoleDebug)
                //    CONSOLE_PRINT("rect %d of %d", i + 1, wRects);

                // timed rendering
                gettimeofday(&tv1, 0);
                for (int rep = 0; rep < innerPasses; ++rep)
                    widget->render(&testImage, QPoint() /*wTimedRects[i].rect.topLeft()*/, wTimedRects[i].rect, QWidget::DrawChildren /*| DrawWindowBackground*/);
                gettimeofday(&tv2, 0);

                // accumulate time
                const double elapsedMs = (double)(tv2.tv_sec - tv1.tv_sec) * 1000.0 + (double)(tv2.tv_usec - tv1.tv_usec) / 1000.0;
                wTimedRects[i].times.append(elapsedMs);

                // send percentages at each step
                percProgress++;
                percCycle++;
                if (percCycle >= percStep) {
                    int percent = (percProgress * 100) / percTotal;
                    percCycle = 0;
                    ipCommClient->sendInteger(Inspector::Probe::Channel_Painting, 3, percent);
                    if (consoleDebug)
                        CONSOLE_PRINT("%d done", percent);
                }
            }
        }

        // tell that we reached 100%
        ipCommClient->sendInteger(Inspector::Probe::Channel_Painting, 3, 100);
        if (consoleDebug)
            CONSOLE_PRINT("100 done");

        // single rect: drop min/max measured time value(s) and normalize
        double normalizer = 1.0 / (double)(innerPasses * (passes - headDrops - tailDrops));
        QVector<__TimedRect>::iterator rIt = wTimedRects.begin(), rEnd = wTimedRects.end();
        for (; rIt != rEnd; rIt++) {
            __TimedRect & tRect = *rIt;
            tRect.totalTime = 0;
            qSort(tRect.times);
            for (int idx = headDrops; idx < (tRect.times.size() - tailDrops); ++idx)
                tRect.totalTime += tRect.times[idx];
            tRect.totalTime *= normalizer;
        }
        if (consoleDebug)
            CONSOLE_PRINT("done statistics");

        // send out the resulting Mesh
        Inspector::Probe::RegularMeshRealData mesh;
        mesh.physicalSize = QRect(0, 0, wW, wH);
        mesh.rows = wRows;
        mesh.columns = wCols;
        mesh.meanPatchSize = meanPatchSize;
        foreach (const __TimedRect & tRect, wTimedRects)
            mesh.data.append(tRect.totalTime);
        ipCommClient->sendMesh(Inspector::Probe::Channel_Painting, 5, mesh);
        if (consoleDebug)
            CONSOLE_PRINT("done sending mesh");

        // process just the first widget
        break;
    }

    // signal the presence of this operation
    ipThermalAnalysis = false;

    // tell that the operation has finished
    ipCommClient->sendCustom(Inspector::Probe::Channel_Painting, 2);

    // run event loop, to flush out the localsocket - FIXME - avoid this - DOESN'T WORK ANYWAY
    //app->processEvents();
}
