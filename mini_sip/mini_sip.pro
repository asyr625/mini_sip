#-------------------------------------------------
#
# Project created by QtCreator 2015-08-31T14:40:23
#
#-------------------------------------------------

QT       -= core gui

TARGET = ../lib/mini_sip
TEMPLATE = lib

INCLUDEPATH += $(TOOLKITS)/include

INCLUDEPATH += ../util
INCLUDEPATH += ../net_util
INCLUDEPATH += ../mikey
INCLUDEPATH += ../sip
INCLUDEPATH += ../sip/headers
INCLUDEPATH += ../crypto
INCLUDEPATH += ../stun
INCLUDEPATH += ../libzrtpcpp/src
INCLUDEPATH += ../libzrtpcpp/src/libzrtpcpp
#INCLUDEPATH += ../libzrtpcpp/src/libzrtpcpp/crypt/openssl


INCLUDEPATH += ./contacts
INCLUDEPATH += ./ip_provider
INCLUDEPATH += ./logging
INCLUDEPATH += ./gui
INCLUDEPATH += ./config

INCLUDEPATH += ./media
INCLUDEPATH += ./media/aec
INCLUDEPATH += ./media/codec
INCLUDEPATH += ./media/rtp
INCLUDEPATH += ./media/sound_card
INCLUDEPATH += ./media/spaudio
INCLUDEPATH += ./media/visca
INCLUDEPATH += ./media/vnc
INCLUDEPATH += ./media/zrtp
INCLUDEPATH += ./media/video
INCLUDEPATH += ./media/video/codec
INCLUDEPATH += ./media/video/display
INCLUDEPATH += ./media/video/grabber

INCLUDEPATH += ./signaling/sdp
INCLUDEPATH += ./signaling/sip



DEFINES += MINI_SIP_LIBRARY

