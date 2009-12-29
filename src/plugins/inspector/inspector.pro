TEMPLATE = lib
TARGET = Inspector
QT += network svg

include(../../qtcreatorplugin.pri)
include(inspector_dependencies.pri)

HEADERS += \
    abstractmodule.h \
    abstractview.h \
    combotreewidget.h \
    commserver.h \
    inspectorframe.h \
    instance.h \
    inspectorplugin.h \
    instanceview.h \
    notificationwidget.h \
    plotgrid.h \
    modulecontroller.h \
    taskbarwidget.h \
    tasksscene.h \
    taskswidget.h \
    module-painting/paintmodule.h \
    module-painting/painttemperatureview.h

SOURCES += \
    abstractmodule.cpp \
    abstractview.cpp \
    combotreewidget.cpp \
    commserver.cpp \
    inspectorframe.cpp \
    instance.cpp \
    inspectorplugin.cpp \
    instanceview.cpp \
    notificationwidget.cpp \
    plotgrid.cpp \
    modulecontroller.cpp \
    taskbarwidget.cpp \
    tasksscene.cpp \
    taskswidget.cpp \
    module-painting/paintmodule.cpp \
    module-painting/painttemperatureview.cpp

FORMS += \
    instanceview.ui \
    module-painting/painttemperatureview.ui

RESOURCES += \
    inspector.qrc

OTHER_FILES += \
    Inspector.pluginspec
