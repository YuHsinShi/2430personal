#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msudp.h"
#include "mediastreamer2/msitc.h"
#include "mediastreamer2/mstee.h"
#include "mediastreamer2/msvideoout.h"
#include "mediastreamer2/msextdisplay.h"
#include "mediastreamer2/msvolume.h"
#include "mediastreamer2/msequalizer.h"
#include "mediastreamer2/mscodecutils.h"
#include "mediastreamer2/hwengine.h"
#include "mediastreamer2/msfilerec.h"
#include "mediastreamer2/msfileplayer.h"
#include "iniparser/dictionary.h"
#include "iniparser/iniparser.h"

#ifdef CFG_AUDIO_CODEC_G711_ULAW
    #define AUDIO_CODEC "PCMU"
#endif
#ifdef CFG_AUDIO_CODEC_G711_ALAW
    #define AUDIO_CODEC "PCMA"
#endif
#ifdef CFG_AUDIO_CODEC_OPUS
    #define AUDIO_CODEC "OPUS"
#endif

#ifdef ENABLE_AUDIO_ENGENEER_MODEL
#define SOUND "Esound:"
#else
#define SOUND "sound:"
#endif    

//#define PURE_WAV_RECORD
static void configure_itc(AudioStream *stream, LinphoneAudioStreamFlow select_flow);

static void audio_pure_wav_record_graph_link(MSConnectionHelper *h,AudioStream *stream){
    
    if (stream->a_recorder.tee) { // link pure audio record graph
        ms_connection_helper_link(h, stream->a_recorder.tee, 0, 0);
        ms_filter_link(stream->a_recorder.tee, 1, stream->a_recorder.itcsink, 0);
        ms_filter_link(stream->a_recorder.audio_input,0,stream->a_recorder.recorder,0);
        ms_filter_call_method(stream->a_recorder.itcsink,MS_ITC_SINK_CONNECT,stream->a_recorder.audio_input);            
    }    
}

static void audio_pure_wav_record_graph_unlink(MSConnectionHelper *h,AudioStream *stream){
    if(stream->a_recorder.tee!=NULL) {// unlink pure audio record graph
        MSRecorderState rstate;
        ms_connection_helper_unlink(h,stream->a_recorder.tee,0,0);
        ms_filter_unlink(stream->a_recorder.tee,1,stream->a_recorder.itcsink,0);
        ms_filter_unlink(stream->a_recorder.audio_input,0,stream->a_recorder.recorder,0);

        if (ms_filter_call_method(stream->a_recorder.recorder,MS_FILE_REC_GET_STATE,&rstate)==0){
            if (rstate!=MSRecorderClosed){
                ms_filter_call_method_noarg(stream->a_recorder.recorder, MS_FILE_REC_CLOSE);
            }
        }                              
        ms_filter_call_method(stream->a_recorder.itcsink,MS_ITC_SINK_CONNECT,NULL);
    }
}

static void audio_mkv_rec_graph_link(MSConnectionHelper *h,AudioStream *stream,LinphoneAudioStreamFlow select_flow){
    if (stream->teeforrecord) {
        int pin = 1;
        // [udprecv]--pin0--[teeforrecord]--pin0--
        //ms_filter_call_method(stream->teeforrecord,MS_TEE_MUTE,&pin);
        ms_connection_helper_link (h,stream->teeforrecord,0,0);
        // [udprecv]--pin0--[teeforrecord]--pin0--
        //                                --pin1--[itcsink]
        ms_filter_link(stream->teeforrecord,1,stream->itcsink,0);
        configure_itc(stream, select_flow);
    }
}

static void audio_mkv_rec_graph_unlink(MSConnectionHelper *h,AudioStream *stream){
    if (stream->teeforrecord!=NULL) {
        ms_connection_helper_unlink(h,stream->teeforrecord,0,0);
        ms_filter_unlink(stream->teeforrecord,1,stream->itcsink,0);
    }    
}

static void choose_display_name(VideoStream *stream){
    stream->display_name=ms_strdup(video_stream_get_default_video_renderer());
}

static void event_cb(void *ud, MSFilter* f, unsigned int event, void *eventdata){
    VideoStream *st=(VideoStream*)ud;
    if (st->eventcb!=NULL){
        st->eventcb(st->event_pointer,f,event,eventdata);
    }
}

static void video_stream_set_recorder_video_codec(VideoStream *stream, const char *mime) {
    MSVideoSize vsize;
    MSPinFormat pinFmt;

    if (!stream || !stream->av_recorder.recorder)
        return;
#ifdef CFG_WIN32_SIMULATOR
    MS_VIDEO_SIZE_ASSIGN(vsize, CIF);
#else
    MS_VIDEO_SIZE_ASSIGN(vsize, 720P);
#endif
    pinFmt.pin = 0;
    pinFmt.fmt = ms_factory_get_video_format(ms_factory_get_fallback(), mime, vsize, 25.00, NULL);
    ms_filter_call_method(stream->av_recorder.recorder, MS_FILTER_SET_INPUT_FMT, &pinFmt);
}

static void audio_stream_configure_resampler(MSFilter *resampler,MSFilter *from,MSFilter *to) {
    int from_rate=8000, to_rate=8000;
    //ms_filter_call_method(from,MS_FILTER_GET_SAMPLE_RATE,&from_rate);
    //ms_filter_call_method(to,MS_FILTER_GET_SAMPLE_RATE,&to_rate);
    ms_filter_call_method(resampler,MS_FILTER_SET_SAMPLE_RATE,&from_rate);
    ms_filter_call_method(resampler,MS_FILTER_SET_OUTPUT_SAMPLE_RATE,&to_rate);
    ms_message("configuring %s-->%s from rate[%i] to rate [%i]",
               from->desc->name, to->desc->name, from_rate,to_rate);
}

