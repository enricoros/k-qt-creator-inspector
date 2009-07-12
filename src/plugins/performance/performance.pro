TEMPLATE = lib
TARGET = Performance
QT += network

include(../../qtcreatorplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/debugger/debugger.pri)

HEADERS += performanceplugin.h performancepane.h performanceserver.h

SOURCES += performanceplugin.cpp performancepane.cpp performanceserver.cpp

RESOURCES += performance.qrc

OTHER_FILES += Performance.pluginspec
