#-------------------------------------------------
#
# Project created by QtCreator 2019-05-06T13:51:51
#
#-------------------------------------------------

QT       += core gui
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Dwin_Test_Tool
TEMPLATE = app


SOURCES += main.cpp\
        dwintest.cpp \
    qextserialbase.cpp \
    qextserialport.cpp \
    win_qextserialport.cpp \
    spindelegate.cpp

HEADERS  += dwintest.h \
    qextserialbase.h \
    qextserialport.h \
    win_qextserialport.h \
    spindelegate.h

FORMS    += dwintest.ui

RESOURCES += \
    images.qrc