SOURCES += mini_sip.cpp \
    media/media.cpp \
    media/realtime_media.cpp \
    media/codec/codec.cpp \
    media/session.cpp \
    media/session_registry.cpp \
    media/media_handler.cpp \
    media/audio_media.cpp \
    media/stream_player.cpp \
    media/subsystem_media.cpp \
    ip_provider/ip_provider.cpp \
    ip_provider/simple_ip_provider.cpp \
    ip_provider/stun_ip_provider.cpp \
    media/streams_player.cpp \
    media/audio_stream_player.cpp \
    media/media_processor.cpp \
    media/media_command_string.cpp \
    media/media_stream.cpp \
    signaling/sip/sip.cpp \
    signaling/sdp/sdp_header.cpp \
    signaling/sip/presence_message_content.cpp \
    signaling/sip/sip_dialog_presence_client.cpp \
    signaling/sip/sip_dialog_presence_server.cpp \
    signaling/sip/video_fast_update_message_content.cpp \
    signaling/sip/default_dialog_handler.cpp \
    signaling/sdp/sdp_headera.cpp \
    signaling/sdp/sdp_headerb.cpp \
    signaling/sdp/sdp_headerc.cpp \
    signaling/sdp/sdp_headeri.cpp \
    signaling/sdp/sdp_headero.cpp \
    signaling/sdp/sdp_headers.cpp \
    signaling/sdp/sdp_headert.cpp \
    signaling/sdp/sdp_headerv.cpp \
    signaling/sdp/sdp_headerm.cpp \
    signaling/sdp/sdp_packet.cpp \
    signaling/sip/sip_dialog_voip.cpp \
    signaling/sip/sip_configuration.cpp \
    contacts/phone_book.cpp \
    contacts/contact_db.cpp \
    gui/bell.cpp \
    gui/gui.cpp \
    gui/log_entry.cpp \
    gui/console_debugger.cpp \
    media/video/codec/video_codec.cpp \
    media/video/grabber/grabber.cpp \
    media/video/display/video_display.cpp \
    media/video/codec/avdecoder.cpp \
    media/call_recorder.cpp \
    media/dtmf_sender.cpp \
    media/rtp_receiver.cpp \
    media/reliable_media_server.cpp \
    media/reliable_media.cpp \
    media/audio_plugin.cpp \
    media/spaudio/sp_audio.cpp \
    logging/logger.cpp \
    logging/logging_manager.cpp \
    media/rtp/rtp_stream.cpp \
    media/rtp/rtp_header.cpp \
    media/rtp/rtp_packet.cpp \
    media/rtp/rtcp_mgr.cpp \
    media/rtp/rtcp_packet.cpp \
    media/rtp/srtp_packet.cpp \
    media/rtp/crypto_context.cpp \
    media/video/image_handler.cpp \
    media/video/threaded_frame_converter.cpp \
    media/video/video_exception.cpp \
    media/video/video_media.cpp \
    media/video/video_stream_player.cpp \
    signaling/sip/sip_dialog_voip_client.cpp \
    signaling/sip/sip_dialog_voip_server.cpp \
    media/rtp/rtcp_report.cpp \
    media/rtp/rtcp_report_app_camctrl.cpp \
    media/rtp/rtcp_report_app_face.cpp \
    media/rtp/rtcp_report_app_view.cpp \
    media/rtp/rtcp_report_fir.cpp \
    media/rtp/rtcp_report_reception_block.cpp \
    media/rtp/rtcp_report_rr.cpp \
    media/rtp/sdes_item.cpp \
    media/rtp/sdes_chunk.cpp \
    media/rtp/sdes_cname.cpp \
    media/rtp/sdes_email.cpp \
    media/rtp/sdes_loc.cpp \
    media/rtp/sdes_name.cpp \
    media/rtp/rtcp_report_sdes.cpp \
    media/rtp/rtcp_report_sr.cpp \
    media/rtp/sdes_note.cpp \
    media/rtp/sdes_phone.cpp \
    media/rtp/sdes_tool.cpp \
    media/rtp/sdes_unknow.cpp \
    media/rtp/rtcp_report_sender_info.cpp \
    media/rtp/xr_report_block.cpp \
    media/rtp/rtcp_report_xr.cpp \
    media/rtp/xr_voip_report_block.cpp \
    media/sound_card/resampler.cpp \
    media/sound_card/sound_io_plc_interface.cpp \
    media/sound_card/sound_source.cpp \
    media/sound_card/sound_io.cpp \
    media/sound_card/sound_driver.cpp \
    media/sound_card/sound_device.cpp \
    media/sound_card/audio_mixer_simple.cpp \
    media/sound_card/audio_mixer.cpp \
    media/sound_card/audio_mixer_spatial.cpp \
    media/sound_card/file_sound_device.cpp \
    media/sound_card/file_sound_source.cpp \
    media/sound_card/silence_sensor.cpp \
    media/sound_card/sound_driver_registry.cpp \
    media/sound_card/sound_recorder_callback.cpp \
    media/visca/visca_ctrl.cpp \
    media/vnc/media_shared_workspace_plugin.cpp \
    media/vnc/media_shared_workspace.cpp \
    media/sound_card/simple_resampler.cpp \
    #media/sound_card/float_resampler.cpp \
    media/sound_card/alsa_card.cpp \
    media/sound_card/alsa_sound_device.cpp \
    media/sound_card/alsa_sound_driver.cpp \
    media/sound_card/direct_sound_driver.cpp \
    media/sound_card/file_sound_driver.cpp \
    media/sound_card/oss_sound_driver.cpp \
    media/sound_card/port_audio_driver.cpp \
    media/sound_card/oss_sound_device.cpp \
    #media/sound_card/direct_sound_device.cpp \
    #media/video/display/file_display.cpp \
    media/video/grabber/decklinksdk/DeckLinkAPIDispatch.cxx \
    media/video/grabber/yuv_file_grabber.cpp \
    media/video/grabber/v4l2_grabber.cpp \
    #media/video/grabber/matrox_grabber.cpp \
    #media/video/grabber/dc1394_grabber.cpp \
    media/video/grabber/deck_link_grabber.cpp \
    media/codec/g711_codec.cpp \
    media/codec/gsm_codec.cpp \
    media/codec/speex_codec.cpp \
    media/zrtp/zrtp_host_bridge_minisip.cpp \
    media/video/grabber/yuv2rgb.cpp \
    media/video/video_plugin.cpp \
    ip_provider/simple_ip6_provider.cpp \
    config/conf_backend.cpp \
    config/online_conf_back.cpp \
    config/user_config.cpp \
    config/dir_cache_manager.cpp \
    config/xml_config_plugin.cpp \
    config/gconf_backend.cpp \
    contacts/xml_phone_book_io.cpp \
    contacts/ldap_phone_book_io.cpp \
    contacts/online_xml_phone_book_io.cpp \
    media/codec/g711/codec_g711.cxx \
    media/video/display/file_display.cpp \
    media/sound_card/float_resampler.cpp

