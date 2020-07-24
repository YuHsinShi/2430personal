
#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msfileplayer.h"
#include "mediastreamer2/msfilerec.h"
#include "mediastreamer2/msasr.h"

void asr_st_free(AsrStream *st){
    ms_ticker_destroy(st->ticker);
    if(st->source)  ms_filter_destroy(st->source);
    if(st->denoise) ms_filter_destroy(st->denoise);
    if(st->volume)  ms_filter_destroy(st->volume);
    if(st->asr   )  ms_filter_destroy(st->asr);
    if(st->sndwrite) ms_filter_destroy(st->sndwrite);
    if(st->filewrite) ms_filter_destroy(st->filewrite);
    ms_free(st);
}

AsrStream * asr_start_with_cb(
    const char *infile,const char *outfile,
    MSSndCard *captcard,MSSndCard *sndcard,
    MSFilterNotifyFunc func,void * user_data)
{
    AsrStream *st;
    MSConnectionHelper h;
    st=(AsrStream *)ms_new0(AsrStream,1);   
    int sample_rate =16000;
    int use_ec=0;
    if(infile && captcard && sndcard) use_ec=1;
    castor3snd_reinit_for_video_memo_play();//load sbc codec
    Castor3snd_reinit_for_diff_rate(sample_rate,16,1);//force to 16K

    if (infile){
        int interval = -1;
        st->source=ms_filter_new(MS_FILE_PLAYER_ID); //data from wav file
        ms_filter_call_method(st->source,MS_FILE_PLAYER_OPEN,(void*)infile);
        ms_filter_call_method(st->source,MS_FILE_PLAYER_LOOP,&interval);//loop play
        ms_filter_call_method_noarg(st->source,MS_FILE_PLAYER_START);
        if(use_ec) {
            ms_filter_call_method(st->source,MS_FILE_PLAYER_SET_SPECIAL_CASE,&interval);//16K
            st->sndread = ms_snd_card_create_reader(captcard);//data from rec(mic)
        }
        
    }else{
        st->source = ms_snd_card_create_reader(captcard);//data from rec(mic)
    }
    
    if(use_ec){
        int delay =CFG_AEC_DELAY_MS;
        int bypass =1;
        st->ec=ms_filter_new(MS_SBC_AEC_ID);//mic & spk & fileplay with echo cancellation.
        ms_filter_call_method(st->ec,MS_ECHO_CANCELLER_SET_DELAY,&delay);
        ms_filter_call_method(st->ec,MS_ECHO_CANCELLER_SET_BYPASS_MODE,&bypass);
        st->dummy = ms_filter_new(MS_VOID_SINK_ID);
        //st->volume = ms_filter_new(MS_VOLUME_ID);//agc 
        st->asr=ms_filter_new(MS_ASR_ID);//ASR 
    }else{
        //st->denoise=ms_filter_new(MS_FILTER_DENOISE_ID);//denoise
        //st->volume = ms_filter_new(MS_VOLUME_ID);//agc 
        st->asr=ms_filter_new(MS_ASR_ID);//ASR 
    }

    if(outfile){
        int bypass =1;
        st->filewrite=ms_filter_new(MS_FILE_REC_ID); //rec wav file for train data;
        ms_filter_call_method(st->filewrite,MS_FILTER_SET_SAMPLE_RATE,&sample_rate);
        ms_filter_call_method_noarg(st->filewrite,MS_FILE_REC_CLOSE);
        ms_filter_call_method(st->filewrite,MS_FILE_REC_OPEN,(void*)outfile);
        ms_filter_call_method_noarg(st->filewrite,MS_FILE_REC_START);
        ms_filter_call_method(st->asr,MS_ASR_BYPASS,&bypass);//set asr by pass
    }

    if(sndcard){
        st->sndwrite=ms_snd_card_create_writer(sndcard);//data play out (spk)
    }else{
        st->sndwrite=ms_filter_new(MS_VOID_SINK_ID);//discard data

    }
     
    if(func!=NULL)//callback function
        ms_filter_set_notify_callback(st->asr,func,user_data);

    
    st->ticker=ms_ticker_new();

    ms_ticker_set_name(st->ticker,"Audio (asr) MSTicker");
    //
    ms_connection_helper_start(&h);
    ms_connection_helper_link(&h,st->source,-1,0);
    if (st->ec)
        ms_connection_helper_link(&h,st->ec,0,0);
    if (st->denoise)
        ms_connection_helper_link(&h,st->denoise,0,0);
    if (st->volume && !st->ec)
        ms_connection_helper_link(&h,st->volume,0,0);
    if (st->asr && !st->ec)
        ms_connection_helper_link(&h,st->asr,0,0);
    ms_connection_helper_link(&h,st->sndwrite,0,-1);
    
    if (st->filewrite){
        ms_connection_helper_start(&h);
        ms_connection_helper_link(&h,st->asr,-1,1); 
        ms_connection_helper_link(&h,st->filewrite,0,-1); 
    }
    
    if(st->ec){
        ms_connection_helper_start(&h);
        ms_connection_helper_link(&h,st->sndread,-1,0);
        if (st->volume)
            ms_connection_helper_link(&h,st->volume,0,0);
        if (st->ec)
            ms_connection_helper_link(&h,st->ec,1,1);
        if (st->asr)
            ms_connection_helper_link(&h,st->asr,0,0);
        ms_connection_helper_link(&h,st->dummy,0,-1);
        ms_ticker_attach(st->ticker,st->sndread);
    }
 
    //
    
    ms_ticker_attach(st->ticker,st->source);

    if(st->volume)
        _msvoleme_default_set(st->volume,1,0,0.8,0.0);//agc,ng,ngcoef,gain

    return st;
}

