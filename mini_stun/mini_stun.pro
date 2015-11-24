TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = ../bin/ministun

SOURCES += main.cpp

INCLUDEPATH += ../stun
INCLUDEPATH += ../net_util
INCLUDEPATH += ../util
