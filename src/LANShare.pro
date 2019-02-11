#-------------------------------------------------
#
# Project created by QtCreator 2016-11-20T13:00:50
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LANShare
TEMPLATE = app
CONFIG += c++17

RC_ICONS += img/icon.ico

SOURCES += main.cpp\
    settings.cpp \
    util.cpp \
	singleinstance.cpp \
    ui/mainwindow.cpp \
    ui/receiverselectordialog.cpp \
    ui/aboutdialog.cpp \
    ui/settingsdialog.cpp \
    transfer/devicebroadcaster.cpp \
    transfer/receiver.cpp \
    transfer/sender.cpp \
    transfer/transfer.cpp \
    transfer/transferserver.cpp \
#    transfer/transfermanager.cpp \
	model/device.cpp \
	model/devicelistmodel.cpp \
	model/transferinfo.cpp \
	model/transfertablemodel.cpp

HEADERS  += settings.h \
    util.h \
	singleinstance.h \
    ui/mainwindow.h \
    ui/receiverselectordialog.h \
    ui/aboutdialog.h \
    ui/settingsdialog.h \
    transfer/devicebroadcaster.h \
    transfer/receiver.h \
    transfer/sender.h \
    transfer/transfer.h \
    transfer/transferserver.h \
#    transfer/transfermanager.h \
	model/device.h \
	model/devicelistmodel.h \
	model/transferinfo.h \
	model/transfertablemodel.h

FORMS    += ui/mainwindow.ui \
    ui/receiverselectordialog.ui \
    ui/aboutdialog.ui \
    ui/settingsdialog.ui

RESOURCES += \
	res.qrc