static int video_stream_udp_start_with_source (VideoStream *stream, const char *rem_ip, uint32_t rem_port,
	bool_t mobile_call, bool_t video_from_ipcam, MSWebCam* cam, MSFilter* source){
	MSPixFmt format;
	MSVideoSize disp_size;
	int tmp;
	JBParameters jbp;
	udp_config_t udp_conf;
	bool_t	  mobile_call_mode = mobile_call;

	if( source == NULL && stream->dir != VideoStreamRecvOnly){
		ms_error("mediatream.c:video no defined source");
		return -1;
	}

	if (stream->dir==VideoStreamSendRecv || stream->dir==VideoStreamSendOnly){
		/* hack for ITE */
		/* filters graph ***************************************
		 *
		 * castor3cam --> UdpSend
		 *
		 * *****************************************************/
		MSConnectionHelper ch;
		/*plumb the outgoing stream */
		printf("### %s:%d dir=%s\n", __FUNCTION__, __LINE__, stream->dir==VideoStreamSendRecv?"VideoStreamSendRecv":"VideoStreamSendOnly");
		/* to see which codec is really active */
		//printf("### ms_filter_create_encoder: %s\n", pt->mime_type);

		//stream->ms.encoder=ms_filter_create_encoder(pt->mime_type);
		//if (stream->ms.encoder==NULL){
			/* big problem: we don't have a registered codec for this payload...*/
			//ms_error("videostream.c: No encoder available for payload %i:%s.",payload,pt->mime_type);
			//return -1;
		//}
		/* creates the filters */
		stream->cam=cam;
		stream->source = source;
		printf("### %s:%d create cam \"%s\"\n", __FUNCTION__, __LINE__, stream->source->desc->name);
		
		if (mobile_call_mode){
			printf("set mobile call mode\n");
			ms_filter_call_method(stream->source,MS_FILTER_SET_MOBILE,&mobile_call_mode);
		}	

		stream->ms.udpsend = ms_filter_new (MS_UDP_SEND_ID);
		if(stream->ms.udpsend== NULL){
			ms_error("mediastream.c: No available filter for payload %s.","MS_UDP_SEND_ID");
			return -1;
 		}
		memset(&udp_conf,'\0',sizeof(udp_config_t));
		udp_conf.remote_port = rem_port;
		udp_conf.cur_socket = -1;
		udp_conf.c_type = VIDEO_OUTPUT;	
		memset(udp_conf.group_ip,'\0',16);
		memcpy(udp_conf.remote_ip, rem_ip, 16);
		ms_filter_call_method(stream->ms.udpsend,MS_UDP_SEND_SET_PARA,&udp_conf);
		//if (pt->normal_bitrate>0){
		//	  ms_message("Limiting bitrate of video encoder to %i bits/s",pt->normal_bitrate);
		//	  ms_filter_call_method(stream->ms.encoder,MS_FILTER_SET_BITRATE,&pt->normal_bitrate);
		//}
		//if (pt->send_fmtp){
		//	  printf("### %s:%d need send fmtp=\"%s\"\n", __FUNCTION__, __LINE__, pt->send_fmtp);
		//	  ms_filter_call_method(stream->ms.encoder,MS_FILTER_ADD_FMTP,pt->send_fmtp);
		//}

		//configure_video_source (stream);
		/* and then connect all */
		ms_filter_link(stream->source, 0, stream->ms.udpsend, 0);
	}
	if (stream->dir==VideoStreamSendRecv || stream->dir==VideoStreamRecvOnly){
		/* hack for iTE H.264 video stream */
		/* filters graph ***************************************
		 *
		 * UdpRecv --> H.264 decoder --> castor3display
		 *
		 * ****************************************************/
		MSConnectionHelper ch;
	
		printf("### %s:%d dir=%s\n", __FUNCTION__, __LINE__, stream->dir==VideoStreamSendRecv?"VideoStreamSendRecv":"VideoStreamRecvOnly");
		/*to see which codec is really active */
		printf("### ms_filter_create_decoder: %s\n", "H264");
	
		/* create decoder first */
		stream->ms.decoder = ms_filter_create_decoder("H264");
		if (stream->ms.decoder==NULL){
			/* big problem: we don't have a registered decoderfor this payload...*/
			ms_error("mediastream.c: No decoder available for payload:%s.","H264");
			return -1;
		}
		ms_filter_set_notify_callback(stream->ms.decoder, event_cb, stream);
	
		if(!video_from_ipcam)
		{
			stream->ms.udprecv = ms_filter_new (MS_UDP_RECV_ID);
			if(stream->ms.udprecv == NULL){
				ms_error("mediastream.c: No filter available for payload %s.","VIDEO MS_UDP_RECV_ID");
				return -1;
	  		}
			memset(&udp_conf,'\0',sizeof(udp_config_t));
			udp_conf.remote_port = rem_port;
			udp_conf.cur_socket = -1;
			udp_conf.c_type = VIDEO_INPUT;
		    ms_filter_call_method(stream->ms.udprecv,MS_UDP_RECV_SET_PARA,&udp_conf);
		}
		else
			stream->sourceforIPCamStreaming = ms_filter_new(MS_ITC_IPCAM_ID);
	
		/* need no JPEG writer, hence we need no tee filter */
		stream->jpegwriter=ms_filter_new(MS_JPEG_WRITER_ID);
		if (stream->jpegwriter){
			stream->tee2=ms_filter_new(MS_TEE_ID);
			stream->filewriter = ms_filter_new(MS_FILE_WRITER_ID);
			stream->itcsinkforfilewriter = ms_filter_new(MS_ITC_SINK_ID);
			stream->itcsourceforfilewriter = ms_filter_new(MS_ITC_SOURCE_ID);
		}	 	

        stream->output=ms_filter_new_from_name (stream->display_name);

        stream->av_recorder.recorder = ms_filter_new(MS_MKV_RECORDER_ID);
        if (stream->av_recorder.recorder)
        {
            stream->teeforrecord = ms_filter_new(MS_TEE_ID);
            stream->itcsink = ms_filter_new(MS_ITC_SINK_ID);
            stream->av_recorder.audio_input = ms_filter_new(MS_ITC_SOURCE_ID);
            stream->av_recorder.video_input = ms_filter_new(MS_ITC_SOURCE_ID);
        }
        /* and connect the filters */
		ms_connection_helper_start (&ch);
	
		// [udprecv]--pin0--
		if(!video_from_ipcam)
			ms_connection_helper_link (&ch,stream->ms.udprecv,-1,0);
		else
			ms_connection_helper_link (&ch,stream->sourceforIPCamStreaming,-1,0);

		if (stream->teeforrecord) {
            // [udprecv]--pin0--[teeforrecord]--pin0--
            ms_connection_helper_link (&ch,stream->teeforrecord,0,0);
            // [udprecv]--pin0--[teeforrecord]--pin0--
            //                                --pin1-- --pin0--[itcsink]
            ms_filter_link(stream->teeforrecord,1,stream->itcsink,0);

            // [itc_source/video]--pin0-- --pin1--[recorder]
            ms_filter_link(stream->av_recorder.video_input,0,stream->av_recorder.recorder,0);
            // [itc_source/audio]--pin0-- --pin1--[recorder]
            ms_filter_link(stream->av_recorder.audio_input,0,stream->av_recorder.recorder,1);
            ms_filter_call_method(stream->itcsink,MS_ITC_SINK_CONNECT,stream->av_recorder.video_input);
        }
		
		ms_connection_helper_link (&ch,stream->ms.decoder,0,0);
		if (stream->tee2){
			// [udprecv]--pin0--[decoder]--pin0--[tee2]--pin0--
			ms_connection_helper_link (&ch,stream->tee2,0,0);
			// [udprecv]--pin0--[decoder]--pin0--[tee2]--pin0--
			//										   --pin1--[jpegwriter]
			ms_filter_link(stream->tee2,1,stream->jpegwriter,0);
			ms_filter_link(stream->jpegwriter,0,stream->itcsinkforfilewriter,0);
			ms_filter_link(stream->itcsourceforfilewriter,0,stream->filewriter,0);
			ms_filter_call_method(stream->itcsinkforfilewriter,MS_ITC_SINK_CONNECT,stream->itcsourceforfilewriter);
	
		}
		ms_connection_helper_link (&ch,stream->output,0,-1);
		/* the video source must be send for preview , if it exists*/

		video_stream_set_recorder_video_codec(stream, "H264");
	}
	
	/* create the ticker */
	stream->ms.sessions.ticker = ms_ticker_new();
	ms_ticker_set_name(stream->ms.sessions.ticker,"Video MSTicker");
	if (stream->dir==VideoStreamSendRecv || stream->dir==VideoStreamRecvOnly){
		stream->av_recorder.ticker = ms_ticker_new();
    	ms_ticker_set_name(stream->av_recorder.ticker,"MKVRecoder MSTicker");
		stream->tickerforfilewriter =  ms_ticker_new();
		ms_ticker_set_name(stream->tickerforfilewriter,"FileWriter MSTicker");
	}
	
	/* attach the graphs */
	if (stream->source)
		ms_ticker_attach (stream->ms.sessions.ticker, stream->source);	  
	if (stream->ms.udprecv && !video_from_ipcam)
		ms_ticker_attach (stream->ms.sessions.ticker, stream->ms.udprecv);
	else if (stream->sourceforIPCamStreaming)
		ms_ticker_attach (stream->ms.sessions.ticker, stream->sourceforIPCamStreaming);
	if (stream->dir==VideoStreamSendRecv || stream->dir==VideoStreamRecvOnly){
    	if (stream->av_recorder.video_input)
        	ms_ticker_attach (stream->av_recorder.ticker, stream->av_recorder.video_input);
    	if (stream->itcsourceforfilewriter)
        	ms_ticker_attach (stream->tickerforfilewriter, stream->itcsourceforfilewriter);
	}
	return 0;
}

