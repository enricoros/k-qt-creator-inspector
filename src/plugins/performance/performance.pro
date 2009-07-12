TEMPLATE = lib
TARGET = Performance
QT += network

include(../../qtcreatorplugin.pri)
include(performance_dependencies.pri)

HEADERS += performanceplugin.h performancepane.h performanceserver.h

SOURCES += performanceplugin.cpp performancepane.cpp performanceserver.cpp

RESOURCES += performance.qrc

OTHER_FILES += Performance.pluginspec
