/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/mscodecutils.h"
#include "mediastreamer2/msticker.h"
#include "ortp/rtp.h"

#include "opus.h"
#ifdef CFG_RISC2_OPUS_CODEC
#include "risc2/opuscodec/opuscodec.h"
#endif

#define APPLICATION  OPUS_APPLICATION_VOIP
#define CHANNEL_MONO 1
#define SAMPLE_RATE  CFG_AUDIO_SAMPLING_RATE
#define FRAME_SIZE   (SAMPLE_RATE/50) //20ms

typedef struct _OpusEncData{
    MSBufferizer *bz;
    OpusEncoder *encoder;
} OpusEncData;

typedef struct _OpusDecData{
    OpusDecoder *decoder;
    /* concealment properties */
	MSConcealerContext *concealer;
	MSRtpPayloadPickerContext rtp_picker_context;
	int sequence_number;
	int lastPacketLength;
} OpusDecData;

static OpusEncData * opus_enc_data_new(){
    OpusEncData *obj=(OpusEncData *)ms_new(OpusEncData,1);
    obj->bz=ms_bufferizer_new();
    return obj;
}

static void opus_enc_init(MSFilter *obj){
    obj->data=opus_enc_data_new();
    OpusEncData *dt=(OpusEncData*)obj->data;
    int err = 0;
    dt->encoder = opus_encoder_create(SAMPLE_RATE, CHANNEL_MONO, APPLICATION, &err);
    if (!dt->encoder || err < 0) {
        printf("%s(%d) failed to create an encoder: %s\n", __FILE__, __LINE__, opus_strerror(err));
        if (!dt->encoder) {
            opus_encoder_destroy(dt->encoder);
        }
        return;
    }
#ifdef CFG_OPUS_DTX_ENABLE
    opus_encoder_ctl(dt->encoder, OPUS_SET_DTX(1));
#endif
#ifdef CFG_OPUS_FEC_ENABLE
    opus_encoder_ctl(dt->encoder, OPUS_SET_BITRATE(22000));
    opus_encoder_ctl(dt->encoder, OPUS_SET_INBAND_FEC(1));
    opus_encoder_ctl(dt->encoder, OPUS_SET_PACKET_LOSS_PERC(25));
#endif
}

static void opus_enc_uninit(MSFilter *obj){
    OpusEncData *dt=(OpusEncData*)obj->data;
    opus_encoder_destroy(dt->encoder);
    ms_bufferizer_destroy(dt->bz);
    ms_free(dt);
}

static void opus_enc_process(MSFilter *obj){
    mblk_t *m;
    OpusEncData *dt=(OpusEncData*)obj->data;
    MSBufferizer *bz=dt->bz;
    uint16_t buffer[FRAME_SIZE] = {0};

    while((m = ms_queue_get(obj->inputs[0]))!=NULL){
        ms_bufferizer_put(bz,m);
    }
    
    while (ms_bufferizer_read(bz, buffer, sizeof(buffer)) == sizeof(buffer)){
        mblk_t *o=allocb(FRAME_SIZE, 0);
        int len;
        len = opus_encode(dt->encoder, buffer, FRAME_SIZE, o->b_wptr, FRAME_SIZE);
        if (len < 0) {
            printf("%s(%d) encode failed: %s\n", __FILE__, __LINE__, opus_strerror(len));
            freemsg(o);
            break;
        }
        o->b_wptr+=len;
        ms_queue_put(obj->outputs[0],o);
    }
}

static int enc_add_fmtp(MSFilter *f, void *arg){
    //dummy
    return 0;
}

static int enc_add_attr(MSFilter *f, void *arg){
    //dummy
    return 0;
}

static MSFilterMethod opus_enc_methods[]={
    {   MS_FILTER_ADD_ATTR      ,   enc_add_attr},
    {   MS_FILTER_ADD_FMTP      ,   enc_add_fmtp},
    {   0               ,   NULL        }
};

#ifdef _MSC_VER

MSFilterDesc ms_opus_enc_desc={
    MS_OPUS_ENC_ID,
    "MSOpusEnc",
    N_("opus encoder"),
    MS_FILTER_ENCODER,
    "opus",
    1,
    1,
    opus_enc_init,
    NULL,
    opus_enc_process,
    NULL,
    opus_enc_uninit,
    opus_enc_methods
};

#else

