HEADERS += \
    $$PWD/localcommserver.h \
    $$PWD/nokiaqtframework.h \
    $$PWD/nokiaqtinspectionmodel.h \
    $$PWD/blueprint/blueprintmodule.h \
    $$PWD/info/infomodule.h \
    $$PWD/info/infopanel.h \
    $$PWD/painting/paintingmodel.h \
    $$PWD/painting/paintingmodule.h \
    $$PWD/painting/temperaturepanel.h \
    $$PWD/warnings/notificationwidget.h \
    $$PWD/warnings/warningsmodule.h

SOURCES += \
    $$PWD/localcommserver.cpp \
    $$PWD/nokiaqtframework.cpp \
    $$PWD/nokiaqtinspectionmodel.cpp \
    $$PWD/blueprint/blueprintmodule.cpp \
    $$PWD/info/infomodule.cpp \
    $$PWD/info/infopanel.cpp \
    $$PWD/painting/paintingmodel.cpp \
    $$PWD/painting/paintingmodule.cpp \
    $$PWD/painting/temperaturepanel.cpp \
    $$PWD/warnings/notificationwidget.cpp \
    $$PWD/warnings/warningsmodule.cpp

FORMS += \
    $$PWD/info/infopanel.ui \
    $$PWD/painting/temperaturepanel.ui

RESOURCES += \
    $$PWD/nokiaqtframework.qrc

contains(CONFIG, USE_VTK) {
DEFINES += \
    INSPECTOR_PAINTING_VTK

HEADERS += \
    $$PWD/painting/temperature3dview.h

SOURCES += \
    $$PWD/painting/temperature3dview.cpp

INCLUDEPATH += /usr/include/vtk
LIBS += -lQVTK
}
