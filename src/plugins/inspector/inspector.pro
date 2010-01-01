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
    instance.h \
    instancemodel.h \
    inspectorplugin.h \
    notificationwidget.h \
    plotgrid.h \
    modulecontroller.h \
    taskbarwidget.h \
    tasksscene.h \
    taskswidget.h \
    viewcontainerwidget.h \
    window.h \
    module-info/infomodule.h \
    module-info/infoview.h \
    module-painting/paintmodule.h \
    module-painting/painttemperatureview.h

SOURCES += \
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
    taskbarwidget.cpp \
    tasksscene.cpp \
    taskswidget.cpp \
    viewcontainerwidget.cpp \
    window.cpp \
    module-info/infomodule.cpp \
    module-info/infoview.cpp \
    module-painting/paintmodule.cpp \
    module-painting/painttemperatureview.cpp

FORMS += \
    module-info/infoview.ui \
    module-painting/painttemperatureview.ui

RESOURCES += \
    inspector.qrc

OTHER_FILES += \
    Inspector.pluginspec
