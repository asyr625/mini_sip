#-------------------------------------------------
#
# Project created by QtCreator 2015-09-11T16:38:18
#
#-------------------------------------------------

QT       -= core gui

TARGET = ../lib/stun
TEMPLATE = lib

DEFINES += STUN_LIBRARY

INCLUDEPATH += ../net_util
INCLUDEPATH += ../util

SOURCES += stun.cpp \
    stun_attribute.cpp \
    stun_message.cpp \
    stun_test.cpp

HEADERS += stun.h\
    stun_attribute.h \
    stun_message.h \
    stun_test.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
