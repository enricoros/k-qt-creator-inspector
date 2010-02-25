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
    dashboardwindow.h \
    iframework.h \
    iframeworkmodule.h \
    iframeworktask.h \
    iinspectionmodel.h \
    inspection.h \
    inspectiontarget.h \
    inspectionwindow.h \
    inspectorcontainer.h \
    inspectorplugin.h \
    inspectorrunner.h \
    inspectorstyle.h \
    modulemenuwidget.h \
    panelcontainerwidget.h \
    plotgrid.h \
    probeinjectingdebugger.h \
    runcontrolwatcher.h \
    shareddebugger.h \
    singletabwidget.h \
    statusbarwidget.h \
    tasksmodel.h \
    tasksscroller.h \
    nvidiacudaframework/nvidiacudaframework.h

SOURCES += \
    abstracteasymodel.cpp \
    abstractpanel.cpp \
    dashboardwindow.cpp \
    iframework.cpp \
    iframeworkmodule.cpp \
    iframeworktask.cpp \
    iinspectionmodel.cpp \
    inspection.cpp \
    inspectiontarget.cpp \
    inspectionwindow.cpp \
    inspectorcontainer.cpp \
    inspectorplugin.cpp \
    inspectorrunner.cpp \
    inspectorstyle.cpp \
    modulemenuwidget.cpp \
    panelcontainerwidget.cpp \
    plotgrid.cpp \
    probeinjectingdebugger.cpp \
    runcontrolwatcher.cpp \
    shareddebugger.cpp \
    singletabwidget.cpp \
    statusbarwidget.cpp \
    tasksmodel.cpp \
    tasksscroller.cpp \
    nvidiacudaframework/nvidiacudaframework.cpp

RESOURCES += \
    inspector.qrc \
    nvidiacudaframework/nvidiacudaframework.qrc

include(nokiaqtframework/nokiaqtframework.pri)

OTHER_FILES += \
    Inspector.pluginspec
