TEMPLATE = lib
CONFIG += shared
linux-* {
    CONFIG -= release
    CONFIG += debug
}
SOURCES = gdbmacros.cpp perfunction.cpp
false {
    DEFINES += USE_QT_GUI=0
    QT = core \
        network
}
else {
    DEFINES += USE_QT_GUI=1
    QT = core \
        gui \
        network
}
exists($$QMAKE_INCDIR_QT/QtCore/private/qobject_p.h):DEFINES += HAS_QOBJECT_P_H
HEADERS += gdbmacros_p.h
