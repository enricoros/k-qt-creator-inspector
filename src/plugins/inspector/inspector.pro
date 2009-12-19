TEMPLATE = lib
TARGET = Inspector
QT += network svg

include(../../qtcreatorplugin.pri)
include(inspector_dependencies.pri)

HEADERS += \
    abstractprobe.h \
    abstractview.h \
    combotreewidget.h \
    commserver.h \
    defaultview.h \
    infodialog.h \
    inspectorframe.h \
    inspectorinstance.h \
    inspectorplugin.h \
    notificationwidget.h \
    plotgrid.h \
    probecontroller.h \
    taskbarwidget.h \
    tasksscene.h \
    taskswidget.h \
    paint-probe/paintprobe.h \
    paint-probe/ptview.h

SOURCES += \
    abstractprobe.cpp \
    abstractview.cpp \
    combotreewidget.cpp \
    commserver.cpp \
    defaultview.cpp \
    infodialog.cpp \
    inspectorframe.cpp \
    inspectorinstance.cpp \
    inspectorplugin.cpp \
    notificationwidget.cpp \
    plotgrid.cpp \
    probecontroller.cpp \
    taskbarwidget.cpp \
    tasksscene.cpp \
    taskswidget.cpp \
    paint-probe/paintprobe.cpp \
    paint-probe/ptview.cpp

FORMS += \
    commview.ui \
    infodialog.ui \
    paint-probe/ptview.ui

RESOURCES += \
    inspector.qrc

OTHER_FILES += \
    Inspector.pluginspec
