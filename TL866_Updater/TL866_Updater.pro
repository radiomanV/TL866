#-------------------------------------------------
#
# Project created by QtCreator 2014-01-28T20:16:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TL866_Updater
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    advdialog.cpp \
    firmware.cpp \
    crc16.cpp \
    editdialog.cpp \
    hexwriter.cpp \
    notifier.cpp


HEADERS  += mainwindow.h \
    advdialog.h \
    firmware.h \
    crc16.h \
    editdialog.h \
    hexwriter.h \
    notifier.h \
    tl866_global.h

FORMS    += mainwindow.ui \
    editdialog.ui \
    advdialog.ui

RESOURCES += \
    resources.qrc

unix:!macx{
HEADERS += usb_linux.h
SOURCES += usb_linux.cpp
LIBS += -ludev \
        -lusb-1.0
}

win32:{
HEADERS += usb_win.h
SOURCES += usb_win.cpp
LIBS += user32.lib \
        Setupapi.lib
RC_FILE = win_resources.rc
}

