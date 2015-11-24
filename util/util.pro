#-------------------------------------------------
#
# Project created by QtCreator 2015-08-20T11:04:10
#
#-------------------------------------------------

QT       -= core gui

TARGET = ../lib/util
TEMPLATE = lib

DEFINES += UTIL_LIBRARY

SOURCES += \
    state_machine.cpp \
    sobject.cpp \
    timeout_provider.cpp \
    exception.cpp \
    ssingleton.cpp \
    message_router.cpp \
    command_string.cpp \
    mini_list.cpp \
    dbg.cpp \
    critical_section.cpp \
    sip_uri.cpp \
    cache_item.cpp \
    file_system.cpp \
    circular_buffer.cpp \
    file_system_utils.cpp \
    text_ui.cpp \
    vmd5.cpp \
    istring.cpp \
    rollover_prone_to_monotonic.cpp \
    string_utils.cpp \
    timestamp.cpp \
    cond_var_posix.cpp \
    library_posix.cpp \
    my_assert.cpp \
    my_error.cpp \
    mutex_posix.cpp \
    my_semaphore_posix.cpp \
    my_time_posix.cpp \
    splugin_posix.cpp \
    xml_parser.cpp \
    thread_posix.cpp

HEADERS += \
    state_machine.h \
    sobject.h \
    timeout_provider.h \
    thread.h \
    exception.h \
    ssingleton.h \
    message_router.h \
    command_string.h \
    mini_list.h \
    util_config.h \
    my_assert.h \
    dbg.h \
    mutex.h \
    critical_section.h \
    sip_uri.h \
    cond_var.h \
    my_semaphore.h \
    library.h \
    splugin.h \
    cache_item.h \
    my_time.h \
    file_system.h \
    circular_buffer.h \
    file_system_utils.h \
    text_ui.h \
    vmd5.h \
    istring.h \
    rollover_prone_to_monotonic.h \
    my_error.h \
    my_types.h \
    string_utils.h \
    timestamp.h \
    termmanip.h \
    util_defines.h \
    xml_parser.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