void video_stream_udp_free (VideoStream * stream) {
    if (stream->ms.udpsend != NULL)
        ms_filter_destroy (stream->ms.udpsend);
	if (stream->ms.udprecv!= NULL)
        ms_filter_destroy (stream->ms.udprecv);
	if (stream->ms.decoder!= NULL)
        ms_filter_destroy (stream->ms.decoder);

    if (stream->source != NULL)
        ms_filter_destroy (stream->source);
    if (stream->sourceforIPCamStreaming != NULL)
        ms_filter_destroy (stream->sourceforIPCamStreaming);
    if (stream->output != NULL)
        ms_filter_destroy (stream->output);
    if (stream->tee!=NULL)
        ms_filter_destroy(stream->tee);
    if (stream->tee2!=NULL)
        ms_filter_destroy(stream->tee2);
    if (stream->jpegwriter!=NULL)
        ms_filter_destroy(stream->jpegwriter);
    if (stream->filewriter!=NULL)
        ms_filter_destroy(stream->filewriter);
    if (stream->itcsinkforfilewriter!=NULL)
        ms_filter_destroy(stream->itcsinkforfilewriter);
    if (stream->itcsourceforfilewriter!=NULL)
        ms_filter_destroy(stream->itcsourceforfilewriter);
    if (stream->tickerforfilewriter != NULL)
        ms_ticker_destroy(stream->tickerforfilewriter);
    if (stream->output2!=NULL)
        ms_filter_destroy(stream->output2);
    if (stream->ms.sessions.ticker != NULL)
        ms_ticker_destroy (stream->ms.sessions.ticker);
    if (stream->display_name!=NULL)
        ms_free(stream->display_name);
    if (stream->ms.rc!=NULL){
        ms_bitrate_controller_destroy(stream->ms.rc);
    }
    if (stream->av_recorder.recorder!=NULL)
        ms_filter_destroy(stream->av_recorder.recorder);
    if (stream->av_recorder.audio_input!=NULL)
        ms_filter_destroy(stream->av_recorder.audio_input);
    if (stream->av_recorder.video_input!=NULL)
        ms_filter_destroy(stream->av_recorder.video_input);
    if (stream->av_recorder.ticker != NULL)
        ms_ticker_destroy (stream->av_recorder.ticker);
    if (stream->teeforrecord!=NULL)
        ms_filter_destroy(stream->teeforrecord);
    if (stream->itcsink!=NULL)
        ms_filter_destroy(stream->itcsink);

    ms_free (stream);
}

