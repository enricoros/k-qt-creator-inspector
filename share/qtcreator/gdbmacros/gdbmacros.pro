TEMPLATE = lib
CONFIG += shared
linux-* {
    CONFIG -= release
    CONFIG += debug
}
SOURCES = gdbmacros.cpp
false {
    DEFINES += USE_QT_GUI=0
    QT = core
}
else {
    DEFINES += USE_QT_GUI=1
    QT = core \
        gui
}
exists($$QMAKE_INCDIR_QT/QtCore/private/qobject_p.h):DEFINES += HAS_QOBJECT_P_H
HEADERS += gdbmacros_p.h

# Inspector: add some functions and link to the network
# enabling by default for the compilation within creator
#SUPPORT_INSPECTOR_PLUGIN = $$(QTCREATOR_WITH_INSPECTOR)
#!isEmpty(SUPPORT_INSPECTOR_PLUGIN) {
    message("Inspector: compiling the Debugging Helpers with instrumentation functions.")
    SOURCES += perfunction.cpp
    QT += network
#}
