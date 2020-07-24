/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2009  Simon MORLAT (simon.morlat@linphone.org)

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

#include <mediastreamer2/msequalizer.h>
#include <mediastreamer2/dsptools.h>

#include <math.h>

#ifdef _MSC_VER
#include <malloc.h>
#define alloca _alloca
#endif

#ifdef MS_FIXED_POINT
#define GAIN_ZERODB 20000
#else
#define GAIN_ZERODB 1.0
#endif

#define TAPS 512

typedef struct _EqualizerState{
    MSBufferizer *buff;
	int rate;
	int nfft; /*number of fft points in time*/
	ms_word16_t *fft_cpx;
} EqualizerState;

static void equalizer_state_flatten(EqualizerState *s){
	int i;
	ms_word16_t val=GAIN_ZERODB/s->nfft;
	s->fft_cpx[0]=val;
	for(i=1;i<s->nfft;i+=2)
		s->fft_cpx[i]=val;
}

/* TODO: rate also beyond 16000 */
static EqualizerState * equalizer_state_new(int nfft){
	EqualizerState *s=(EqualizerState *)ms_new0(EqualizerState,1);
	s->rate=44100;
	s->nfft=nfft;
	s->fft_cpx=(ms_word16_t*)ms_new0(ms_word16_t,s->nfft);
	equalizer_state_flatten(s);
    s->buff=ms_bufferizer_new();
	return s;
}

static void equalizer_state_destroy(EqualizerState *s){
    ms_bufferizer_destroy(s->buff);
	ms_free(s->fft_cpx);
	ms_free(s);
}

static int equalizer_state_hz_to_index(EqualizerState *s, int hz){
	int ret;
	if (hz<0){
		ms_error("Bad frequency value %i",hz);
		return -1;
	}
	if (hz>(s->rate/2)){
		hz=(s->rate/2);
	}
	/*round to nearest integer*/
	ret=((hz*s->nfft)+(s->rate/2))/s->rate;
	if (ret==s->nfft/2) ret=(s->nfft/2)-1;
	return ret;
}

static int equalizer_state_index2hz(EqualizerState *s, int index){
	return (index * s->rate + s->nfft/2) / s->nfft;
}

/* The natural peaking equalizer amplitude transfer function is multiplied to the discrete f-points.
 * Note that for PEQ no sqrt is needed for the overall calculation, applying it to gain yields the
 * same response.
 */
static float equalizer_compute_gainpoint(int f, int freq_0, float sqrt_gain, int freq_bw)
{
	float k1, k2;
	k1 = ((float)(f*f)-(float)(freq_0*freq_0));
	k1*= k1;
	k2 = (float)(f*freq_bw);
	k2*= k2;
	return (k1+k2*sqrt_gain)/(k1+k2/sqrt_gain);
}

static void equalizer_point_set(EqualizerState *s, int i, int f, float gain){
	ms_message("Setting gain %f for freq_index %i (%i Hz)\n",gain,i,f);
	s->fft_cpx[1+((i-1)*2)] = (s->fft_cpx[1+((i-1)*2)]*(int)(gain*32768))/32768;
}

static void equalizer_state_set(EqualizerState *s, int freq_0, float gain, int freq_bw){
	int i, f;
	int delta_f = equalizer_state_index2hz(s, 1);
	float sqrt_gain = sqrt(gain);
	int mid = equalizer_state_hz_to_index(s, freq_0);
	freq_bw-= delta_f/2;   /* subtract a constant - compensates for limited fft steepness at low f */
	if (freq_bw < delta_f/2)
		freq_bw = delta_f/2;
	i = mid;
	f = equalizer_state_index2hz(s, i);
	equalizer_point_set(s, i, f, gain);   /* gain according to argument */
	do {	/* note: to better accomodate limited fft steepness, -delta is applied in f-calc ... */
		i++;
		f = equalizer_state_index2hz(s, i);
		gain = equalizer_compute_gainpoint(f-delta_f, freq_0, sqrt_gain, freq_bw);
		equalizer_point_set(s, i, f, gain);
	}
	while (i < s->nfft/2 && (gain>1.1 || gain<0.9));
	i = mid;
	do {	/* ... and here +delta, as to  */
		i--;
		f = equalizer_state_index2hz(s, i);
		gain = equalizer_compute_gainpoint(f+delta_f, freq_0, sqrt_gain, freq_bw);
		equalizer_point_set(s, i, f, gain);
	}
	while (i>=0 && (gain>1.1 || gain<0.9));
}

static void dump_table(ms_word16_t *t, int len){
	int i;
	for(i=0;i<len;i++) {
#ifdef MS_FIXED_POINT
        if(len%8==0) printf("\n");
		printf("[%i] %i\t",i,t[i]);
#else
		ms_message("[%i]\t%f",i,t[i]);
#endif
    }
}

static void equalizerV2_init(MSFilter *f){
	f->data=equalizer_state_new(TAPS);
}

static void equalizerV2_uninit(MSFilter *f){
	equalizer_state_destroy((EqualizerState*)f->data);
}

static void equalizerV2_process(MSFilter *f){
	uint32_t nbytes = TAPS;
    mblk_t *m;
	EqualizerState *s=(EqualizerState*)f->data;
    while((m = ms_queue_get(f->inputs[0]))!=NULL){
        ms_bufferizer_put(s->buff,m);
    }
    
    while(ms_bufferizer_get_avail(s->buff)>=nbytes){
        mblk_t *out;
        out=allocb(nbytes,0);
        ms_bufferizer_read(s->buff,out->b_wptr,nbytes);
        OLS_Filtering(out->b_wptr, out->b_wptr, s->fft_cpx);
        out->b_wptr+=nbytes;
        ms_queue_put(f->outputs[0],out);
    }
}

static int equalizerV2_set_gain(MSFilter *f, void *data){
	EqualizerState *s=(EqualizerState*)f->data;
	MSEqualizerGain *d=(MSEqualizerGain*)data;
	equalizer_state_set(s,d->frequency,d->gain,d->width);
    
    // dump_table(s->fft_cpx,s->nfft);
	return 0;
}

static int equalizerV2_set_rate(MSFilter *f, void *data){
	EqualizerState *s=(EqualizerState*)f->data;
	s->rate=*(int*)data;
	return 0;
}

static MSFilterMethod equalizerV2_methods[]={
    {	MS_FILTER_SET_SAMPLE_RATE	,	equalizerV2_set_rate	},
	{	MS_EQUALIZERV2_SET_GAIN		,	equalizerV2_set_gain	},
	{	0				,	NULL			}
};

#ifdef _MSC_VER

MSFilterDesc ms_equalizerV2_desc={
	MS_EQUALIZERV2_ID,
	"MSEqualizerV2",
	N_("Parametric sound equalizerV2."),
	MS_FILTER_OTHER,
	NULL,
	1,
	1,
	equalizerV2_init,
	NULL,
	equalizerV2_process,
	NULL,
	equalizerV2_uninit,
	equalizerV2_methods
};

#else

MSFilterDesc ms_equalizerV2_desc={
	.id= MS_EQUALIZERV2_ID,
	.name="MSEqualizerV2",
	.text=N_("Parametric sound equalizerV2."),
	.category=MS_FILTER_OTHER,
	.ninputs=1,
	.noutputs=1,
	.init=equalizerV2_init,
	.process=equalizerV2_process,
	.uninit=equalizerV2_uninit,
	.methods=equalizerV2_methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_equalizerV2_desc)
