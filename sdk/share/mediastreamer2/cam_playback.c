#ifdef HAVE_CONFIG_H
    #include "mediastreamer-config.h"
#endif

#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "mediastreamer2/msitc.h"
#include "private.h"
#include "video_encoder/video_encoder_it970.h"

#if defined(CFG_SENSOR_ENABLE)
#define SENSOR_IN_W SENSOR_WIDTH
#define SENSOR_IN_H SENSOR_HEIGHT
#else
#define SENSOR_IN_W 1280
#define SENSOR_IN_H 720
#endif

#define STREAM_BUF_SIZE 250 * 1024

/**************************************************************
*with_preview = false
*sensor->capture->video process (ISP1)
*
*with_preview = true
*sensor->capture->video process (ISP1)->video process (ISP0)
*
*
*memory_mode = true
*capture write memory, then video process (ISP1) read memory.
*
*memory_mode = false
*capture with video process (ISP1) onfly
*
***************************************************************/  
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

VideoStream *cam_playback_start(bool_t with_preview, bool_t memory_mode)
{
#if 0
    VideoStream        *stream;
    MSConnectionHelper ch;
    MSVideoSize        inputvsize;
#ifdef __OPENRTOS__        
	if (with_preview == 0)
    {   
        stream         = (VideoStream *)ms_new0(VideoStream, 1);
        if(memory_mode)
        {
            stream->source = ms_filter_new(MS_CAPVPMEM_ID);
            inputvsize.width  = SENSOR_IN_W;
            inputvsize.height = SENSOR_IN_H;
            ms_filter_call_method(stream->source,MS_FILTER_SET_VIDEO_SIZE,&inputvsize);    

        }
        else
            stream->source = ms_filter_new(MS_CAPVPONFLY_ID);
    	
    	ms_connection_helper_start(&ch);
    	ms_connection_helper_link(&ch, stream->source, -1, -1);
        stream->tickerforCamPlayback = ms_ticker_new();
        ms_ticker_set_name(stream->tickerforCamPlayback, "Camera Playback MSTicker");
        ms_ticker_attach(stream->tickerforCamPlayback, stream->source);        
    }
    else
    {
#ifdef CFG_LCD_ENABLE    
        stream         = (VideoStream *)ms_new0(VideoStream, 1);

        if(memory_mode)
        {
            stream->source = ms_filter_new(MS_CAPVPMEM_DISPLAY_ID);
            inputvsize.width  = SENSOR_IN_W;
            inputvsize.height = SENSOR_IN_H;
            ms_filter_call_method(stream->source,MS_FILTER_SET_VIDEO_SIZE,&inputvsize);    
        }
        else
            stream->source = ms_filter_new(MS_CAPVPONFLY_DISPLAY_ID);

     	ms_connection_helper_start(&ch);
        ms_connection_helper_link(&ch, stream->source, -1, -1);
        stream->tickerforCamPlayback = ms_ticker_new();
        ms_ticker_set_name(stream->tickerforCamPlayback, "Camera Playback MSTicker");
        ms_ticker_attach(stream->tickerforCamPlayback, stream->source);                  
#endif
    }

#endif
    return stream;
#else
    return NULL;
#endif
}

void cam_playback_stop(VideoStream *stream, bool_t with_preview)
{
    MSConnectionHelper ch;
#ifdef __OPENRTOS__ 

    if (with_preview == 0)
    {
        ms_ticker_detach(stream->tickerforCamPlayback, stream->source);
        ms_connection_helper_start(&ch);
	    ms_connection_helper_unlink(&ch, stream->source, -1, -1);
        ms_ticker_destroy(stream->tickerforCamPlayback);
        ms_filter_destroy(stream->source);
        ms_free(stream);

    }
    else
    {
#ifdef CFG_LCD_ENABLE      
        ms_ticker_detach(stream->tickerforCamPlayback, stream->source);            
        ms_connection_helper_start(&ch);
        ms_connection_helper_unlink(&ch, stream->source, -1, -1);
        ms_filter_destroy(stream->source);
		ms_ticker_destroy(stream->tickerforCamPlayback);
        ms_free(stream);
#endif
    }	
#endif
}


