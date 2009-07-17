TEMPLATE = lib
TARGET = Performance
QT += network svg

include(../../qtcreatorplugin.pri)
include(performance_dependencies.pri)

HEADERS += performanceplugin.h \
    performancenotification.h \
    performanceserver.h  \
    performanceinformation.h  \
    performanceplot.h  \
    performancewindow.h  \
    performancemanager.h

SOURCES += performanceplugin.cpp  \
    performancenotification.cpp  \
    performanceserver.cpp  \
    performanceinformation.cpp  \
    performanceplot.cpp  \
    performancewindow.cpp  \
    performancemanager.cpp

FORMS += performanceinformation.ui  \
    performancewindow.ui

RESOURCES += performance.qrc

OTHER_FILES += Performance.pluginspec
