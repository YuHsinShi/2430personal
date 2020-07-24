/*
    this module is used by OLS_Filtering() for equalizer.
*/
#include <math.h>
#include <memory.h>
#include <stdio.h>

#define TAPS 512
#define GAIN_ZERODB 20000

typedef struct _EqualizerGain{
	int frequency; ///< In hz
	float gain; ///< between 0-1.2
	int width; ///< frequency band width around mid frequency for which the gain is applied, in Hz. Use 0 for the lowest frequency resolution.
}EqualizerGain;

typedef struct _EqualizerState{
	int rate;
	short fft_cpx[TAPS];
} EqualizerState;

static EqualizerState s;
extern void OLS_Filtering(int16_t *Sin, int16_t *Sout, void *cpx);

static void equalizer_state_flatten(EqualizerState *s){
	int i;
	short val=GAIN_ZERODB/TAPS;
    memset(s->fft_cpx, 0, sizeof(s->fft_cpx));
	s->fft_cpx[0]=val;
	for(i=1;i<TAPS;i+=2)
		s->fft_cpx[i]=val;
}

static int equalizer_state_hz_to_index(EqualizerState *s, int hz){
	int ret;
	if (hz<0){
		return -1;
	}
	if (hz>(s->rate/2)){
		hz=(s->rate/2);
	}
	/*round to nearest integer*/
	ret=((hz*TAPS)+(s->rate/2))/s->rate;
	if (ret==TAPS/2) ret=(TAPS/2)-1;
	return ret;
}

static int equalizer_state_index2hz(EqualizerState *s, int index){
	return (index * s->rate + TAPS/2) / TAPS;
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
	while (i < TAPS/2 && (gain>1.1 || gain<0.9));
	i = mid;
	do {	/* ... and here +delta, as to  */
		i--;
		f = equalizer_state_index2hz(s, i);
		gain = equalizer_compute_gainpoint(f+delta_f, freq_0, sqrt_gain, freq_bw);
		equalizer_point_set(s, i, f, gain);
	}
	while (i>=0 && (gain>1.1 || gain<0.9));
}

static void dump_table(short *t, int len){
	int i;
	for(i=0;i<len;i++) {
        if(len%8==0) printf("\n");
		printf("[%i] %i\t",i,t[i]);
    }
    printf("\n");
}

static void equalizer_set_gain(EqualizerGain *data){
	EqualizerGain *d=data;
	equalizer_state_set(&s, d->frequency, d->gain, d->width);
    // dump_table(s.fft_cpx,TAPS);
}

//======= Interface =======

void audioEqualizerSetRate(int data){
	s.rate=data;
    equalizer_state_flatten(&s);
}

void audioEqualizerInit(int data){
    audioEqualizerSetRate(data);
    // this is init set gain by Kconfig saved. if setting value from another way, this section would be modified.
    // section start
    const char *SPKgains=CFG_AUDIO_EQUALIZER_GAIN_SET;
    if (SPKgains){
        do{
            int bytes;
            EqualizerGain g;
            if (sscanf(SPKgains,"%d:%f:%d %n",&g.frequency,&g.gain,&g.width,&bytes)==3){
                // printf("Read audio equalizer gains: %d(~%d) --> %f\n",g.frequency,g.width,g.gain);
                equalizer_set_gain(&g);
                SPKgains+=bytes;
            }else break;
        }while(1);
    }
    // section end
    // End set gain by Kconfig saved.
}

void audioEqualizerProcess(void *Sin, void *Sout){
    OLS_Filtering(Sin, Sout, s.fft_cpx);
}

void audioEqualizerSetGain(int frequency, float gain, int width){
    EqualizerGain g;
    g.frequency = frequency;
    g.gain = gain;
    g.width = width;    
    equalizer_set_gain(&g);
}