VideoStream *cam_playback_with_rec_start()
{
	VideoStream *stream;
	MSWebCam *cam;
	MSConnectionHelper ch;

	stream = (VideoStream *)ms_new0(VideoStream, 1);
	cam = ms_web_cam_manager_get_default_cam( ms_web_cam_manager_get());
	printf("YC: %s, %d cam = %s\n", __FUNCTION__, __LINE__, cam->name);

	stream->cam = cam;
	stream->source = ms_web_cam_create_reader(cam);

	stream->av_recorder.recorder = ms_filter_new(MS_MKV_RECORDER_ID);
    if (stream->av_recorder.recorder)
    {
        stream->teeforrecord = ms_filter_new(MS_TEE_ID);
        stream->itcsink = ms_filter_new(MS_ITC_SINK_ID);
        stream->av_recorder.audio_input = ms_filter_new(MS_ITC_SOURCE_ID);
        stream->av_recorder.video_input = ms_filter_new(MS_ITC_SOURCE_ID);
    }
	stream->output = ms_filter_new(MS_ITC_SINK_ID);
	
	ms_connection_helper_start (&ch);
	ms_connection_helper_link (&ch,stream->source,-1,0);
	ms_connection_helper_link (&ch,stream->teeforrecord,0,0);
	ms_filter_link(stream->teeforrecord,1,stream->itcsink,0);
	ms_filter_link(stream->av_recorder.video_input,0,stream->av_recorder.recorder,0);
    ms_filter_link(stream->av_recorder.audio_input,0,stream->av_recorder.recorder,1);
	ms_filter_call_method(stream->itcsink,MS_ITC_SINK_CONNECT,stream->av_recorder.video_input);
	video_stream_set_recorder_video_codec(stream, "H264");

	ms_connection_helper_link (&ch,stream->output,0,-1);

	stream->ms.sessions.ticker = ms_ticker_new();
	ms_ticker_set_name(stream->ms.sessions.ticker,"Video MSTicker");
	stream->av_recorder.ticker = ms_ticker_new();
	ms_ticker_set_name(stream->av_recorder.ticker,"MKVRecoder MSTicker");
	
	if (stream->source)
        ms_ticker_attach (stream->ms.sessions.ticker, stream->source); 
	if (stream->av_recorder.video_input)
       	ms_ticker_attach (stream->av_recorder.ticker, stream->av_recorder.video_input);

	return stream;
}


void cam_playback_with_rec_stop(VideoStream *stream)
{
	if (stream->ms.sessions.ticker){
        if (stream->source)
            ms_ticker_detach(stream->ms.sessions.ticker,stream->source);
		if (stream->av_recorder.video_input)
            ms_ticker_detach(stream->av_recorder.ticker,stream->av_recorder.video_input);
		
		if (stream->source){
			MSRecorderState rstate;
			MSConnectionHelper h;
			ms_connection_helper_start (&h);
			ms_connection_helper_unlink(&h, stream->source, -1, 0);
			//ms_connection_helper_unlink(&h,stream->itcsink,0,-1);
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
			ms_connection_helper_unlink(&h,stream->output,0,-1);
		}

		ms_filter_destroy(stream->source);
		ms_filter_destroy(stream->output);
		ms_ticker_destroy (stream->ms.sessions.ticker);
		ms_filter_destroy(stream->av_recorder.recorder);
		ms_filter_destroy(stream->itcsink);
		ms_filter_destroy(stream->teeforrecord);
        ms_filter_destroy(stream->av_recorder.audio_input);
        ms_filter_destroy(stream->av_recorder.video_input);
		ms_ticker_destroy (stream->av_recorder.ticker);
	}
	ms_free (stream);
}

void cam_playback_snapshot(char *file)
{
	FILE *fp;
	JPEG_ENCODE_PARAMETER enPara;

	fp = fopen(file, "wb");
	// JPEG Encoding   
    enPara.quality = 85;
    enPara.strmBuf = malloc(STREAM_BUF_SIZE);
    enPara.strmBuf_size = STREAM_BUF_SIZE;

	if (enPara.strmBuf)
    {
#ifndef _WIN32    
        JPEGEncodeFrame(&enPara);
#endif
    }
    
	fwrite(enPara.strmBuf, 1 ,enPara.enSize, fp);
	fclose(fp);
}
