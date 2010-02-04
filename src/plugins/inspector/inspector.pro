TEMPLATE = lib
TARGET = Inspector

QT += gui \
    network \
    svg

include(../../qtcreatorplugin.pri)
include(inspector_dependencies.pri)

HEADERS += \
    abstracteasymodel.h \
    abstractpanel.h \
    combotreewidget.h \
    commserver.h \
    iframeworkmodule.h \
    iinspectorframework.h \
    instance.h \
    instancemodel.h \
    inspectorcontainer.h \
    inspectorplugin.h \
    inspectorwindow.h \
    panelcontainerwidget.h \
    plotgrid.h \
    singletabwidget.h \
    statusbarwidget.h \
    targetwindow.h \
    tasksmodel.h \
    tasksscroller.h \
    nvidiacudaframework/nvidiacudaframework.h

SOURCES += \
    abstracteasymodel.cpp \
    abstractpanel.cpp \
    combotreewidget.cpp \
    commserver.cpp \
    iframeworkmodule.cpp \
    iinspectorframework.cpp \
    instance.cpp \
    instancemodel.cpp \
    inspectorcontainer.cpp \
    inspectorplugin.cpp \
    inspectorwindow.cpp \
    panelcontainerwidget.cpp \
    plotgrid.cpp \
    singletabwidget.cpp \
    statusbarwidget.cpp \
    targetwindow.cpp \
    tasksmodel.cpp \
    tasksscroller.cpp \
    nvidiacudaframework/nvidiacudaframework.cpp

RESOURCES += \
    inspector.qrc

include(nokiaqtframework/nokiaqtframework.pri)

OTHER_FILES += \
    Inspector.pluginspec
