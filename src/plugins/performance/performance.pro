TEMPLATE = lib
TARGET = Performance
QT += network svg

include(../../qtcreatorplugin.pri)
include(performance_dependencies.pri)

HEADERS += performanceplugin.h performancepane.h performanceserver.h performanceinformation.h performanceplot.h performancewindow.h

SOURCES += performanceplugin.cpp performancepane.cpp performanceserver.cpp performanceinformation.cpp performanceplot.cpp performancewindow.cpp

FORMS += performanceinformation.ui performancewindow.ui

RESOURCES += performance.qrc

OTHER_FILES += Performance.pluginspec