static void audio_stream_udp_free(AudioStream *stream) {
    if (stream->ms.udpsend!=NULL) ms_filter_destroy(stream->ms.udpsend);
    if (stream->ms.udprecv!=NULL) ms_filter_destroy(stream->ms.udprecv);
    if (stream->ms.encoder!=NULL) ms_filter_destroy(stream->ms.encoder);
    if (stream->ms.decoder!=NULL) ms_filter_destroy(stream->ms.decoder);
    if (stream->ms.qi) ms_quality_indicator_destroy(stream->ms.qi);
	
    if (stream->soundread!=NULL) ms_filter_destroy(stream->soundread);
    if (stream->soundwrite!=NULL) ms_filter_destroy(stream->soundwrite);
    if (stream->dtmfgen!=NULL) ms_filter_destroy(stream->dtmfgen);
    if (stream->ec!=NULL)   ms_filter_destroy(stream->ec);
	if (stream->mixvoice!=NULL) ms_filter_destroy(stream->mixvoice);
    if (stream->volrecv!=NULL) ms_filter_destroy(stream->volrecv);
    if (stream->volsend!=NULL) ms_filter_destroy(stream->volsend);
    if (stream->equalizerSPK!=NULL) ms_filter_destroy(stream->equalizerSPK);
    if (stream->equalizerMIC!=NULL) ms_filter_destroy(stream->equalizerMIC);
    if (stream->ms.sessions.ticker!=NULL) ms_ticker_destroy(stream->ms.sessions.ticker);
    if (stream->read_resampler!=NULL) ms_filter_destroy(stream->read_resampler);
    if (stream->write_resampler!=NULL) ms_filter_destroy(stream->write_resampler);
    if (stream->dtmfgen_rtp!=NULL) ms_filter_destroy(stream->dtmfgen_rtp);
    if (stream->ms.rc) ms_bitrate_controller_destroy(stream->ms.rc);
    if (stream->itcsink!=NULL) ms_filter_destroy(stream->itcsink);
    if (stream->teeforrecord!=NULL) ms_filter_destroy(stream->teeforrecord);
    if (stream->a_recorder.tee!=NULL) ms_filter_destroy(stream->a_recorder.tee);
    if (stream->a_recorder.itcsink!=NULL) ms_filter_destroy(stream->a_recorder.itcsink);
    if (stream->a_recorder.audio_input!=NULL) ms_filter_destroy(stream->a_recorder.audio_input);
    if (stream->a_recorder.recorder!=NULL) ms_filter_destroy(stream->a_recorder.recorder);
    if (stream->a_recorder.ticker!=NULL) ms_ticker_destroy(stream->a_recorder.ticker);
    ms_free(stream);
}

VideoStream *video_stream_udp_new(unsigned short loc_port, bool_t use_ipv6,bool_t call_mobile){
    VideoStream *stream = (VideoStream *)ms_new0 (VideoStream, 1);
    stream->ms.type = VideoStreamType;
	stream->ms.udp_port = loc_port;
    MS_VIDEO_SIZE_ASSIGN(stream->sent_vsize, CIF);
    stream->dir=VideoStreamSendRecv;
    choose_display_name(stream);
    return stream;
}

int video_stream_udp_start (VideoStream *stream, const char *rem_ip, int rem_port,
    bool_t mobile_call, bool_t video_from_ipcam, MSWebCam *cam){
    if (cam==NULL && stream->dir != VideoStreamRecvOnly){
        cam = ms_web_cam_manager_get_default_cam( ms_web_cam_manager_get() );
    }
    if(stream->dir == VideoStreamRecvOnly)
        return video_stream_udp_start_with_source(stream, rem_ip, rem_port, mobile_call, video_from_ipcam, NULL, NULL);
    else
        return video_stream_udp_start_with_source(stream, rem_ip, stream->ms.udp_port, mobile_call, video_from_ipcam, cam, ms_web_cam_create_reader(cam));
}

void
video_stream_udp_stop (VideoStream * stream)
{
	if (stream->ms.sessions.ticker){
        if (stream->source)
            ms_ticker_detach(stream->ms.sessions.ticker,stream->source);
        if (stream->ms.udprecv)
            ms_ticker_detach(stream->ms.sessions.ticker,stream->ms.udprecv);
        else if (stream->sourceforIPCamStreaming)
            ms_ticker_detach(stream->ms.sessions.ticker,stream->sourceforIPCamStreaming);
		if (stream->av_recorder.video_input)
            ms_ticker_detach(stream->av_recorder.ticker,stream->av_recorder.video_input);
        if (stream->itcsourceforfilewriter)
            ms_ticker_detach(stream->tickerforfilewriter, stream->itcsourceforfilewriter);

        if (stream->source){
            ms_filter_unlink(stream->source, 0, stream->ms.udpsend,0);
        }
        if (stream->ms.udprecv || stream->sourceforIPCamStreaming){
            MSConnectionHelper h;
            ms_connection_helper_start (&h);
            if(stream->ms.udprecv)
                ms_connection_helper_unlink (&h,stream->ms.udprecv,-1,0);
            else if(stream->sourceforIPCamStreaming)
                ms_connection_helper_unlink (&h,stream->sourceforIPCamStreaming,-1,0);
            if (stream->teeforrecord) {
                MSRecorderState rstate;
                ms_connection_helper_unlink(&h,stream->teeforrecord,0,0);
                ms_filter_unlink(stream->teeforrecord,1,stream->itcsink,0);

				ms_filter_unlink(stream->av_recorder.video_input,0,stream->av_recorder.recorder,0);
                ms_filter_unlink(stream->av_recorder.audio_input,0,stream->av_recorder.recorder,1);
                if (ms_filter_call_method(stream->av_recorder.recorder,MS_RECORDER_GET_STATE,&rstate)==0){
                    if (rstate!=MSRecorderClosed){
                        ms_filter_call_method_noarg(stream->av_recorder.recorder, MS_RECORDER_CLOSE);
                    }
                }	
                ms_filter_call_method(stream->itcsink,MS_ITC_SINK_CONNECT,NULL);
            }
            ms_connection_helper_unlink (&h,stream->ms.decoder,0,0);
            if (stream->tee2){
                ms_connection_helper_unlink (&h,stream->tee2,0,0);
                ms_filter_unlink(stream->tee2,1,stream->jpegwriter,0);
                ms_filter_unlink(stream->jpegwriter,0,stream->itcsinkforfilewriter,0);
                ms_filter_unlink(stream->itcsourceforfilewriter,0,stream->filewriter,0);

                ms_filter_call_method(stream->itcsinkforfilewriter,MS_ITC_SINK_CONNECT, NULL);

            }
            if(stream->output)
                ms_connection_helper_unlink(&h,stream->output,0,-1);
            if (stream->tee && stream->output2==NULL)
                ms_filter_unlink(stream->tee,1,stream->output,1);
        }
    }
    video_stream_udp_free (stream);
}

