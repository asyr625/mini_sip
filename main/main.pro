TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = ../bin/minisip

QMAKE_CXXFLAGS += -std=c++0x

INCLUDEPATH += $(TOOLKITS)/include
INCLUDEPATH += $(TOOLKITS)/include/SDL2
INCLUDEPATH += ../crypto
INCLUDEPATH += ../util
INCLUDEPATH += ../net_util
INCLUDEPATH += ../mikey
INCLUDEPATH += ../sip
INCLUDEPATH += ../sip/headers
INCLUDEPATH += ../mini_sip
INCLUDEPATH += ../mini_sip/media/video/display
INCLUDEPATH += ../mini_sip/media/video/codec
INCLUDEPATH += ../mini_sip/media
INCLUDEPATH += ../mini_sip/media/rtp
INCLUDEPATH += ../mini_sip/logging
INCLUDEPATH += ../mini_sip/ip_provider
INCLUDEPATH += ../mini_sip/signaling/sdp
INCLUDEPATH += ../mini_sip/signaling/sip
INCLUDEPATH += ../mini_sip/contacts
INCLUDEPATH += ../mini_sip/config

INCLUDEPATH += ../mini_sip/media/video
INCLUDEPATH += ../mini_sip/gui

SOURCES += main.cpp \
    text.cpp \
    statistics_text_plane.cpp \
    opengl_display.cpp \
    animate.cpp \
    opengl_window.cpp \
    video_cafe_gui/video_cafe_gui.cpp \
    video_cafe_gui/video_cafe_output.cpp \
    video_cafe_gui/video_cafe_main.cpp \
    h264/h264_encoder.cpp

HEADERS += \
    text.h \
    statistics_text_plane.h \
    opengl_window.h \
    opengl_display.h \
    animate.h \
    video_cafe_gui/video_cafe_gui.h \
    video_cafe_gui/video_cafe_output.h \
    h264/h264_encoder.h

