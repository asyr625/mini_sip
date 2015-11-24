#-------------------------------------------------
#
# Project created by QtCreator 2015-08-20T11:04:46
#
#-------------------------------------------------

QT       -= core gui

TARGET = ../lib/sip
TEMPLATE = lib

INCLUDEPATH += ../util
INCLUDEPATH += ../net_util
INCLUDEPATH += ../crypto

INCLUDEPATH += ./headers

DEFINES += SIP_LIBRARY

QMAKE_CXXFLAGS += -std=c++0x


SOURCES += \
    sip_message.cpp \
    sip_stack.cpp \
    sip_smcommand.cpp \
    sip_response.cpp \
    sip_socket_server.cpp \
    sip_authentication_digest.cpp \
    sip_dialog.cpp \
    sip_stack_internal.cpp \
    sip_exception.cpp \
    sip_timers.cpp \
    sip_message_content_factory.cpp \
    sip_command_dispatcher.cpp \
    sip_layer_dialog.cpp \
    sip_layer_transaction.cpp \
    sip_transaction.cpp \
    sip_layer_transport.cpp \
    sip_dialog_config.cpp \
    sip_transport.cpp \
    sip_transport_udp.cpp \
    sip_transport_tls_sctp.cpp \
    sip_transport_tls.cpp \
    sip_transport_tcp.cpp \
    sip_transport_sctp.cpp \
    sip_transport_dtls_udp.cpp \
    sip_transaction_invite_client.cpp \
    sip_transaction_invite_server.cpp \
    sip_transaction_non_invite_client.cpp \
    sip_transaction_non_invite_server.cpp \
    sip_transaction_invite_server_ua.cpp \
    sip_request.cpp \
    sip_transition_utils.cpp \
    sip_command_string.cpp \
    sip_dialog_register.cpp \
    headers/sip_header.cpp \
    headers/sip_header_accept.cpp \
    headers/sip_header_string.cpp \
    headers/sip_header_accept_contact.cpp \
    headers/sip_header_allow.cpp \
    headers/sip_header_allow_events.cpp \
    headers/sip_header_authorization.cpp \
    headers/sip_header_proxy_authenticate.cpp \
    headers/sip_header_authentication_info.cpp \
    headers/sip_header_proxy_authorization.cpp \
    headers/sip_header_content_type.cpp \
    headers/sip_header_call_id.cpp \
    headers/sip_header_cseq.cpp \
    headers/sip_header_contact.cpp \
    headers/sip_header_content_length.cpp \
    headers/sip_header_event.cpp \
    headers/sip_header_expires.cpp \
    headers/sip_header_from.cpp \
    headers/sip_header_record_route.cpp \
    headers/sip_header_require.cpp \
    headers/sip_header_route.cpp \
    headers/sip_header_rack.cpp \
    headers/sip_header_rseq.cpp \
    headers/sip_header_subject.cpp \
    headers/sip_header_subscription_state.cpp \
    headers/sip_header_supported.cpp \
    headers/sip_header_snake_sm.cpp \
    headers/sip_header_session_expires.cpp \
    headers/sip_header_www_authenticate.cpp \
    headers/sip_header_unknown.cpp \
    headers/sip_header_unsupported.cpp \
    headers/sip_header_user_agent.cpp \
    headers/sip_header_to.cpp \
    headers/sip_header_via.cpp \
    headers/sip_header_warning.cpp \
    headers/sip_header_refer_to.cpp \
    headers/sip_header_max_forwards.cpp \
    sip_message_content_im.cpp \
    sip_message_content_mime.cpp \
    sip_message_content_rcl.cpp \
    sip_message_content_unknown.cpp \
    sip_dialog_management.cpp \
    sip_utils.cpp

HEADERS += \
    sip_message_content.h \
    sip_message.h \
    sip_stack.h \
    sip_smcommand.h \
    sip_response.h \
    sip_socket_server.h \
    sip_authentication_digest.h \
    sip_dialog.h \
    sip_stack_internal.h \
    sip_exception.h \
    sip_timers.h \
    sip_message_content_factory.h \
    sip_command_dispatcher.h \
    sip_layer_dialog.h \
    sip_layer_transaction.h \
    sip_transaction.h \
    sip_layer_transport.h \
    sip_dialog_config.h \
    sip_transport.h \
    sip_transport_udp.h \
    sip_transport_tls_sctp.h \
    sip_transport_tls.h \
    sip_transport_tcp.h \
    sip_transport_sctp.h \
    sip_transport_dtls_udp.h \
    sip_transaction_invite_client.h \
    sip_transaction_invite_server.h \
    sip_transaction_non_invite_client.h \
    sip_transaction_non_invite_server.h \
    sip_transaction_invite_server_ua.h \
    sip_request.h \
    sip_transition_utils.h \
    sip_command_string.h \
    sip_dialog_register.h \
    headers/sip_header.h \
    headers/sip_header_accept.h \
    headers/sip_header_string.h \
    headers/sip_header_accept_contact.h \
    headers/sip_header_allow.h \
    headers/sip_header_allow_events.h \
    headers/sip_header_authorization.h \
    headers/sip_header_proxy_authenticate.h \
    headers/sip_header_authentication_info.h \
    headers/sip_header_proxy_authorization.h \
    headers/sip_header_content_type.h \
    headers/sip_header_call_id.h \
    headers/sip_header_cseq.h \
    headers/sip_header_contact.h \
    headers/sip_header_content_length.h \
    headers/sip_header_event.h \
    headers/sip_header_expires.h \
    headers/sip_header_from.h \
    headers/sip_header_record_route.h \
    headers/sip_header_require.h \
    headers/sip_header_route.h \
    headers/sip_header_rack.h \
    headers/sip_header_rseq.h \
    headers/sip_header_subject.h \
    headers/sip_header_subscription_state.h \
    headers/sip_header_supported.h \
    headers/sip_header_snake_sm.h \
    headers/sip_header_session_expires.h \
    headers/sip_header_www_authenticate.h \
    headers/sip_header_unknown.h \
    headers/sip_header_unsupported.h \
    headers/sip_header_user_agent.h \
    headers/sip_header_to.h \
    headers/sip_header_via.h \
    headers/sip_header_warning.h \
    headers/sip_header_refer_to.h \
    headers/sip_header_max_forwards.h \
    sip_message_content_im.h \
    sip_message_content_mime.h \
    sip_message_content_rcl.h \
    sip_message_content_unknown.h \
    sip_dialog_management.h \
    sip_utils.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
