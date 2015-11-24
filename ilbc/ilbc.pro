#-------------------------------------------------
#
# Project created by QtCreator 2015-09-11T16:42:07
#
#-------------------------------------------------

QT       -= core gui

TARGET = ilbc
TEMPLATE = lib

INCLUDEPATH += ./ilbc
INCLUDEPATH += ../util
INCLUDEPATH += ../mini_sip/media
INCLUDEPATH += ../mini_sip/media/codec

SOURCES += ilbc_codec.cpp \
    ilbc/anaFilter.cxx \
    ilbc/constants.cxx \
    ilbc/createCB.cxx \
    ilbc/doCPLC.cxx \
    ilbc/enhancer.cxx \
    ilbc/filter.cxx \
    ilbc/FrameFlassify.cxx \
    ilbc/gainquant.cxx \
    ilbc/getCBvec.cxx \
    ilbc/helpfun.cxx \
    ilbc/hpInput.cxx \
    ilbc/hpOutput.cxx \
    ilbc/iCBConstruct.cxx \
    ilbc/iCBSearch.cxx \
    ilbc/iLBC_decode.cxx \
    ilbc/iLBC_encode.cxx \
    ilbc/LPCdecode.cxx \
    ilbc/LPCencode.cxx \
    ilbc/lsf.cxx \
    ilbc/packing.cxx \
    ilbc/StateConstructW.cxx \
    ilbc/StateSearchW.cxx \
    ilbc/syntFilter.cxx

HEADERS += ilbc_codec.h \
    ilbc/anaFilter.h \
    ilbc/constants.h \
    ilbc/createCB.h \
    ilbc/doCPLC.h \
    ilbc/enhancer.h \
    ilbc/filter.h \
    ilbc/FrameClassify.h \
    ilbc/gainquant.h \
    ilbc/getCBvec.h \
    ilbc/helpfun.h \
    ilbc/hpInput.h \
    ilbc/hpOutput.h \
    ilbc/iCBConstruct.h \
    ilbc/iCBSearch.h \
    ilbc/iLBC_decode.h \
    ilbc/iLBC_define.h \
    ilbc/iLBC_encode.h \
    ilbc/LPCdecode.h \
    ilbc/LPCencode.h \
    ilbc/lsf.h \
    ilbc/packing.h \
    ilbc/StateConstructW.h \
    ilbc/StateSearchW.h \
    ilbc/syntFilter.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
