#-------------------------------------------------
#
# Project created by QtCreator 2015-12-23T13:57:40
#
#-------------------------------------------------

QT       += widgets
CONFIG += C++11

TARGET = $$qtLibraryTarget(AutoUpdaterWidgets)
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    updatecontroller.cpp \
    progressdialog.cpp \
    updateinfodialog.cpp

HEADERS += \
    updatecontroller.h \
	updatecontroller_p.h \
    progressdialog.h \
    updateinfodialog.h

INCLUDEPATH += $$PWD/../AutoUpdater
DEPENDPATH += $$PWD/../AutoUpdater

FORMS += \
    progressdialog.ui \
    updateinfodialog.ui

RESOURCES += \
    autoupdaterwidgets_resource.qrc
