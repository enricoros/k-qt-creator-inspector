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
    module-warnings/notificationwidget.h \
    module-warnings/warningsmodule.h \
    nokiaqtframework/nokiaqtframework.h \
    nvidiacudaframework/nvidiacudaframework.h

SOURCES += \
    abstracteasymodel.cpp \
    abstractpanel.cpp \
    combotreewidget.cpp \
    commserver.cpp \
    iframeworkmodule.cpp \
    instance.cpp \
    instancemodel.cpp \
    inspectorcontainer.cpp \
    inspectorplugin.cpp \
    inspectorwindow.cpp \
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
    module-warnings/notificationwidget.cpp \
    module-warnings/warningsmodule.cpp \
    nokiaqtframework/nokiaqtframework.cpp \
    nvidiacudaframework/nvidiacudaframework.cpp

FORMS += \
    module-info/infopanel.ui \
    module-painting/temperaturepanel.ui

RESOURCES += \
    inspector.qrc

OTHER_FILES += \
    Inspector.pluginspec