HEADERS += mini_sip.h\
    media/media.h \
    media/realtime_media.h \
    media/codec/codec.h \
    media/session.h \
    media/session_registry.h \
    media/media_handler.h \
    media/audio_media.h \
    media/stream_player.h \
    media/subsystem_media.h \
    ip_provider/ip_provider.h \
    ip_provider/simple_ip_provider.h \
    ip_provider/stun_ip_provider.h \
    media/streams_player.h \
    media/audio_stream_player.h \
    media/media_processor.h \
    media/media_command_string.h \
    signaling/sip/sip.h \
    signaling/sdp/sdp_header.h \
    signaling/sip/presence_message_content.h \
    signaling/sip/sip_dialog_presence_client.h \
    signaling/sip/sip_dialog_presence_server.h \
    signaling/sip/video_fast_update_message_content.h \
    signaling/sip/default_dialog_handler.h \
    signaling/sdp/sdp_headera.h \
    signaling/sdp/sdp_headerb.h \
    signaling/sdp/sdp_headerc.h \
    signaling/sdp/sdp_headeri.h \
    signaling/sdp/sdp_headero.h \
    signaling/sdp/sdp_headers.h \
    signaling/sdp/sdp_headert.h \
    signaling/sdp/sdp_headerv.h \
    signaling/sdp/sdp_headerm.h \
    signaling/sdp/sdp_packet.h \
    signaling/sip/sip_dialog_voip.h \
    signaling/sip/sip_configuration.h \
    contacts/phone_book.h \
    contacts/contact_db.h \
    gui/bell.h \
    gui/gui.h \
    gui/log_entry.h \
    gui/console_debugger.h \
    media/aec/aec.h \
    media/aec/aecfix.h \
    media/video/codec/video_codec.h \
    media/video/grabber/grabber.h \
    media/video/display/video_display.h \
    media/video/codec/avdecoder.h \
    media/call_recorder.h \
    media/dtmf_sender.h \
    media/rtp_receiver.h \
    media/reliable_media_server.h \
    media/reliable_media.h \
    media/audio_plugin.h \
    media/spaudio/sp_audio.h \
    logging/logger.h \
    logging/logging_manager.h \
    signaling/sip/irequest_video_keyframe.h \
    media/rtp/rtp_stream.h \
    media/rtp/rtp_header.h \
    media/rtp/rtp_packet.h \
    media/rtp/rtcp_mgr.h \
    media/rtp/rtcp_packet.h \
    media/rtp/srtp_packet.h \
    media/rtp/crypto_context.h \
    media/video/image_handler.h \
    media/video/threaded_frame_converter.h \
    media/video/video_exception.h \
    media/video/video_media.h \
    media/video/video_stream_player.h \
    signaling/sip/sip_dialog_voip_client.h \
    signaling/sip/sip_dialog_voip_server.h \
    media/rtp/rtcp_report.h \
    media/rtp/rtcp_report_app_camctrl.h \
    media/rtp/rtcp_report_app_face.h \
    media/rtp/rtcp_report_app_view.h \
    media/rtp/rtcp_report_fir.h \
    media/rtp/rtcp_report_reception_block.h \
    media/rtp/rtcp_report_rr.h \
    media/rtp/sdes_item.h \
    media/rtp/sdes_chunk.h \
    media/rtp/sdes_cname.h \
    media/rtp/sdes_email.h \
    media/rtp/sdes_loc.h \
    media/rtp/sdes_name.h \
    media/rtp/rtcp_report_sdes.h \
    media/rtp/rtcp_report_sr.h \
    media/rtp/sdes_note.h \
    media/rtp/sdes_phone.h \
    media/rtp/sdes_tool.h \
    media/rtp/sdes_unknow.h \
    media/rtp/rtcp_report_sender_info.h \
    media/rtp/xr_report_block.h \
    media/rtp/rtcp_report_xr.h \
    media/rtp/xr_voip_report_block.h \
    media/sound_card/resampler.h \
    media/sound_card/sound_io_plc_interface.h \
    media/sound_card/sound_source.h \
    media/sound_card/sound_io.h \
    media/sound_card/sound_driver.h \
    media/sound_card/sound_device.h \
    media/sound_card/audio_mixer_simple.h \
    media/sound_card/audio_mixer.h \
    media/sound_card/audio_mixer_spatial.h \
    media/sound_card/file_sound_device.h \
    media/sound_card/file_sound_source.h \
    media/sound_card/silence_sensor.h \
    media/sound_card/sound_driver_registry.h \
    media/sound_card/sound_recorder_callback.h \
    media/visca/visca_ctrl.h \
    media/vnc/media_shared_workspace.h \
    media/vnc/media_shared_workspace_plugin.h \
    media/sound_card/simple_resampler.h \
    #media/sound_card/float_resampler.h \
    media/sound_card/alsa_card.h \
    media/sound_card/alsa_sound_device.h \
    media/sound_card/alsa_sound_driver.h \
    media/sound_card/direct_sound_driver.h \
    media/sound_card/file_sound_driver.h \
    media/sound_card/oss_sound_driver.h \
    media/sound_card/port_audio_driver.h \
    media/sound_card/oss_sound_device.h \
    #media/sound_card/direct_sound_device.h \
    #media/video/display/file_display.h \
    media/video/grabber/decklinksdk/DeckLinkAPI.h \
    media/video/grabber/decklinksdk/DeckLinkAPI_v7_1.h \
    media/video/grabber/decklinksdk/DeckLinkAPI_v7_3.h \
    media/video/grabber/decklinksdk/LinuxCOM.h \
    media/video/grabber/yuv_file_grabber.h \
    media/video/grabber/v4l2_grabber.h \
    #media/video/grabber/matrox_grabber.h \
    #media/video/grabber/dc1394_grabber.h \
    media/video/grabber/deck_link_grabber.h \
    media/codec/g711_codec.h \
    media/codec/gsm_codec.h \
    media/codec/speex_codec.h \
    media/zrtp/zrtp_host_bridge_minisip.h \
    media/video/grabber/yuv2rgb.h \
    media/video/video_plugin.h \
    ip_provider/simple_ip6_provider.h \
    media/media_stream.h \
    config/conf_backend.h \
    config/online_conf_back.h \
    config/user_config.h \
    config/dir_cache_manager.h \
    media/audio_defines.h \
    mini_defines.h \
    config/xml_config_plugin.h \
    config/gconf_backend.h \
    contacts/xml_phone_book_io.h \
    contacts/ldap_phone_book_io.h \
    contacts/online_xml_phone_book_io.h \
    media/codec/g711/codec_g711.h \
    media/video/display/file_display.h \
    media/sound_card/float_resampler.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
