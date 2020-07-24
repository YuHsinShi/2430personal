#include "mediastreamer2/msvolume.h"
#include "mediastreamer2/msticker.h"
#include <math.h>

typedef struct Volume{
	float energy;
	float level_pk;
	float instant_energy;
	float gain; 				/**< the one really applied, smoothed target_gain version*/
	float static_gain;	/**< the one fixed by the user */
	int     dc_offset;
	//float gain_k;
	float vol_upramp;
	float vol_fast_upramp;
	float vol_downramp;
	float target_gain; /*the target gain choosed by echo limiter and noise gate*/
    float max_energy;
	int sustain_time; /* time in ms for which echo limiter remains active after resuming from speech to silence.*/
	int sustain_dur;
	int sample_rate;
	int nsamples;
	int ng_cut_time; /*noise gate cut time, after last speech detected*/
	int ng_noise_dur;
	float ng_threshold;
	float ng_floorgain;
	float ng_gain;
    float ng_threshold
	MSBufferizer *buffer;
	bool_t agc_enabled;
	bool_t noise_gate_enabled;
	bool_t remove_dc;
	bool_t fast_upramp;
}Volume;

//init
Volume* VOICE_INIT(void);
void VOICE_set_dB_gain(Volume *v,float dBgain);
void VOICE_set_max_energy(Volume *v,float coef);
void VOICE_set_agc(Volume *v,bool arg);
void VOICE_set_noise_gate(Volume *v,bool arg);
//process
void VOICE_applyProcess(Volume *v,mblk_t *TxIn);
//uninit
void VOICE_UNINIT(Volume *v);
