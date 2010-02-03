TEMPLATE = lib
TARGET = Inspector

QT += gui \
    network \
    svg

include(../../qtcreatorplugin.pri)
include(inspector_dependencies.pri)

HEADERS += \
    abstracteasymodel.h \
    abstractmodule.h \
    abstractpanel.h \
    combotreewidget.h \
    commserver.h \
    iinspectorframework.h \
    instance.h \
    instancemodel.h \
    inspectorcontainer.h \
    inspectorplugin.h \
    inspectorwindow.h \
    nokiaqtframework.h \
    notificationwidget.h \
    nvidiacudaframework.h \
    panelcontainerwidget.h \
    plotgrid.h \
    modulecontroller.h \
    singletabwidget.h \
    statusbarwidget.h \
    targetwindow.h \
    tasksmodel.h \
    tasksscroller.h \
    module-blueprint/blueprintmodule.h \
    module-info/infomodule.h \
    module-info/infopanel.h \
    module-painting/paintingmodel.h \
    module-painting/paintingmodule.h \
    module-painting/temperaturepanel.h \
    module-warnings/warningsmodule.h

SOURCES += \
    abstracteasymodel.cpp \
    abstractmodule.cpp \
    abstractpanel.cpp \
    combotreewidget.cpp \
    commserver.cpp \
    instance.cpp \
    instancemodel.cpp \
    inspectorcontainer.cpp \
    inspectorplugin.cpp \
    inspectorwindow.cpp \
    nokiaqtframework.cpp \
    notificationwidget.cpp \
    nvidiacudaframework.cpp \
    panelcontainerwidget.cpp \
    plotgrid.cpp \
    modulecontroller.cpp \
    singletabwidget.cpp \
    statusbarwidget.cpp \
    targetwindow.cpp \
    tasksmodel.cpp \
    tasksscroller.cpp \
    module-blueprint/blueprintmodule.cpp \
    module-info/infomodule.cpp \
    module-info/infopanel.cpp \
    module-painting/paintingmodel.cpp \
    module-painting/paintingmodule.cpp \
    module-painting/temperaturepanel.cpp \
    module-warnings/warningsmodule.cpp

FORMS += \
    module-info/infopanel.ui \
    module-painting/temperaturepanel.ui

RESOURCES += \
    inspector.qrc

OTHER_FILES += \
    Inspector.pluginspec
