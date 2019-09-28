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
    editdialog.cpp \
    hexwriter.cpp \
    crc.cpp


HEADERS  += mainwindow.h \
    advdialog.h \
    firmware.h \
    editdialog.h \
    hexwriter.h \
    crc.h

FORMS    += mainwindow.ui \
    editdialog.ui \
    advdialog.ui

RESOURCES += \
    resources.qrc

unix:macx{
HEADERS += usb_macos.h \
        notifier_macos.h
SOURCES += usb_macos.cpp \
        notifier_macos.cpp
LIBS += -L/opt/local/lib \
        -lusb-1.0 \
        -framework IOKit \
        -framework Carbon
QMAKE_CXXFLAGS += "-std=c++0x"
INCPATH += /opt/local/include
}

unix:!macx{
HEADERS += usb_linux.h \
        notifier_linux.h
SOURCES += usb_linux.cpp \
        notifier_linux.cpp
LIBS += -ludev \
        -lusb-1.0
}

win32:{
HEADERS += usb_win.h \
        notifier_win.h
SOURCES += usb_win.cpp \
        notifier_win.cpp
LIBS += -luser32 \
        -lsetupapi
RC_FILE = win_resources.rc
}

