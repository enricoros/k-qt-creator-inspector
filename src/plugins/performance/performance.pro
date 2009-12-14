TEMPLATE = lib
TARGET = Performance
QT += network svg

include(../../qtcreatorplugin.pri)
include(performance_dependencies.pri)

HEADERS += \
    infoview.h \
    performanceplugin.h \
    performancenotification.h \
    performanceserver.h \
    performancewindow.h \
    performancemanager.h \
    plotgrid.h \
    ptview.h

SOURCES += \
    infoview.cpp \
    performanceplugin.cpp \
    performancenotification.cpp \
    performanceserver.cpp \
    performancewindow.cpp \
    performancemanager.cpp \
    plotgrid.cpp \
    ptview.cpp

FORMS += \
    commview.ui \
    infoview.ui \
    ptview.ui

RESOURCES += performance.qrc

OTHER_FILES += Performance.pluginspec