MSFilterDesc ms_opus_enc_desc={
    .id=MS_OPUS_ENC_ID,
    .name="MSOpusEnc",
    .text=N_("opus encoder"),
    .category=MS_FILTER_ENCODER,
    .enc_fmt="opus",
    .ninputs=1,
    .noutputs=1,
    .init=opus_enc_init,
    .process=opus_enc_process,
    .uninit=opus_enc_uninit,
    .methods=opus_enc_methods
};

#endif

static OpusDecoder *ms_opus_decoder_create(opus_int32 Fs, int channels, int *error)
{
#ifdef CFG_RISC2_OPUS_CODEC
    int size = opus_decoder_get_size(channels);
    OpusDecoder *dec = malloc(size);
    OPUS_INIT_DATA tInitData = {0};
    tInitData.sampleRate = Fs;
    tInitData.channels = channels;
    tInitData.mode = OPUS_DECODE_INIT;
    tInitData.codecAddr = ((uint8_t *)dec - iteRiscGetTargetMemAddress(RISC1_IMAGE_MEM_TARGET));
    ioctl(ITP_DEVICE_RISC2, ITP_IOCTL_OPUS_CREATE, &tInitData);
    *error = tInitData.err;
    ithInvalidateDCacheRange((uint8_t *)dec, size);
    return dec;
#else
    return opus_decoder_create(Fs, channels, error);
#endif
}

static int ms_opus_decode(OpusDecoder *st, const unsigned char *data,
      opus_int32 len, opus_int16 *pcm, int frame_size, int decode_fec)
{
#ifdef CFG_RISC2_OPUS_CODEC
    OPUS_CONTEXT tOpusCtxt = {0};
    tOpusCtxt.frameSize = frame_size;
    tOpusCtxt.codecAddr = ((uint8_t *)st - iteRiscGetTargetMemAddress(RISC1_IMAGE_MEM_TARGET));
    tOpusCtxt.ctxtLen = len;
    tOpusCtxt.fecDec = decode_fec;
    memcpy(tOpusCtxt.ctxtBuffer, data, tOpusCtxt.ctxtLen);
    ioctl(ITP_DEVICE_RISC2, ITP_IOCTL_OPUS_DECODE, &tOpusCtxt);
    ithInvalidateDCacheRange((uint8_t *)st, opus_decoder_get_size(CHANNEL_MONO));
    if (tOpusCtxt.err < 0) {
        return tOpusCtxt.err;
    }
    memcpy(pcm, tOpusCtxt.ctxtBuffer, tOpusCtxt.ctxtLen * sizeof(uint16_t));
    return tOpusCtxt.ctxtLen;
#else
    return opus_decode(st, data, len, pcm, frame_size, decode_fec);
#endif
}

static OpusDecData * opus_dec_data_new(){
    OpusDecData *obj=(OpusDecData *)ms_new(OpusDecData,1);
    return obj;
}

static void opus_dec_init(MSFilter *obj){
    obj->data=opus_dec_data_new();
    OpusDecData *dt=(OpusDecData*)obj->data;

    int err = 0;
    dt->decoder = ms_opus_decoder_create(SAMPLE_RATE, CHANNEL_MONO, &err);
    if (!dt->decoder || err < 0) {
        printf("%s(%d) failed to create decoder: %s\n", __FILE__, __LINE__, opus_strerror(err));
        if (!dt->decoder) {
            opus_decoder_destroy(dt->decoder);
        }
    }
    dt->lastPacketLength = FRAME_SIZE*1000/SAMPLE_RATE;
    /* initialise the concealer context */
    dt->concealer = ms_concealer_context_new(UINT32_MAX);
}

static void opus_dec_uninit(MSFilter *obj){
    OpusDecData *dt=(OpusDecData*)obj->data;
    opus_decoder_destroy(dt->decoder);
    ms_concealer_context_destroy(dt->concealer);
    ms_free(dt);
}