AudioStream *audio_stream_udp_new(int loc_port, bool_t ipv6)
{
	AudioStream *stream=(AudioStream *)ms_new0(AudioStream,1);
    MSFilterDesc *ec_desc=ms_filter_lookup_by_name("MSOslec");

    ms_filter_enable_statistics(TRUE);
    ms_filter_reset_statistics();

    stream->ms.type = AudioStreamType;
    /*some filters are created right now to allow configuration by the application before start() */
    //stream->ms.udpsend=ms_filter_new(MS_UDP_SEND_ID);

#ifdef VIDEO_ENABLED
    /*
     * In practice, these filters are needed only for audio+video recording.
     */
    if (ms_factory_lookup_filter_by_id(ms_factory_get_fallback(), MS_MKV_RECORDER_ID)){
        stream->itcsink=ms_filter_new(MS_ITC_SINK_ID);
        if (stream->itcsink)
            stream->teeforrecord=ms_filter_new(MS_TEE_ID);
    }
#endif
#ifdef PURE_WAV_RECORD
    stream->a_recorder.itcsink = ms_filter_new(MS_ITC_SINK_ID);
    if(stream->a_recorder.itcsink){
        int encode_type = 1;//HX : PCM wav file  
        stream->a_recorder.tee = ms_filter_new(MS_TEE_ID);
        stream->a_recorder.audio_input = ms_filter_new(MS_ITC_SOURCE_ID);
        stream->a_recorder.recorder = ms_filter_new(MS_FILE_REC_ID);
        ms_filter_call_method_noarg(stream->a_recorder.recorder,MS_FILE_REC_CLOSE);
        ms_filter_call_method(stream->a_recorder.recorder,MS_FILE_REC_SET_SPECIAL_CASE,&encode_type);
    }
#endif
	stream->ms.udp_port = loc_port;
    return stream;
}

static void configure_itc(AudioStream *stream, LinphoneAudioStreamFlow select_flow){
    if (stream->itcsink){
        MSPinFormat pinfmt={0};
        pinfmt.pin=0;
        if(select_flow == AudioFromUdpRecv)
            pinfmt.fmt=ms_factory_get_audio_format(ms_factory_get_fallback(),"PCMU", stream->audiorate, 1, NULL);
        if(select_flow == AudioFromUdpRecvSoundRead)
            pinfmt.fmt=ms_factory_get_audio_format(ms_factory_get_fallback(),"A_PCML", stream->audiorate, 1, NULL);    
        ms_filter_call_method(stream->itcsink,MS_FILTER_SET_INPUT_FMT,&pinfmt);
        
        ms_message("configure_itc(): format set to %s",ms_fmt_descriptor_to_string(pinfmt.fmt));
    }
}

static MSFilter *audio_streamer_fileplayer_filter(char *filename){
    int special_case= 0;
    int codec_type = 0;  
    int interval = 2000;
    MSFilter *filter;
    filter = ms_filter_new(MS_FILE_PLAYER_ID);
    if (filename){
        ms_filter_call_method(filter,MS_FILE_PLAYER_OPEN,(void*)filename);
        ms_filter_call_method(filter,MS_FILTER_GET_CODEC_TYPE,&codec_type);
        ms_filter_call_method(filter,MS_FILE_PLAYER_SET_SPECIAL_CASE,&special_case);
        ms_filter_call_method(filter,MS_FILE_PLAYER_LOOP,&interval);
        ms_filter_call_method_noarg(filter,MS_FILE_PLAYER_START);    
    }
    
    return filter;
}

