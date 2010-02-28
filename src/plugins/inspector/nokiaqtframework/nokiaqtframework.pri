HEADERS += \
    $$PWD/datautils.h \
    $$PWD/localcommserver.h \
    $$PWD/nokiaqtframework.h \
    $$PWD/nokiaqtinspectionmodel.h \
    $$PWD/blueprint/blueprintmodule.h \
    $$PWD/info/infomodule.h \
    $$PWD/info/infopanel.h \
    $$PWD/painting/paintingmodule.h \
    $$PWD/painting/thermalmodel.h \
    $$PWD/painting/thermalpanel.h \
    $$PWD/painting/thermaltask.h \
    $$PWD/warnings/notificationwidget.h \
    $$PWD/warnings/warningsmodule.h

SOURCES += \
    $$PWD/datautils.cpp \
    $$PWD/localcommserver.cpp \
    $$PWD/nokiaqtframework.cpp \
    $$PWD/nokiaqtinspectionmodel.cpp \
    $$PWD/blueprint/blueprintmodule.cpp \
    $$PWD/info/infomodule.cpp \
    $$PWD/info/infopanel.cpp \
    $$PWD/painting/paintingmodule.cpp \
    $$PWD/painting/thermalmodel.cpp \
    $$PWD/painting/thermalpanel.cpp \
    $$PWD/painting/thermaltask.cpp \
    $$PWD/warnings/notificationwidget.cpp \
    $$PWD/warnings/warningsmodule.cpp

FORMS += \
    $$PWD/info/infopanel.ui \
    $$PWD/painting/thermalpanel.ui

RESOURCES += \
    $$PWD/nokiaqtframework.qrc


SUPPORT_VTK = $$(QTCREATOR_WITH_INSPECTOR_VTK)
!isEmpty(SUPPORT_VTK) {
    message("Adding experimental support for VTK Thermal Visualization.")

    DEFINES += \
        INSPECTOR_PAINTING_VTK

    HEADERS += \
        $$PWD/painting/thermal3danalysis.h

    SOURCES += \
        $$PWD/painting/thermal3danalysis.cpp

    INCLUDEPATH += /usr/include/vtk-5.4 /usr/include/vtk-5.2 /usr/include/vtk
    LIBS += -lQVTK
} else {
    message("If you want to try Surface Visualization please set the QTCREATOR_WITH_INSPECTOR_VTK environment variable.")
}
