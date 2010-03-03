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
    ibackend.h \
    ibackendmodule.h \
    ibackendtask.h \
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
    nvidiacudabackend/nvidiacudabackend.h

SOURCES += \
    abstracteasymodel.cpp \
    abstractpanel.cpp \
    dashboardwindow.cpp \
    ibackend.cpp \
    ibackendmodule.cpp \
    ibackendtask.cpp \
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
    nvidiacudabackend/nvidiacudabackend.cpp

RESOURCES += \
    inspector.qrc \
    nvidiacudabackend/nvidiacudabackend.qrc

include(nokiaqtbackend/nokiaqtbackend.pri)

OTHER_FILES += \
    Inspector.pluginspec
