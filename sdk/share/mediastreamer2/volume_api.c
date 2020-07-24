#include "volume_api.h"

static const float max_e = (32768* 0.3);/*is RMS factor */
static const float coef = 0.2; /* floating averaging coeff. for energy */
//static const float gain_k = 0.02; /* floating averaging coeff. for gain */
static const float vol_upramp = 0.4;
static const float vol_downramp = 0.45;   /* not yet runtime parameterizable */
static const float min_ng_floorgain=0.005;
static const float agc_threshold=0.5;

static inline int16_t saturate(int val) {
	return (val>32767) ? 32767 : ( (val<-32767) ? -32767 : val);
}

static void update_energy(int16_t *signal, int numsamples, Volume *v) {
	int i;
	float acc = 0;
	float en;
	int lp = 0, pk = 0;
		
	for (i=0;i<numsamples;++i){
		int s=signal[i];
		acc += s * s;

		lp = abs(s);
		if (lp > pk) pk = lp;
	}
	en = (sqrt(acc / numsamples)+1) / v->max_energy;
	v->energy = (en * coef) + v->energy * (1.0 - coef);
	v->level_pk = (float)pk / v->max_energy;
	v->instant_energy = en;// currently non-averaged energy seems better (short artefacts)
}

static void apply_gain(Volume *v, mblk_t *m, float tgain) {
	int16_t *sample;
	int dc_offset = 0;
	int32_t intgain;
	float gain;

	/* ramps with factors means linear ramps in logarithmic domain */
	
	if (v->gain < tgain) {
		if (v->gain<v->ng_floorgain) v->gain=v->ng_floorgain;
        
		v->gain *= 1 + (v->fast_upramp ? v->vol_fast_upramp : v->vol_upramp);
		if (v->gain > tgain) v->gain = tgain;
        
	}else if (v->gain > tgain) {
		v->gain *= 1 - v->vol_downramp;
        
		if (v->gain < tgain) v->gain = tgain;
		v->fast_upramp=FALSE;
	}

	gain=v->gain * v->ng_gain;
	intgain = gain* 4096;

	if (v->remove_dc){
		for (sample=(int16_t*)m->b_rptr;sample<(int16_t*)m->b_wptr;++sample){
			dc_offset+= *sample;
			*sample = saturate(((*sample - v->dc_offset) * intgain) / 4096);
		}
		/* offset smoothing */
		v->dc_offset = (v->dc_offset*7 + dc_offset*2/(m->b_wptr - m->b_rptr)) / 8;
	}else if (gain!=1){
		for (sample=(int16_t*)m->b_rptr;sample<(int16_t*)m->b_wptr;++sample){
			*sample = saturate(((*sample) * intgain) / 4096);
		}
	}
}

static void volume_noise_gate_process(Volume *v , float energy, mblk_t *om){
	int nsamples=((om->b_wptr-om->b_rptr)/2);
	if (energy > v->ng_threshold) {
        v->ng_noise_dur = v->ng_cut_time;
        if(v->ng_tgain<.2){
            v->ng_tgain +=0.05;
        }else if(v->ng_tgain>.2 && v->ng_tgain<1.0){
            v->ng_tgain +=0.1;
            v->ng_threshold=0.025;
        }else{
            v->ng_tgain=1;
            v->ng_threshold=0.025;
        }
	}
	else {
        
		if (v->ng_noise_dur > 0) {
			v->ng_noise_dur -= (nsamples * 1000) / v->sample_rate;
			//noise buffer time:;
		}else{        
            if(v->ng_tgain>0.1){
                
                v->ng_tgain -=0.05;
                v->ng_threshold=0.03;
            }else{
                v->ng_threshold=0.05;
                v->ng_tgain=0;
            }
        }
	}

	v->ng_gain = v->ng_tgain;
    //v->ng_gain = v->ng_gain*0.75 + v->ng_tgain*0.25;
    //v->ng_gain = v->ng_gain*0.25 + v->ng_tgain*0.75;


Volume* VOICE_INIT(void){
	Volume *v=(Volume*)ms_new0(Volume,1);
	v->energy=0;
	v->level_pk = 0;
	v->static_gain = v->gain = v->target_gain = 1;
	v->dc_offset = 0;
	v->vol_upramp = vol_upramp;
	v->vol_fast_upramp=vol_upramp*3;
	v->vol_downramp = vol_downramp;
	v->sustain_time=200;
	v->sustain_dur = 0;
	v->agc_enabled=FALSE;
	v->buffer=ms_bufferizer_new();
	v->sample_rate=CFG_AUDIO_SAMPLING_RATE;
	v->nsamples=80;
	v->noise_gate_enabled=TRUE;
	v->ng_cut_time = 400;/*TODO: ng_sustain (milliseconds)*/
	v->ng_noise_dur=0;
	v->ng_threshold=0.05;
	v->ng_floorgain=min_ng_floorgain;
	v->ng_gain = 1;
	v->remove_dc=FALSE;
    v->max_energy=max_e;

    //preprocess
    if(v->ng_floorgain < min_ng_floorgain) v->ng_floorgain = min_ng_floorgain;
	if (v->noise_gate_enabled) v->gain = v->target_gain = v->ng_floorgain; // start with floorgain (soft start)
    
    v->nsamples=(int)(0.01*(float)v->sample_rate);
	v->fast_upramp=TRUE;
    return v;
}


void VOICE_applyProcess(Volume *v,mblk_t *TxIn){

	float target_gain;
	update_energy((int16_t*)TxIn->b_rptr, (TxIn->b_wptr - TxIn->b_rptr) / 2, v);
	target_gain = v->static_gain;

	if (v->agc_enabled)
        target_gain/= volume_agc_process(v, om);
    
    if (v->noise_gate_enabled)
        volume_noise_gate_process(v, v->instant_energy, TxIn);
    
	apply_gain(v, TxIn, target_gain);

}

void VOICE_UNINIT(Volume *v){
    ms_free(v);  
}

void VOICE_set_dB_gain(Volume *v,float dBgain){
    //set dB gain 0.0~max
    v->gain = v->static_gain = pow(10,(dBgain)/10);
    printf("dBgain=%f,v->static_gain=%f\n",dBgain,v->static_gain);
}

void VOICE_set_max_energy(Volume *v,float coef){
    //set max_energy 0.0~9.9... (0:mute )
    v->max_energy=32768*(1-coef);
}

void VOICE_set_agc(Volume *v,bool arg){
    v->agc_enabled = arg;
}

void VOICE_set_noise_gate(Volume *v,bool arg){
    v->noise_gate_enabled = arg;
}
/*
same as msvolume.c filter
volume_api is light process
------
*init*
------
volume v;
v=VOICE_INIT();
VOICE_set_dB_gain(v,3.0);  
VOICE_set_noise_gate(v,1);
VOICE_set_agc(v,1);
VOICE_set_max_energy(v,0.7);
---------
*process*
---------
mblk_t *om;
...
VOICE_applyProcess(v,om);
--------------------------------
--------
*uninit*
--------
VOICE_UNINIT(v)
*/