TEMPLATE = lib
TARGET = Performance
QT += network svg

include(../../qtcreatorplugin.pri)
include(performance_dependencies.pri)

HEADERS += \
    abstracttest.h \
    infoview.h \
    performanceplugin.h \
    performancenotification.h \
    performanceserver.h \
    performancewindow.h \
    performancemanager.h \
    plotgrid.h \
    taskbarwidget.h \
    tasksscene.h \
    taskswidget.h \
    testcontrol.h \
    paint-test/painttest.h \
    paint-test/ptview.h

SOURCES += \
    abstracttest.cpp \
    infoview.cpp \
    performanceplugin.cpp \
    performancenotification.cpp \
    performanceserver.cpp \
    performancewindow.cpp \
    performancemanager.cpp \
    plotgrid.cpp \
    taskbarwidget.cpp \
    tasksscene.cpp \
    taskswidget.cpp \
    testcontrol.cpp \
    paint-test/painttest.cpp \
    paint-test/ptview.cpp

FORMS += \
    commview.ui \
    infoview.ui \
    paint-test/ptview.ui

RESOURCES += performance.qrc

OTHER_FILES += Performance.pluginspec