int audio_stream_udp_start_full(AudioStream *stream, const char *rem_ip,int rem_port,
    MSSndCard *playcard, MSSndCard *captcard, bool_t use_ec, LinphoneAudioStreamFlow select_flow)
{
    PayloadType *pt;
    MSConnectionHelper h;
	int cur_socket = -1;
	udp_config_t udp_conf;
    int sample_rate = stream->audiorate;
    int tmp;
    
    if(stream->receive_graph){
        stream->ms.udprecv=ms_filter_new(MS_UDP_RECV_ID);
        if(stream->ms.udprecv == NULL){
            ms_error("mediastream_udp.c: No filter available for payload %s.","AUDIO MS_UDP_RECV_ID");
            return -1;
        }
        memset(&udp_conf,'\0',sizeof(udp_config_t));
        udp_conf.remote_port = rem_port;
        udp_conf.cur_socket = -1;
        udp_conf.c_type = AUDIO_INPUT;
        ms_filter_call_method(stream->ms.udprecv,MS_UDP_RECV_SET_PARA,&udp_conf);
	}

	if(stream->send_graph){
        stream->ms.udpsend=ms_filter_new(MS_UDP_SEND_ID);
        if(stream->ms.udpsend == NULL){
            ms_error("mediastream_udp.c: No filter available for payload %s.","AUDIO MS_UDP_SEND_ID");
            return -1;
        }        
		memset(&udp_conf,'\0',sizeof(udp_config_t));
		udp_conf.remote_port = stream->ms.udp_port;
		udp_conf.cur_socket = cur_socket;
		udp_conf.c_type = AUDIO_OUTPUT;	
		memset(udp_conf.group_ip,'\0',16);
		memcpy(udp_conf.remote_ip,rem_ip,16);
		ms_filter_call_method(stream->ms.udpsend,MS_UDP_SEND_SET_PARA,&udp_conf);
	}
    Castor3snd_reinit_for_diff_rate(sample_rate,16,1);//check if IIS need reinited or not (sampling rate ,bitsize,channel) 
    
    if(stream->send_graph && stream->filename){
        stream->soundread = audio_streamer_fileplayer_filter(stream->filename);//data from wav file;
        ms_filter_call_method(stream->soundread,MS_FILTER_GET_SAMPLE_RATE,&tmp);
        if ( tmp != sample_rate) {
            /* need to add resampler*/
            stream->read_resampler=ms_filter_new(MS_RESAMPLE_ID);
            ms_filter_call_method(stream->read_resampler,MS_FILTER_SET_SAMPLE_RATE,&tmp);//input
            ms_filter_call_method(stream->read_resampler,MS_FILTER_SET_OUTPUT_SAMPLE_RATE,&pt->clock_rate);//output
        } 
    }else if(captcard!=NULL && stream->send_graph && !stream->soundread){
        stream->soundread=ms_snd_card_create_reader(captcard);//data from mic
    }else{
        stream->soundread = ms_filter_new(MS_FILE_PLAYER_ID);
    }   
    
    if (playcard!=NULL && stream->receive_graph) stream->soundwrite=ms_snd_card_create_writer(playcard);
	
    if(stream->use_codec){
        stream->ms.encoder=ms_filter_create_encoder(AUDIO_CODEC);
        stream->ms.decoder=ms_filter_create_decoder(AUDIO_CODEC);
    }
    if(stream->use_mix)
        stream->mixvoice = ms_filter_new(MS_MIXVOICE_ID);
    if(stream->use_volsend)
        stream->volsend = ms_filter_new(MS_VOLUME_ID);
    if(stream->use_volrecv)
        stream->volrecv = ms_filter_new(MS_VOLUME_ID);
	
    /*create the equalizer*/
    if (stream->eq_SPK)
        stream->equalizerSPK=ms_filter_new(MS_EQUALIZER_ID);
    if (stream->eq_MIC)
        stream->equalizerMIC=ms_filter_new(MS_EQUALIZER_ID);
    /*configure resampler if needed*/
    //if (stream->read_resampler){
    //    audio_stream_configure_resampler(stream->read_resampler,stream->soundread,stream->ms.udpsend);
    //}

    //if (stream->write_resampler){
    //    audio_stream_configure_resampler(stream->write_resampler,stream->ms.udprecv,stream->soundwrite);
    //} 
    if (stream->soundread && stream->soundwrite && use_ec && stream->receive_graph && stream->send_graph){
        if(use_ec == 1){
            printf("set aec filter(method%d)\n",stream->use_ec);
            stream->ec=ms_filter_new(MS_SBC_AEC_ID);
            ms_filter_call_method(stream->ec,MS_ECHO_CANCELLER_SET_DELAY,&stream->ecdelay);
        }else{  //use_ec == 2       
            printf("no aec filter(method%d)\n",use_ec);
            ms_filter_call_method(stream->soundread,MS_FILTER_SET_USEAEC,&stream->use_ec);
        }
    }
    
    /* and then connect all */
    /* tip: draw yourself the picture if you don't understand */

    /*sending graph*/
    if(stream->send_graph){
    
        ms_connection_helper_start(&h);
        // [soundread]--pin0--
        ms_connection_helper_link(&h,stream->soundread,-1,0);
#ifdef TWO_WAY_AUDIORECORD
        if(select_flow == AudioFromUdpRecvSoundRead)
            audio_mkv_rec_graph_link(&h,stream,select_flow);
#endif
#ifdef PURE_WAV_RECORD  
        if(select_flow == AudioFromUdpRecvSoundRead)
            audio_pure_wav_record_graph_link(&h,stream);
#endif 
        if (stream->ec)
            ms_connection_helper_link(&h,stream->ec,1,1);
        if (stream->volsend)
            ms_connection_helper_link(&h,stream->volsend,0,0);
        if (stream->equalizerMIC)
            ms_connection_helper_link(&h,stream->equalizerMIC,0,0);
        if (stream->read_resampler)
            ms_connection_helper_link(&h,stream->read_resampler,0,0);
        if (stream->ms.encoder)
            ms_connection_helper_link(&h,stream->ms.encoder,0,0);
        ms_connection_helper_link(&h,stream->ms.udpsend,0,-1);
    }
    /*receiving graph*/
    if(stream->receive_graph){
        ms_connection_helper_start(&h);
        // [udprecv]--pin0--
        ms_connection_helper_link(&h,stream->ms.udprecv,-1,0);
        #ifdef TWO_WAY_AUDIORECORD
        if(select_flow == AudioFromUdpRecv)
            audio_mkv_rec_graph_link(&h,stream,select_flow);
        #else
            audio_mkv_rec_graph_link(&h,stream,select_flow);
        #endif
        if(stream->ms.decoder)
            ms_connection_helper_link(&h,stream->ms.decoder,0,0);
        if (stream->write_resampler)
            ms_connection_helper_link(&h,stream->write_resampler,0,0);
        #ifdef PURE_WAV_RECORD
        if(select_flow == AudioFromUdpRecv)
            audio_pure_wav_record_graph_link(&h,stream);
        #endif
        if (stream->dtmfgen)
            ms_connection_helper_link(&h,stream->dtmfgen,0,0);
        if (stream->volrecv)
            ms_connection_helper_link(&h,stream->volrecv,0,0);
        if (stream->mixvoice)
            ms_connection_helper_link(&h,stream->mixvoice,0,0);
        if (stream->equalizerSPK)
            ms_connection_helper_link(&h,stream->equalizerSPK,0,0);
        if (stream->ec)
            ms_connection_helper_link(&h,stream->ec,0,0);
        ms_connection_helper_link(&h,stream->soundwrite,0,-1);
    }
    
    /* create ticker */
    stream->ms.sessions.ticker=ms_ticker_new();
    ms_ticker_set_name(stream->ms.sessions.ticker,"Audio MSTicker");
    if(stream->send_graph) ms_ticker_attach(stream->ms.sessions.ticker,stream->soundread);
    if(stream->receive_graph) ms_ticker_attach(stream->ms.sessions.ticker,stream->ms.udprecv);
	
    stream->ms.start_time=ms_time(NULL);
    stream->ms.is_beginning=TRUE;
    
#ifdef PURE_WAV_RECORD
    stream->a_recorder.ticker = ms_ticker_new();
    ms_ticker_set_name(stream->a_recorder.ticker,"Audiorec MSTicker");
    ms_ticker_attach(stream->a_recorder.ticker,stream->a_recorder.recorder);
#endif
    return 0;
}

