#-------------------------------------------------
#
# Project created by QtCreator 2016-11-20T13:00:50
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LANShare
TEMPLATE = app
CONFIG += c++11

RC_ICONS += img/icon.ico

SOURCES += main.cpp\
        mainwindow.cpp \
    device.cpp \
    devicebroadcaster.cpp \
    devicelistmodel.cpp \
    receiver.cpp \
    sender.cpp \
    settings.cpp \
    transfer.cpp \
    transferserver.cpp \
    receiverselectordialog.cpp \
    aboutdialog.cpp \
    settingsdialog.cpp \
    util.cpp \
    transferinfo.cpp \
#    transfermanager.cpp \
    transfertablemodel.cpp

HEADERS  += mainwindow.h \
    device.h \
    devicebroadcaster.h \
    devicelistmodel.h \
    receiver.h \
    sender.h \
    settings.h \
    transfer.h \
    transferserver.h \
    receiverselectordialog.h \
    aboutdialog.h \
    settingsdialog.h \
    util.h \
    transferinfo.h \
#    transfermanager.h \
    transfertablemodel.h

FORMS    += mainwindow.ui \
    receiverselectordialog.ui \
    aboutdialog.ui \
    settingsdialog.ui

RESOURCES += \
    res.qrc
