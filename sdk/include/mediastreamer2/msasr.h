#include <mediastreamer2/msfilter.h>


#define MS_ASR_PAUSE		MS_FILTER_METHOD(MS_ASR_ID,0,int)

#define MS_ASR_RESET		MS_FILTER_METHOD(MS_ASR_ID,1,int)

#define MS_ASR_RELEASE	    MS_FILTER_METHOD(MS_ASR_ID,2,int)

#define MS_ASR_INIT	        MS_FILTER_METHOD(MS_ASR_ID,3,int)

#define MS_ASR_BYPASS	    MS_FILTER_METHOD(MS_ASR_ID,4,int)

//event
#define MS_ASR_SUCCESS	    MS_FILTER_EVENT_NO_ARG(MS_ASR_ID,0)
#define MS_ASR_FAIL	        MS_FILTER_EVENT_NO_ARG(MS_ASR_ID,1)

#define MS_ASR_SUCCESS_ARG  MS_FILTER_EVENT(MS_ASR_ID, 3, void*)

extern MSFilterDesc ms_asr_desc;