void audio_stream_udp_stop(AudioStream * stream, LinphoneAudioStreamFlow select_flow){
    if (stream->ms.sessions.ticker){
        MSConnectionHelper h;
        if(stream->send_graph) ms_ticker_detach(stream->ms.sessions.ticker,stream->soundread);
        if(stream->receive_graph) ms_ticker_detach(stream->ms.sessions.ticker,stream->ms.udprecv);
#ifdef PURE_WAV_RECORD
        ms_ticker_detach(stream->a_recorder.ticker, stream->a_recorder.recorder);        
#endif
        //rtp_stats_display(rtp_session_get_stats(stream->ms.sessions.rtp_session),"Audio session's RTP statistics");

        /*dismantle the outgoing graph*/
        if(stream->send_graph){
            ms_connection_helper_start(&h);
            ms_connection_helper_unlink(&h,stream->soundread,-1,0);
#ifdef TWO_WAY_AUDIORECORD
            if(select_flow == AudioFromUdpRecvSoundRead)
                audio_mkv_rec_graph_unlink(&h,stream);
#endif
#ifdef PURE_WAV_RECORD
            if(select_flow == AudioFromUdpRecvSoundRead)
                audio_pure_wav_record_graph_unlink(&h,stream);
            #endif
            if (stream->ec!=NULL)
                ms_connection_helper_unlink(&h,stream->ec,1,1);
            if (stream->volsend!=NULL)
                ms_connection_helper_unlink(&h,stream->volsend,0,0);
            if (stream->equalizerMIC!=NULL)
                ms_connection_helper_unlink(&h,stream->equalizerMIC,0,0);
            if (stream->dtmfgen_rtp)
                ms_connection_helper_unlink(&h,stream->dtmfgen_rtp,0,0);
            if (stream->read_resampler!=NULL)
                ms_connection_helper_unlink(&h,stream->read_resampler,0,0);
            if (stream->ms.encoder)
                ms_connection_helper_unlink(&h,stream->ms.encoder,0,0);
            ms_connection_helper_unlink(&h,stream->ms.udpsend,0,-1);
        }
        
        /*dismantle the receiving graph*/
        if(stream->receive_graph){
            ms_connection_helper_start(&h);
            ms_connection_helper_unlink(&h,stream->ms.udprecv,-1,0);
            #ifdef TWO_WAY_AUDIORECORD
            if(select_flow == AudioFromUdpRecv)
                audio_mkv_rec_graph_unlink(&h,stream);
            #else
                audio_mkv_rec_graph_unlink(&h,stream);
            #endif  
            if (stream->ms.decoder)
                ms_connection_helper_unlink(&h,stream->ms.decoder,0,0);
            if (stream->write_resampler!=NULL)
                ms_connection_helper_unlink(&h,stream->write_resampler,0,0);
            #ifdef PURE_WAV_RECORD
            if(select_flow == AudioFromUdpRecv)
                audio_pure_wav_record_graph_unlink(&h,stream);
            #endif        
            if (stream->dtmfgen!=NULL)
                ms_connection_helper_unlink(&h,stream->dtmfgen,0,0);
            if (stream->volrecv!=NULL)
                ms_connection_helper_unlink(&h,stream->volrecv,0,0);
            if (stream->mixvoice!=NULL)
                ms_connection_helper_unlink(&h,stream->mixvoice,0,0);        
            if (stream->equalizerSPK!=NULL)
                ms_connection_helper_unlink(&h,stream->equalizerSPK,0,0);
            if (stream->ec!=NULL)
                ms_connection_helper_unlink(&h,stream->ec,0,0);
            ms_connection_helper_unlink(&h,stream->soundwrite,0,-1);
        }

    }
    audio_stream_udp_free(stream);
    ms_filter_log_statistics();
}

#if ENABLE_AUDIO_ENGENEER_MODEL
#include "ite/itp.h"
char *Get_Storage_path(void)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus = NULL;
    int i;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        driveStatus = &driveStatusTable[i];
		 if (driveStatus->disk >= ITP_DISK_MSC00 && driveStatus->disk <= ITP_DISK_MSC17)
        {
            if (driveStatus->avail )
            {
                printf("USB #%d inserted: %s\n", driveStatus->disk - ITP_DISK_MSC00, driveStatus->name[0]);
                return driveStatus->name[0];
            }
        }
    }
    return NULL;
} 
#endif

