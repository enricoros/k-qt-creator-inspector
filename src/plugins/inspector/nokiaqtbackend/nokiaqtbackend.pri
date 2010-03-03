HEADERS += \
    $$PWD/datautils.h \
    $$PWD/localcommserver.h \
    $$PWD/nokiaqtbackend.h \
    $$PWD/nokiaqtinspectionmodel.h \
    $$PWD/anomaly/notificationwidget.h \
    $$PWD/anomaly/anomalymodule.h \
    $$PWD/blueprint/blueprintmodule.h \
    $$PWD/info/infomodule.h \
    $$PWD/info/infopanel.h \
    $$PWD/painting/frequencypanel.h \
    $$PWD/painting/frequencytask.h \
    $$PWD/painting/paintingmodule.h \
    $$PWD/painting/paintingpanel.h \
    $$PWD/painting/thermalmodel.h \
    $$PWD/painting/thermalpanel.h \
    $$PWD/painting/thermaltask.h

SOURCES += \
    $$PWD/datautils.cpp \
    $$PWD/localcommserver.cpp \
    $$PWD/nokiaqtbackend.cpp \
    $$PWD/nokiaqtinspectionmodel.cpp \
    $$PWD/anomaly/notificationwidget.cpp \
    $$PWD/anomaly/anomalymodule.cpp \
    $$PWD/blueprint/blueprintmodule.cpp \
    $$PWD/info/infomodule.cpp \
    $$PWD/info/infopanel.cpp \
    $$PWD/painting/frequencypanel.cpp \
    $$PWD/painting/frequencytask.cpp \
    $$PWD/painting/paintingmodule.cpp \
    $$PWD/painting/paintingpanel.cpp \
    $$PWD/painting/thermalmodel.cpp \
    $$PWD/painting/thermalpanel.cpp \
    $$PWD/painting/thermaltask.cpp

FORMS += \
    $$PWD/info/infopanel.ui \
    $$PWD/painting/frequencypanel.ui \
    $$PWD/painting/paintingpanel.ui \
    $$PWD/painting/thermalpanel.ui

RESOURCES += \
    $$PWD/nokiaqtbackend.qrc


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
