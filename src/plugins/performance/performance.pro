TEMPLATE = lib
TARGET = Performance
QT += network

include(../../qtcreatorplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)

HEADERS += performanceplugin.h performancepane.h performanceserver.h

SOURCES += performanceplugin.cpp performancepane.cpp performanceserver.cpp

RESOURCES += performance.qrc
