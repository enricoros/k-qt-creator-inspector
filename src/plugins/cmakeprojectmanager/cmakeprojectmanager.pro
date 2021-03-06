TEMPLATE = lib
TARGET = CMakeProjectManager
include(../../qtcreatorplugin.pri)
include(cmakeprojectmanager_dependencies.pri)
HEADERS = cmakeproject.h \
    cmakeprojectplugin.h \
    cmakeprojectmanager.h \
    cmakeprojectconstants.h \
    cmakeprojectnodes.h \
    cmaketarget.h \
    makestep.h \
    cmakerunconfiguration.h \
    cmakeopenprojectwizard.h \
    cmakebuildenvironmentwidget.h \
    cmakebuildconfiguration.h
SOURCES = cmakeproject.cpp \
    cmakeprojectplugin.cpp \
    cmakeprojectmanager.cpp \
    cmakeprojectnodes.cpp \
    cmaketarget.cpp \
    makestep.cpp \
    cmakerunconfiguration.cpp \
    cmakeopenprojectwizard.cpp \
    cmakebuildenvironmentwidget.cpp \
    cmakebuildconfiguration.cpp
RESOURCES += cmakeproject.qrc
FORMS +=

OTHER_FILES += CMakeProjectManager.pluginspec
