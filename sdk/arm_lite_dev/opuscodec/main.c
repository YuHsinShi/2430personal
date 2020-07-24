#include <stdarg.h>
#include <string.h>
#include "ite/ith.h"

#include "arm_lite_dev/opuscodec/opuscodec.h"
#include "opus.h"

static void opus_create(void)
{
    int err = 0;
    OPUS_INIT_DATA *ptInitData = (OPUS_INIT_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t sampleRate = ptInitData->sampleRate;
    uint32_t application = ptInitData->application;
    uint32_t channels = ptInitData->channels;
    uint32_t mode = ptInitData->mode;

    if (mode == OPUS_ENCODE_INIT)
    {
        OpusEncoder *encoder = (OpusEncoder *) ptInitData->codecAddr;
        err = opus_encoder_init(encoder, sampleRate, channels, application);
        if (err < 0) {
            ptInitData->err = err;
        #ifdef CFG_CPU_WB
            ithFlushDCacheRange((void*)ptInitData, sizeof(OPUS_INIT_DATA));
            ithFlushMemBuffer();
        #endif
            return;
        }
#ifdef CFG_OPUS_DTX_ENABLE
        opus_encoder_ctl(encoder, OPUS_SET_DTX(1));
#endif
#ifdef CFG_OPUS_FEC_ENABLE
        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(22000));
        opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1));
        opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(25));
#endif
    }
    else if (mode == OPUS_DECODE_INIT)
    {
        OpusDecoder *decoder = (OpusDecoder *) ptInitData->codecAddr;
        err = opus_decoder_init(decoder, sampleRate, channels);
        if (err < 0) {
            ptInitData->err = err;
        #ifdef CFG_CPU_WB
            ithFlushDCacheRange((void*)ptInitData, sizeof(OPUS_INIT_DATA));
            ithFlushMemBuffer();
        #endif
        }
    }
}

static void opus_enc(void)
{
    OPUS_CONTEXT *ptOpusCtxt = (OPUS_CONTEXT*) CMD_DATA_BUFFER_OFFSET;
    int i;
    uint32_t ctxtLen = ptOpusCtxt->ctxtLen;
    uint16_t pcm_bytes[ctxtLen];
    uint8_t  cbits[ctxtLen];
    OpusEncoder *encoder = (OpusEncoder *) ptOpusCtxt->codecAddr;

    memcpy(pcm_bytes, ptOpusCtxt->ctxtBuffer, ctxtLen * sizeof(uint16_t));
    
    int nbBytes = opus_encode(encoder, pcm_bytes, ctxtLen, cbits, ctxtLen);
    if (nbBytes < 0) {
        ptOpusCtxt->err = nbBytes;
        return;
    }
    
    memcpy(ptOpusCtxt->ctxtBuffer, cbits, nbBytes);
    ptOpusCtxt->ctxtLen = nbBytes;
#ifdef CFG_CPU_WB
    ithFlushDCacheRange((void*)ptOpusCtxt, sizeof(OPUS_CONTEXT));
    ithFlushMemBuffer();
#endif
}

static void opus_dec(void)
{
    OPUS_CONTEXT *ptOpusCtxt = (OPUS_CONTEXT*) CMD_DATA_BUFFER_OFFSET;
    int i;
    uint32_t ctxtLen = ptOpusCtxt->ctxtLen;
    uint32_t frameSize = ptOpusCtxt->frameSize;
    int      fecDec =  ptOpusCtxt->fecDec;
    uint8_t  cbits[ctxtLen];
    uint16_t out[frameSize];
    OpusDecoder *decoder = (OpusDecoder *) ptOpusCtxt->codecAddr;
    
    memcpy(cbits, ptOpusCtxt->ctxtBuffer, ctxtLen);

    int frame_size = opus_decode(decoder, cbits, ctxtLen, out, frameSize, fecDec);
    if (frame_size < 0) {
        ptOpusCtxt->err = frame_size;
    #ifdef CFG_CPU_WB
        ithFlushDCacheRange((void*)ptOpusCtxt, sizeof(OPUS_CONTEXT));
        ithFlushMemBuffer();
    #endif
        return;
    }
    
    memcpy(ptOpusCtxt->ctxtBuffer, out, frame_size * sizeof(uint16_t));
    ptOpusCtxt->ctxtLen = frame_size;
#ifdef CFG_CPU_WB
    ithFlushDCacheRange((void*)ptOpusCtxt, sizeof(OPUS_CONTEXT));
    ithFlushMemBuffer();
#endif
}

int main(int argc, char **argv)
{
    int inputCmd = 0;

    while(1)
    {
        inputCmd = ARMLITE_COMMAND_REG_READ(REQUEST_CMD_REG);
        if (inputCmd && ARMLITE_COMMAND_REG_READ(RESPONSE_CMD_REG) == 0)
        {
#ifdef CFG_CPU_WB
            ithInvalidateDCacheRange((void*) CMD_DATA_BUFFER_OFFSET, MAX_CMD_DATA_BUFFER_SIZE);
#else
            ithInvalidateDCache();
#endif
            switch(inputCmd)
            {
                case OPUS_CREATE:
                    opus_create();
                    break;
                case OPUS_ENCODE:
                    opus_enc();
                    break;
                case OPUS_DECODE:
                    opus_dec();
                    break;
                default:
                    break;
            }
            ARMLITE_COMMAND_REG_WRITE(RESPONSE_CMD_REG, (uint16_t) inputCmd);
        }
    }
    
    return 0;
}
