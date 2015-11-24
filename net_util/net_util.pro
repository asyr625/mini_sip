#-------------------------------------------------
#
# Project created by QtCreator 2015-08-21T10:12:11
#
#-------------------------------------------------

QT       -= core gui

TARGET = ../lib/net_util
TEMPLATE = lib

INCLUDEPATH += ./udns/source
INCLUDEPATH += ../util


DEFINES += NET_UTIL_LIBRARY

SOURCES += \
    socket.cpp \
    ipaddress.cpp \
    stream_socket.cpp \
    tcp_socket.cpp \
    udp_socket.cpp \
    datagram_socket.cpp \
    sctp_socket.cpp \
    socket_server.cpp \
    server_socket.cpp \
    tcp_server_socket.cpp \
    sctp_server_socket.cpp \
    network_exception.cpp \
    network_functions.cpp \
    ip4_server_socket.cpp \
    ip6_server_socket.cpp \
    ip6address.cpp \
    ip4address.cpp \
    directory_set_item.cpp \
    directory_set.cpp \
    dns_naptr_query.cpp \
    downloader.cpp \
    file_url.cpp \
    file_downloader.cpp \
    file_downloader_exception.cpp \
    http_downloader.cpp \
    http_file_system.cpp \
    ldap_url.cpp \
    ldap_exception.cpp \
    ldap_entry.cpp \
    ldap_credentials.cpp \
    ldap_directory_locator.cpp \
    ldap_connection.cpp \
    ldap_downloader.cpp
    #inet_aton.cpp \
    #inet_ntop.cpp \
    #inet_pton.cpp

HEADERS += \
    socket.h \
    ipaddress.h \
    stream_socket.h \
    tcp_socket.h \
    udp_socket.h \
    datagram_socket.h \
    sctp_socket.h \
    socket_server.h \
    server_socket.h \
    tcp_server_socket.h \
    sctp_server_socket.h \
    network_exception.h \
    network_functions.h \
    ip4_server_socket.h \
    ip6_server_socket.h \
    ip6address.h \
    ip4address.h \
    directory_set_item.h \
    directory_set.h \
    dns_naptr_query.h \
    downloader.h \
    file_url.h \
    file_downloader.h \
    file_downloader_exception.h \
    http_downloader.h \
    http_file_system.h \
    ldap_url.h \
    ldap_exception.h \
    ldap_entry.h \
    ldap_credentials.h \
    ldap_directory_locator.h \
    ldap_connection.h \
    ldap_downloader.h \
    inet_aton.h \
    inet_ntop.h \
    inet_pton.h \
    net_config.h \
    my_defines.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
