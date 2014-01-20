#-------------------------------------------------
#
# Project created by QtCreator 2012-12-15T20:28:49
#
#-------------------------------------------------

QT     += core gui network

include(./qtsingleapplication/qtsingleapplication.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IRemote-Desktop
TEMPLATE = app

SOURCES += main.cpp\
    graphicbutton.cpp \
    showcommanddialog.cpp \
    crc.cpp \
    base64.cpp \
    qremoteboxclient.cpp \
    mainwindow.cpp

HEADERS  += \
    graphicbutton.h \
    showcommanddialog.h\
    crc.h \
    base64.h \
    qremoteboxclient.h \
    mainwindow.h

FORMS    += \
    showcommanddialog.ui \
    mainwindow.ui
