#-------------------------------------------------
#
# Project created by QtCreator 2015-10-15T16:48:51
#
#-------------------------------------------------

QT       -= gui

TARGET = ../lib/mikey
TEMPLATE = lib

INCLUDEPATH += ../util
INCLUDEPATH += ../crypto

SOURCES += mikey.cpp \
    key_agreement.cpp \
    key_agreement_dh.cpp \
    mikey_cs_id_map.cpp \
    mikey_exception.cpp \
    mikey_payload.cpp \
    mikey_payload_chash.cpp \
    mikey_payload_cert.cpp \
    mikey_payload_dh.cpp \
    mikey_payload_err.cpp \
    mikey_payload_general_extensions.cpp \
    mikey_payload_hdr.cpp \
    mikey_payload_id.cpp \
    mikey_payload_kemac.cpp \
    mikey_payload_key_data.cpp \
    mikey_payload_pke.cpp \
    mikey_payload_rand.cpp \
    mikey_payload_sign.cpp \
    mikey_payload_t.cpp \
    mikey_payload_v.cpp \
    mikey_payload_sp.cpp \
    mikey_message.cpp \
    key_validity.cpp \
    key_agreement_dhhmac.cpp \
    key_agreement_psk.cpp \
    key_agreement_pke.cpp \
    key_agreement_rsar.cpp \
    mikey_message_dh.cpp \
    mikey_message_dhhmac.cpp \
    mikey_message_rsar.cpp \
    mikey_message_psk.cpp \
    mikey_message_pke.cpp

HEADERS += mikey.h\
    key_agreement.h \
    key_agreement_dh.h \
    mikey_cs_id_map.h \
    mikey_exception.h \
    mikey_payload.h \
    mikey_payload_chash.h \
    mikey_payload_cert.h \
    mikey_payload_dh.h \
    mikey_payload_err.h \
    mikey_payload_general_extensions.h \
    mikey_payload_hdr.h \
    mikey_payload_id.h \
    mikey_payload_kemac.h \
    mikey_payload_key_data.h \
    mikey_payload_pke.h \
    mikey_payload_rand.h \
    mikey_payload_sign.h \
    mikey_payload_t.h \
    mikey_payload_v.h \
    mikey_payload_sp.h \
    mikey_defs.h \
    mikey_message.h \
    key_validity.h \
    key_agreement_dhhmac.h \
    key_agreement_psk.h \
    key_agreement_pke.h \
    key_agreement_rsar.h \
    mikey_message_dh.h \
    mikey_message_dhhmac.h \
    mikey_message_rsar.h \
    mikey_message_psk.h \
    mikey_message_pke.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