void asr_stop(AsrStream *st){
    MSConnectionHelper h;   

    ms_ticker_detach(st->ticker,st->source);
    //
    ms_connection_helper_start(&h);
    ms_connection_helper_unlink(&h,st->source,-1,0);
    if (st->ec)
        ms_connection_helper_unlink(&h,st->ec,0,0);
    if (st->denoise)
        ms_connection_helper_unlink(&h,st->denoise,0,0);
    if (st->volume && !st->ec)
        ms_connection_helper_unlink(&h,st->volume,0,0);
    if (st->asr && !st->ec)
        ms_connection_helper_unlink(&h,st->asr,0,0);
    ms_connection_helper_unlink(&h,st->sndwrite,0,-1);
    
    if(st->filewrite){
        ms_connection_helper_start(&h);
        ms_connection_helper_unlink(&h,st->asr,-1,1); 
        ms_connection_helper_unlink(&h,st->filewrite,0,-1); 
    }
    
    if(st->ec){
        ms_ticker_detach(st->ticker,st->sndread);
        ms_connection_helper_start(&h);
        ms_connection_helper_unlink(&h,st->sndread,-1,0);
        if (st->volume)
            ms_connection_helper_unlink(&h,st->volume,0,0);
        if (st->ec)
            ms_connection_helper_unlink(&h,st->ec,1,1);
        if (st->asr)
            ms_connection_helper_unlink(&h,st->asr,0,0);
        ms_connection_helper_unlink(&h,st->dummy,0,-1);
    }
    //
    asr_st_free(st);
    castor3snd_deinit_for_video_memo_play();//deinit i2s 
}

void asr_pause(AsrStream *st,int pause){
    if(st->asr){
        ms_filter_call_method(st->asr,MS_ASR_PAUSE,&pause);
    }
}

void asr_set_file_loop(AsrStream *st,bool_t loop){
    if(ms_filter_get_id(st->source)==MS_FILE_PLAYER_ID){
        int interval=-1;
        if(loop) interval=2000;
        ms_filter_call_method(st->source,MS_FILE_PLAYER_LOOP,&interval);//loop play
    }
}
