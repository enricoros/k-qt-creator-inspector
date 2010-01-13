TEMPLATE = lib
TARGET = Inspector
QT += network svg

include(../../qtcreatorplugin.pri)
include(inspector_dependencies.pri)

HEADERS += \
    abstracteasymodel.h \
    abstractmodule.h \
    abstractpanel.h \
    combotreewidget.h \
    commserver.h \
    instance.h \
    instancemodel.h \
    inspectorplugin.h \
    notificationwidget.h \
    panelcontainerwidget.h \
    plotgrid.h \
    modulecontroller.h \
    statusbarwidget.h \
    tasksmodel.h \
    tasksscroller.h \
    window.h \
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
    inspectorplugin.cpp \
    notificationwidget.cpp \
    panelcontainerwidget.cpp \
    plotgrid.cpp \
    modulecontroller.cpp \
    statusbarwidget.cpp \
    tasksmodel.cpp \
    tasksscroller.cpp \
    window.cpp \
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