static void opus_dec_process(MSFilter *obj){
#define MAX_FRAME_SIZE (SAMPLE_RATE*120/1000) //max decode frame 120ms
    OpusDecData *dt=(OpusDecData*)obj->data;
    mblk_t *m;
    mblk_t *o;
    int frame_len = 0;

    while((m=ms_queue_get(obj->inputs[0]))!=NULL){
        o=allocb(MAX_FRAME_SIZE * sizeof(uint16_t), 0);
        frame_len = ms_opus_decode(dt->decoder, (const unsigned char *)m->b_rptr, (opus_int32)(m->b_wptr - m->b_rptr), (opus_int16 *)o->b_wptr, MAX_FRAME_SIZE, 0);
        if (frame_len < 0) {
            printf("%s(%d) decoder failed: %s\n", __FILE__, __LINE__, opus_strerror(frame_len));
            freemsg(o);
        } else {
            dt->lastPacketLength = frame_len; // store the packet length for eventual PLC if next two packets are missing
            o->b_wptr += (frame_len * sizeof(uint16_t));
            ms_queue_put(obj->outputs[0],o);
            dt->sequence_number = mblk_get_cseq(m); // used to get eventual FEC information if next packet is missing
            ms_concealer_inc_sample_time(dt->concealer, obj->ticker->time, frame_len*1000/SAMPLE_RATE, 1);
        }
        freemsg(m);
    }

    /* Concealment if needed */
    if (ms_concealer_context_is_concealement_required(dt->concealer, obj->ticker->time)) {
		int imLength = 0;
		m = NULL;
		uint8_t *payload = NULL;
        o = allocb(MAX_FRAME_SIZE * sizeof(uint16_t), 0);
#ifdef CFG_OPUS_FEC_ENABLE
		// try fec : info are stored in the next packet
		if (dt->rtp_picker_context.picker) {
            /* FEC information is in the next packet, last valid packet was d->sequence_number, the missing one shall then be d->sequence_number+1, so check jitter buffer for d->sequence_number+2 */
			/* but we may have the n+1 packet in the buffer and adaptative jitter control keeping it for later, in that case, just go for PLC */
			if (dt->rtp_picker_context.picker(&dt->rtp_picker_context,dt->sequence_number+1) == NULL) { /* missing packet is really missing */
                m = dt->rtp_picker_context.picker(&dt->rtp_picker_context,dt->sequence_number+2); /* try to get n+2 */
				if (m) {
					imLength=rtp_get_payload(m,&payload);
				}
			}
		}
		/* call to the decoder, we'll have either FEC or PLC, do it on the same length that last received packet */
		if (payload) { // found frame to try FEC
			frame_len = ms_opus_decode(dt->decoder, payload, imLength, (opus_int16 *)o->b_wptr, dt->lastPacketLength, 1);
		} 
        else 
#endif
        { // do PLC
            frame_len = ms_opus_decode(dt->decoder, NULL, 0, (opus_int16 *)o->b_wptr, dt->lastPacketLength, 0);
		}
		if (frame_len < 0) {
            printf("%s(%d) decoder failed in concealment: %s\n", __FILE__, __LINE__, opus_strerror(frame_len));
			freemsg(o);
		} else {
			o->b_wptr += (frame_len * sizeof(uint16_t));
			mblk_set_plc_flag(o,TRUE);
			ms_queue_put(obj->outputs[0], o);
			dt->sequence_number++;
			ms_concealer_inc_sample_time(dt->concealer, obj->ticker->time, frame_len*1000/SAMPLE_RATE, 0);
		}
	}
}

static int ms_opus_dec_add_fmtp(MSFilter *f, void *arg){
    //dummy
    return 0;
}

static int ms_opus_set_rtp_picker(MSFilter *f, void *arg) {
    OpusDecData *d = (OpusDecData *)f->data;
	d->rtp_picker_context=*(MSRtpPayloadPickerContext*)arg;
	return 0;
}

static MSFilterMethod opus_dec_methods[] = {
    {	MS_FILTER_ADD_FMTP,	                ms_opus_dec_add_fmtp    },
#ifdef CFG_OPUS_FEC_ENABLE
	{	MS_FILTER_SET_RTP_PAYLOAD_PICKER,	ms_opus_set_rtp_picker	},
#endif
	{	0,				NULL				}
};

#ifdef _MSC_VER

MSFilterDesc ms_opus_dec_desc={
    MS_OPUS_DEC_ID,
    "MSOpusDec",
    N_("opus decoder"),
    MS_FILTER_DECODER,
    "opus",
    1,
    1,
    opus_dec_init,,
    NULL,
    opus_dec_process,
    NULL,
    opus_dec_uninit,
    opus_dec_methods
};

#else

MSFilterDesc ms_opus_dec_desc={
    .id=MS_OPUS_DEC_ID,
    .name="MSOpusDec",
    .text=N_("opus decoder"),
    .category=MS_FILTER_DECODER,
    .enc_fmt="opus",
    .ninputs=1,
    .noutputs=1,
    .init=opus_dec_init,
    .process=opus_dec_process,
    .uninit=opus_dec_uninit,
    .methods=opus_dec_methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_opus_dec_desc)
MS_FILTER_DESC_EXPORT(ms_opus_enc_desc)