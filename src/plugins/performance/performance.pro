TEMPLATE = lib
TARGET = Performance
QT += network

include(../../qtcreatorplugin.pri)
include(performance_dependencies.pri)

HEADERS += performanceplugin.h performancepane.h performanceserver.h performanceinformation.h

SOURCES += performanceplugin.cpp performancepane.cpp performanceserver.cpp performanceinformation.cpp

FORMS += performanceinformation.ui

RESOURCES += performance.qrc

OTHER_FILES += Performance.pluginspec
