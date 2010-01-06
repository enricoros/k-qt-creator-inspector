TEMPLATE = lib
TARGET = Inspector
QT += network svg

include(../../qtcreatorplugin.pri)
include(inspector_dependencies.pri)

HEADERS += \
    abstracteasymodel.h \
    abstractmodule.h \
    abstractview.h \
    combotreewidget.h \
    commserver.h \
    instance.h \
    instancemodel.h \
    inspectorplugin.h \
    notificationwidget.h \
    plotgrid.h \
    modulecontroller.h \
    statusbarwidget.h \
    tasksmodel.h \
    tasksscene.h \
    taskswidget.h \
    viewcontainerwidget.h \
    window.h \
    module-info/infomodule.h \
    module-info/infoview.h \
    module-painting/paintingmodel.h \
    module-painting/paintingmodule.h \
    module-painting/painttemperatureview.h \
    module-warnings/warningsmodule.h

SOURCES += \
    abstracteasymodel.cpp \
    abstractmodule.cpp \
    abstractview.cpp \
    combotreewidget.cpp \
    commserver.cpp \
    instance.cpp \
    instancemodel.cpp \
    inspectorplugin.cpp \
    notificationwidget.cpp \
    plotgrid.cpp \
    modulecontroller.cpp \
    statusbarwidget.cpp \
    tasksmodel.cpp \
    tasksscene.cpp \
    taskswidget.cpp \
    viewcontainerwidget.cpp \
    window.cpp \
    module-info/infomodule.cpp \
    module-info/infoview.cpp \
    module-painting/paintingmodel.cpp \
    module-painting/paintingmodule.cpp \
    module-painting/painttemperatureview.cpp \
    module-warnings/warningsmodule.cpp

FORMS += \
    module-info/infoview.ui \
    module-painting/painttemperatureview.ui

RESOURCES += \
    inspector.qrc

OTHER_FILES += \
    Inspector.pluginspec