void audio_stream_post_configure(AudioStream *stream, bool_t mic_muted,dictionary* inicfg){

    char str[128];
    if (stream->volsend){      
        bool_t use_agc = CFG_AGC_ENABLE;
        bool_t use_ng  = stream->use_ng;
        double coef     = iniparser_getdouble(inicfg , strcat(strcpy(str,SOUND),"ngcoef"), CFG_NOISE_GATE_THRESHOLD);
        double gain     = iniparser_getdouble(inicfg , strcat(strcpy(str,SOUND),"mic_gain"), CFG_MIC_GAIN);
        _msvoleme_default_set(stream->volsend,use_agc,use_ng,coef,gain);       
    }

    if (stream->volrecv){
        bool_t use_agc = CFG_AGC_ENABLE_SPK;
        bool_t use_ng  = CFG_NOISE_GATE_ENABLE_SPK;
        double coef     = iniparser_getdouble(inicfg , strcat(strcpy(str,SOUND),"ngcoef_spk"), CFG_NOISE_GATE_THRESHOLD_SPK);
        double gain     = iniparser_getdouble(inicfg , strcat(strcpy(str,SOUND),"spk_gain"), CFG_SPEAKER_GAIN);  
        _msvoleme_default_set(stream->volrecv,use_agc,use_ng,coef,gain); 
    }

    if (stream->equalizerSPK){
        //const char *SPKgains=lp_config_get_string(lc->config,"sound","eq_SPKgains",NULL);
		//char *SPKgains=CFG_EQUALIZER_SPKGAIN_SET;
        char *SPKgains=iniparser_getstring(inicfg , strcat(strcpy(str,SOUND),"eq_SPKgains"), CFG_EQUALIZER_SPKGAIN_SET);      
        ms_filter_call_method(stream->equalizerSPK,MS_EQUALIZER_SET_ACTIVE,&stream->equalizerSPK);
        ms_filter_call_method(stream->equalizerSPK,MS_FILTER_SET_SAMPLE_RATE,&stream->audiorate);        
        
        if (SPKgains){
            do{
                int bytes;
                MSEqualizerGain g;
                if (sscanf(SPKgains,"%f:%f:%f %n",&g.frequency,&g.gain,&g.width,&bytes)==3){
                    ms_message("Read equalizer SPKgains: %f(~%f) --> %f",g.frequency,g.width,g.gain);
                    ms_filter_call_method(stream->equalizerSPK,MS_EQUALIZER_SET_GAIN,&g);
                    SPKgains+=bytes;
                }else break;
            }while(1);
        }
    }

    if (stream->equalizerMIC){
        MSFilter *f=stream->equalizerMIC;
        //const char *MICgains=lp_config_get_string(lc->config,"sound","eq_MICgains",NULL);
		//const char *MICgains=CFG_EQUALIZER_MICGAIN_SET;
        char *MICgains=iniparser_getstring(inicfg , strcat(strcpy(str,SOUND),"eq_MICgains"), CFG_EQUALIZER_MICGAIN_SET);
        ms_filter_call_method(stream->equalizerMIC,MS_EQUALIZER_SET_ACTIVE,&stream->eq_MIC);        
        ms_filter_call_method(stream->equalizerMIC,MS_FILTER_SET_SAMPLE_RATE,&stream->audiorate);
        if (MICgains){
            do{
                int bytes;
                MSEqualizerGain g;
                if (sscanf(MICgains,"%f:%f:%f %n",&g.frequency,&g.gain,&g.width,&bytes)==3){
                    ms_message("Read equalizer MICgains: %f(~%f) --> %f",g.frequency,g.width,g.gain);
                    ms_filter_call_method(stream->equalizerMIC,MS_EQUALIZER_SET_GAIN,&g);
                    MICgains+=bytes;
                }else break;
            }while(1);
        }
    }
    
}

/*void video_stream_set_recorder_audio_codec(VideoStream *stream) {
    if (stream->av_recorder.audio_input && stream->av_recorder.recorder){
        MSPinFormat pinfmt={0};
        ms_filter_call_method(stream->av_recorder.audio_input,MS_FILTER_GET_OUTPUT_FMT,&pinfmt);
        if (pinfmt.fmt){
            ms_message("Configuring av recorder with video format %s",ms_fmt_descriptor_to_string(pinfmt.fmt));
            pinfmt.pin=1;
            ms_filter_call_method(stream->av_recorder.recorder,MS_FILTER_SET_INPUT_FMT,&pinfmt);
        }
    }
}*/

void video_stream_udp_link_audio(VideoStream *v_stream, AudioStream *a_stream){
    if ((NULL == v_stream) || (NULL == a_stream))
        return;

    v_stream->audiostream=a_stream;
    if (v_stream->av_recorder.audio_input && a_stream->itcsink){
        ms_message("video_stream_link_audio() connecting itc filters");
        ms_filter_call_method(a_stream->itcsink,MS_ITC_SINK_CONNECT,v_stream->av_recorder.audio_input);
    }
}

void video_stream_udp_unlink_audio(VideoStream *v_stream, AudioStream *a_stream){
    if ((NULL == v_stream) || (NULL == a_stream))
        return;

    v_stream->audiostream=NULL;
    if (v_stream->av_recorder.audio_input && a_stream->itcsink){
        ms_filter_call_method(a_stream->itcsink,MS_ITC_SINK_CONNECT,NULL);
    }
}

void audio_stream_udp_link_audio_record(AudioStream *a_stream){
    if (NULL == a_stream)
        return;

    if (a_stream->a_recorder.audio_input && a_stream->a_recorder.itcsink){
        ms_filter_call_method(a_stream->a_recorder.itcsink,MS_ITC_SINK_CONNECT,a_stream->a_recorder.audio_input);
    }
}

void audio_stream_udp_unlink_audio_record(AudioStream *a_stream){
    if (NULL == a_stream)
        return;

    if (a_stream->a_recorder.audio_input && a_stream->a_recorder.itcsink){
        ms_filter_call_method(a_stream->a_recorder.itcsink,MS_ITC_SINK_CONNECT,NULL);
    }
}

