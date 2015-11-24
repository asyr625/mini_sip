#ifndef ILBC_CODEC_H
#define ILBC_CODEC_H

#include<string>

#include "codec.h"

#include "ilbc/iLBC_define.h"
#include "ilbc/iLBC_encode.h"
#include "ilbc/iLBC_decode.h"

class Ilbc_Codec_Instance: public Codec_Instance
{
public:
    Ilbc_Codec_Instance();

    /**
         * @returns Number of bytes in output buffer
         */
    virtual uint32_t encode(void *in_buf, int32_t in_buf_size, void *out_buf);

    /**
         *
         * @returns Number of frames in output buffer
         */
    virtual uint32_t decode(void *in_buf, int32_t in_buf_size, void *out_buf);

private:
    iLBC_Enc_Inst_t enc_inst;
    iLBC_Dec_Inst_t dec_inst;
};

class ILBCCodecDescription : public Audio_Codec_Description
{
    public:
        ILBCCodecDescription( SRef<Library *> lib );

        virtual SRef<Codec_Instance*> new_instance();

        /**
         * @return Requested sampling freq for the CODEC
         */
        virtual int32_t get_sampling_freq();

        /**
         * Time in milliseconds to put in each frame/packet. This is 30ms for the ILBC codec.
         */
        virtual int32_t get_sampling_size_ms();

        virtual int32_t get_input_nr_samples();

        virtual std::string get_codec_name();

        virtual std::string get_codec_description();

        virtual uint8_t get_sdp_media_type();

        virtual std::string get_sdp_media_attributes();

        virtual uint32_t get_version()const;
};

#endif // ILBC_CODEC_H
