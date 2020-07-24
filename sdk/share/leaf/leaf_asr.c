/**************************************
Linphone Excluded API and Function
***************************************/
#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msfilewriter.h" 

#include "leaf_mediastream.h"
extern pthread_mutex_t Leaf_mutex;
static AsrCallback asrCallback=NULL;

static void leaf_asr_filter_callbackfunc(int state,void *arg)
{
    if (asrCallback)
    {
        return asrCallback(state,arg);
    }
}

void Leaf_asr_callback(void *userdata, struct _MSFilter *f, unsigned int id, void *arg)
{
	switch (f->desc->id)
	{
		case MS_ASR_ID:
            leaf_asr_filter_callbackfunc(ASR_SUCCESS_ARG,arg);
			break;

		default:
			break;
	}		
}

void leaf_start_asr(LeafCall *call,LeafAsrFlowCase asrcase,
    char *infile,char *outfile,AsrCallback func){
        
	MSSndCard *playcard=call->play_sndcard;
	MSSndCard *captcard=call->capt_sndcard;
    if (playcard) ms_snd_card_set_level(playcard,MS_SND_CARD_PLAYBACK,call->sound_conf.play_level);
    if (captcard) ms_snd_card_set_level(captcard,MS_SND_CARD_CAPTURE,call->sound_conf.rec_level); 
    leaf_stop_sound_play(call);
    if(call->asrstream) asr_stop(call->asrstream);//stop asrstream;
    
    switch(asrcase){
        case FileAsr: 
            playcard=NULL;
            outfile =NULL;
            break;//[infile]--[denoise]--[msvolume]--[asr]
        case FilePlayAsr: 
            captcard=NULL;
            outfile =NULL;
            break;//[infile]--[denoise]--[msvolume]--[asr]--[spk]
        case RecAsr:  
            infile  =NULL;
            playcard=NULL;
            outfile =NULL;
            break;//[rec]--[denoise]--[msvolume]--[asr]
        case RecPlayAsr:
            infile  =NULL;
            outfile =NULL;
            break;//[rec]--[denoise]--[msvolume]--[asr]--[spk]
        case RecFile:
            infile  =NULL;
            playcard=NULL;
            break;//[rec]--[denoise]--[msvolume]--[asr:bypass]--[record]
        case RecFilePlay:
            infile  =NULL;
            break;//[rec]--[denoise]--[msvolume]--[asr:bypass]--[record]
                  //                                          |-[spk]  
        case AECFilePlayAsr:
            outfile =NULL;
            break;//[file]--|AEC|--[spk]
                  //[mic ]--|AEC|--[asr]
        default:
            playcard=NULL;
    }
    pthread_mutex_lock(&Leaf_mutex);
    asrCallback = func;
    printf("asr case : %d\n",asrcase);
    if(asrCallback==NULL)
        call->asrstream = asr_start_with_cb(infile,outfile,captcard,playcard ,NULL,NULL);
    else
        call->asrstream = asr_start_with_cb(infile,outfile,captcard,playcard ,Leaf_asr_callback,NULL);
    pthread_mutex_unlock(&Leaf_mutex);
}

//void leaf_start_train_rec(LeafCall *call,LeafAsrFlowCase asrcase,char *outfile,AsrCallback func)

void leaf_pause_asr(LeafCall *call,int pause){
    asr_pause(call->asrstream,pause);
}

void leaf_stop_asr(LeafCall *call){
    if(call->asrstream){
        pthread_mutex_lock(&Leaf_mutex);
        asr_stop(call->asrstream);
        call->asrstream=NULL;
        asrCallback = NULL;
        pthread_mutex_unlock(&Leaf_mutex);
    }
}

bool leaf_get_asrstream_exit(LeafCall *call){
    if(call->asrstream)
        return true;
    else
        return false;
}

void _leaf_set_file_loop(LeafCall *call,bool loop){
    asr_set_file_loop(call->asrstream,loop);
}