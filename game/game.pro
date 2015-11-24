TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


#INCLUDEPATH += ../mini_sip
INCLUDEPATH += ./game_engine
INCLUDEPATH += ./game_util

SOURCES += main.cpp \
    game_engine/scene.cpp \
    game_engine/node.cpp \
    game_util/game_package.cpp \
    game_engine/text_node.cpp \
    game_engine/rectangle_node.cpp \
    game_engine/circle_node.cpp

HEADERS += \
    game_engine/scene.h \
    game_engine/node.h \
    game_util/game_package.h \
    game_engine/text_node.h \
    game_engine/rectangle_node.h \
    game_engine/color_rgba.h \
    game_engine/circle_node.h

