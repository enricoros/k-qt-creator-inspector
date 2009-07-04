TEMPLATE = lib
TARGET = Performance

include(../../qtcreatorplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)

HEADERS += performanceplugin.h \
    performancewindow.h

SOURCES += performanceplugin.cpp \
    performancewindow.cpp
