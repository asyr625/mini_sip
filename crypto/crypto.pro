#-------------------------------------------------
#
# Project created by QtCreator 2015-08-29T13:10:08
#
#-------------------------------------------------

QT       -= core gui

TARGET = ../lib/crypto
TEMPLATE = lib

DEFINES += CRYPTO_LIBRARY

INCLUDEPATH += /usr/include
INCLUDEPATH += ../util
INCLUDEPATH += ../net_util

SOURCES += \
    cert.cpp \
    #openssl/tls_socket.cpp \
    #openssl/tls_server_socket.cpp \
    #openssl/tls_exception.cpp \
    aes.cpp \
    certificate_finder.cpp \
    certificate_path_finder_ucd.cpp \
    dtls_socket.cpp \
    gdenum.cpp \
    oakleydh.cpp \
    sip_sim.cpp \
    sip_simdh.cpp \
    sip_simpk.cpp \
    sip_sim_smart_cardgd.cpp \
    sip_sim_soft.cpp \
    smart_card.cpp \
    smart_card_exception.cpp \
    tls_server_socket.cpp \
    tls_socket.cpp \
    #tls_srp_socket.cpp \
    uuid.cpp \
    zrtpdh.cpp \
    hmac.cpp \
    base64.cpp \
    hmac256.cpp \
    rand.cpp \
    sha1.cpp \
    sha256.cpp \
    tls_exception.cpp \
    cert_cache_manager.cpp \
    uuid_t.cpp \
    sysdep.cpp

HEADERS += \
    cert.h \
    #openssl/tls_socket.h \
    #openssl/tls_server_socket.h \
    #openssl/tls_exception.h \
    aes.h \
    certificate_finder.h \
    certificate_path_finder_ucd.h \
    dtls_socket.h \
    gdenum.h \
    oakleydh.h \
    sip_sim.h \
    sip_simdh.h \
    sip_simpk.h \
    sip_sim_smart_cardgd.h \
    sip_sim_soft.h \
    smart_card.h \
    smart_card_exception.h \
    tls_server_socket.h \
    tls_socket.h \
    #tls_srp_socket.h \
    uuid.h \
    zrtpdh.h \
    hmac.h \
    base64.h \
    hmac256.h \
    rand.h \
    sha1.h \
    sha256.h \
    tls_exception.h \
    cert_cache_manager.h \
    uuid_t.h \
    sysdep.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
