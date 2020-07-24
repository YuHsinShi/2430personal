#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ite/ith.h"
#include "ite/itp.h"

#include "ite970/IPGD_table.inc"
#include "ite970/IPGA_table1.inc" //[-27dB(0x0)~-9dB(0x12)]
//#include "ite970/IPGA_table2.inc"   //[-27dB(0x0)~ 0dB(0x1B)]
//#include "ite970/IPGA_table.inc"  //[-27dB(0x0)~36dB(0x3F)]

//#difine FARADAY_DEBUG_PROGRAM_REGVALUE
//#define DEBUG_PRINT printf
#define DEBUG_PRINT(...)

#define I2S_REG_BASE                0xD0100000

/* ************************************************************************** */
#define MAX_OUT_VOLUME           0x3F
#define MIN_OUT_VOLUME           0x10
static uint8_t current_volstep      = MAX_OUT_VOLUME;
static uint8_t current_volperc      = 99;
static uint8_t current_volstepRL[2] = {MAX_OUT_VOLUME,MAX_OUT_VOLUME};
static uint8_t current_volpercRL[2] = {99,99};

#define MAX_IN_VOLUME            0x12
#define MIN_IN_VOLUME            0x00
static uint8_t current_micstep      = MAX_IN_VOLUME;
static uint8_t current_micperc      = 99;
static uint8_t current_micstepRL[2] = {MAX_IN_VOLUME,MAX_IN_VOLUME};
static uint8_t current_micpercRL[2] = {99,99};

static pthread_mutex_t FARADAY_MUTEX         = PTHREAD_MUTEX_INITIALIZER;

static uint8_t  _faraday_in  = -1;
static uint8_t  _faraday_out = -1;
static bool _faraday_DA_running = 0;
static bool _faraday_AD_running = 0;
static int  _faraday_sampleRate = 48000;

typedef enum _OutPath {
	HPRpathOnly = 0,
	HPLpathOnly,
	HPRLpathBoth,
    SKPpath
}OutPath;

typedef enum _InPath {
	MicRpathOnly = 0,
	MicLpathOnly,
	MicRLpathBoth,
}InPath;

/* ************************************************************************** */

static void faraday_write_reg(unsigned char reg_addr, unsigned int value)
{
#ifdef FARADAY_DEBUG_PROGRAM_REGVALUE
    printf("FARADAY# write reg[0x%02x] = 0x%08X\n", reg_addr, value);
#endif
    ithWriteRegA(I2S_REG_BASE|reg_addr, value);
}

static unsigned int faraday_read_reg(unsigned char reg_addr)
{
    unsigned int value = ithReadRegA(I2S_REG_BASE|reg_addr);
#ifdef FARADAY_DEBUG_PROGRAM_REGVALUE
    printf("FARADAY# read reg[0x%02x] = 0x%08X\n", reg_addr, value);
#endif
    return value;
}

static void _dump_all_reg(void){

    printf("FARADAY# read reg[0x%02x] = 0x%08X\n", 0xD0, ithReadRegA(I2S_REG_BASE|0xD0));
    printf("FARADAY# read reg[0x%02x] = 0x%08X\n", 0xD4, ithReadRegA(I2S_REG_BASE|0xD4));
    printf("FARADAY# read reg[0x%02x] = 0x%08X\n", 0xD8, ithReadRegA(I2S_REG_BASE|0xD8));
    printf("FARADAY# read reg[0x%02x] = 0x%08X\n", 0xDC, ithReadRegA(I2S_REG_BASE|0xDC));
    printf("FARADAY# read reg[0x%02x] = 0x%08X\n", 0xE0, ithReadRegA(I2S_REG_BASE|0xE0));
    printf("FARADAY# read reg[0x%02x] = 0x%08X\n", 0xE4, ithReadRegA(I2S_REG_BASE|0xE4));
    printf("FARADAY# read reg[0x%02x] = 0x%08X\n", 0xE8, ithReadRegA(I2S_REG_BASE|0xE8));
    printf("FARADAY# read reg[0x%02x] = 0x%08X\n", 0xEC, ithReadRegA(I2S_REG_BASE|0xEC));
    printf("FARADAY# read reg[0x%02x] = 0x%08X\n", 0xF0, ithReadRegA(I2S_REG_BASE|0xF0));
    printf("FARADAY# read reg[0x%02x] = 0x%08X\n", 0xF4, ithReadRegA(I2S_REG_BASE|0xF4));
    printf("_faraday_micin   = %d -1:NULL\n",_faraday_in);
    printf("_faraday_spkout  = %d -1:NULL\n",_faraday_out);
}

static inline void faraday_writeRegMask(unsigned char addr, unsigned int data, unsigned int mask)
{
    faraday_write_reg(addr, (faraday_read_reg(addr) & ~mask) | (data & mask));
}

static void _reset_all_reg(void)
{
    faraday_write_reg(0xD0,0x00000CCF);
    faraday_write_reg(0xD4,0x31181004);
    faraday_write_reg(0xD8,0x00000005);
    faraday_write_reg(0xDC,0x00000000);
    faraday_write_reg(0xE0,0x00003939);
    faraday_write_reg(0xE4,0x61611B1B);
    faraday_write_reg(0xE8,0x00003F3F);
    faraday_write_reg(0xEC,0x000000E0);
    faraday_write_reg(0xF0,0x3F3F1000);
    faraday_write_reg(0xF4,0x00002323);    
}

static void amp_ctrol(int mute){
#if 0
    if(mute)
        ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_MUTE,   NULL); //AMP disable
    else
        ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_UNMUTE, NULL); //Amp enable
#endif
}

static void _output_path_mute(int mute){
    
    amp_ctrol(mute);
    if(mute) {faraday_writeRegMask(0xD4,0x3 << 19,0x3 << 19); return ;}
    
    switch (_faraday_out)
    {
    case HPRpathOnly:     
        {/* HP R only */
            faraday_writeRegMask(0xD4,0x1 << 19,0x3 << 19);
            break;
        }
    case HPLpathOnly:     
        {/* HP L only */
            faraday_writeRegMask(0xD4,0x2 << 19,0x3 << 19);
            break;
        }
    case HPRLpathBoth:     
        {/* HP RL both */
            faraday_writeRegMask(0xD4,0x0 << 19,0x3 << 19);
            break;
        }
    case SKPpath :
        {/* Speaker out*/
            break;
        }
    default:
        {/* HP RL both */
            faraday_writeRegMask(0xD4,0x0 << 19,0x3 << 19);
            break;
        }
    }    
}

static void _input_path_mute(int mute){
    
    if(mute) {faraday_writeRegMask(0xD4,0x3 << 21,0x3 << 21); return ;}
          
    switch (_faraday_in)
    {
    case MicRpathOnly:     
        {/* mic R only */
            //faraday_writeRegMask(0xE4, 0x6B<<24, 0x3F<<24);//0x6B(+10dB)
            faraday_writeRegMask(0xD4,0x1 << 21,0x3 << 21);
            break;
        }
    case MicLpathOnly:     
        {/* mic L only */
            //faraday_writeRegMask(0xE4, 0x6B<<24, 0x3F<<24);//0x6B(+10dB)
            faraday_writeRegMask(0xD4,0x2 << 21,0x3 << 21);
            break;
        }
    case MicRLpathBoth:     
        {/* mic RL Both */
            faraday_writeRegMask(0xD4,0x0 << 21,0x3 << 21);
            break;
        }
    default:
        {/* mic RL Both */
            faraday_writeRegMask(0xD4,0x0 << 21,0x3 << 21);
            break;
        }
    }
    
}

static void _internal_DAC_reset(void)
{

    unsigned int data;
    DEBUG_PRINT("faraday# %s\n", __func__);

    if (_faraday_DA_running)
    {
        DEBUG_PRINT("faraday# DAC is running, skip re-init process !\n");
        return;
    }
    if (_faraday_AD_running)
    {
        DEBUG_PRINT("faraday# ADC is running, skip re-init process !\n");
        return;
    } 

    //_reset_all_reg();

    /*SYSTEM control*/
    data = 0x0  |(0x0 << 14 ) //Data in slect                   [14:15]:LINEin(0x3) MICin  (0x0)
                |(0x0 << 12 ) //?                               [12:16]:
                |(0x3 << 10 ) //HPOUT Enable(LR)                [10:11]:Enable(0x3) Disable(0x0) (0x1)OnlyHPL (0x2)OnlyHPR 
                |(0x0 <<  8 ) //ADC front-end power control(LR) [ 8: 9]:Enable(0x0) Disable(0x3) (0x1)OnlyR   (0x2)OnlyL
                |(0x0 <<  6 ) //DAC power-down control(LR)      [ 6: 7]:Enable(0x0) DIsable(0x3) (0x1)OnlyR   (0x2)OnlyL
                |(0x0 <<  4 ) //ADC power-down control(LR)      [ 4: 5]:Enable(0x0) DIsable(0x3) (0x1)OnlyR   (0x2)OnlyL
                |(0xF <<  0 ); //MCLK freequency :              [ 0: 3]
    faraday_write_reg(0xD0,data);

    /*AUDIO control */
    data = 0x0  |(0x1 << 31 )  //ALC zero-crocess                   :(0x1) on
                |(0x3 << 29 )  //ADC digital modulator gain  [29:30]:0x0 +1dB,0x1 +1.5dB ,0x2 +2dB ,0x3 +4dB
                |(0x1 << 28 )  //ADC moudlator DWA control          :Enable(0x1) Disable(0x0)
                |(0x0 << 27 )  //ADC moudlator DEM control          :Enable(0x1) Disable(0x0)
                |(0x1 << 26 )  //Rmic  boost                        :(0x1) +20dB ,(0x0) +0dB
                |(0x1 << 25 )  //Lmic  boost                        :(0x1) +20dB ,(0x0) +0dB
                |(0x0 << 24 )  //speaker driver power               :down(0x1) normal(0x0)
                |(0x0 << 23 )  //mic     driver power               :down(0x1) normal(0x0)
                |(0x0 << 21 )  //ADC mute(LR)                [21:22]:mute(0x3) unmute(0x0)
                |(0x3 << 19 )  //DAC mute(LR)                [19:20]:mute(0x3) unmute(0x0)
                |(0x0 << 18 )  //DAC mix control RL channels        :Enable(0x1) DIsable(0x0)
                |(0x0 << 16 )  //DAC mix invert              [16:17]:0x00 0x01 0x2 0x3
                |(0x1 << 15 )  //ADC HPF                            :Enable(0x1) Disable(0x0)
                |(0x0 << 14 )  //ALC function                       :Enable(0x1) Disable(0x0)
                |(0x1 << 12 )  //DA IN mode slect            [12:13]:0x00 0x01(I2S) 0x02 0x03
                |(0x0 << 10 )  //DAC feeback to ADC          [10:11]:0x0;
                |(0x0 <<  8 )  //de emphasis                   [8:9]:0x0;
                |(0x0 <<  7 )  //DAC anti pop                       :enable (0x0) Disable(0x1);
                |(0x0 <<  5 )  //LINEIN bypass to HPout(LR)         :Enable(0x3) DIsable(0x0)
                |(0x1 <<  4 )  //hp VCM driver power                :down(0x1) normal(0x0)
                |(0x0 <<  3 )  //Depop(internal test only)          :
                |(0x4 <<  0 ); //reference current control     [0:2]:0x4
    faraday_write_reg(0xD4,data);
    
    /*ALC control 1 main set*/
    data = 0x0  |(0x3 << 19 )  //ALC hold time               [19:22]:(0x0)0us ,(0x1)250us,(0x2) 500us,(0x3) 1000us
                |(0x0 << 16 )  //ALC noise gate threshold    [16:18]:(0x7) -36dB ~ (0x0)  -78dB  [6dB decay]
                |(0x7 << 11 )  //minimum PGA gain            [11:13]:(0x7) - 6dB ~ (0x0)  -27dB  [3dB decay]
                |(0x0 <<  8 )  //maximun PGA gain            [ 8:10]:(0x7)  36dB ~ (0x0)   -6dB  [6dB decay]
                |(0xF <<  4 )  //ALC target level            [ 4: 7]:(0xF)-4dBFS ~ (0x0)-34dBFS  [2db decay]
                |(0x0 <<  3 )  //DAC DWA mode control               : Enable(0x1) DIsable(0x0)
                |(0x1 <<  2 )  //DAC DEM mode control               : Enable(0x1) DIsable(0x0)
                |(0x1 <<  0 ); //DAC SDMGAIN                 [ 0: 1]: (0x0)-1db (0x1)-2db (0x2)-3db (0x3)-6db
    faraday_write_reg(0xD8,data);    
    
    /*ALC control 2 sub main set*/
    data = 0x0  |(0x1 <<  8 )  //ALC attack time setting     [8:11]:(0x0)62.25 us ,double when add 1 bit
                |(0x1 <<  0 ); //ALC decay time              [ 0:3]:(0x0)104   us ,double when add 1 bit
    faraday_write_reg(0xDC,data);   
    
    /*volume gain*/
    data = 0x0  |(0x39 << 16)  //SPK   analog gain control          [16:21]:(0x3F) +6dB ~ (0x11) -40dB [1dB decay] (0x39) +0dB
                |(0x39 << 8 )  //HP(R) analog gain control          [ 8:13]:(0x3F) +6dB ~ (0x11) -40dB [1dB decay] (0x39) +0dB
                |(0x39 << 0 ); //HP(L) analog gain control          [ 0: 5]:(0x3F) +6dB ~ (0x11) -40dB [1dB decay] (0x39) +0dB

    faraday_write_reg(0xE0,data);    
    
   /*ADC control gain*/
    data = 0x0  |(0x7F << 24)  //ADC(R) digital gain control [24:30]:(0x7F) +30dB ~ (0x11) -50dB [1dB decay] (0x61) +0dB
                |(0x7F << 16)  //ADC(L) digital gain control [16:22]:(0x7F) +30dB ~ (0x11) -50dB [1dB decay] (0x61) +0dB
                |(0x1B <<  8)  //ADC(R) analog  gain control [ 8:13]:(0x3F) +36dB ~ (0x00) -27dB [1dB decay] (0x1B) +0dB
                |(0x1B <<  0); //ADC(L) analog  gain control [ 0: 5]:(0x3F) +36dB ~ (0x00) -27dB [1dB decay] (0x1B) +0dB
    faraday_write_reg(0xE4,data);    

    /*DAC control gain*/ //0x00003F3F :0x1B1B3F3F?
    data = 0x0  |(0x3F <<  8)  //DAC(R) digital gain control [ 8:13]:(0x3F) +0dB ~ (0x11) -40dB [1dB decay] (0x3F) +0dB
                |(0x3F <<  0); //DAC(L) digital gain control [ 0: 5]:(0x3F) +0dB ~ (0x11) -40dB [1dB decay] (0x3F) +0dB
    faraday_write_reg(0xE8,data);
    
    /*AUDIO control*/
    data = 0x0  |(0x0 << 11)  //chopper stabilize chontrol(L) :(0x0) off
                |(0x0 << 10)  //chopper stabilize chontrol(R) :(0x0) off
                |(0x1 << 9 )  //LINE out power(L/R)           :(0x1) off
                |(0x1 << 8 )  //LINE mute (L/R)               :(0x1) mute
                |(0x1 << 7 )  //mic input selection mode(R)   :(0x1)single-end ,(0x0)differential
                |(0x1 << 6 )  //mic input selection mode(L)   :(0x1)single-end ,(0x0)differential
                |(0x4 << 3 )  //IRSEL_HP?                [3:5]:(0x4)default
                |(0x0 << 2 )  //bias mode control             :(0x1) disable
                |(0x0 << 1 )  //HP(R) power                   : 0x1 off
                |(0x0 << 0 ); //HP(L) power                   : 0x1 off
    faraday_write_reg(0xEC,data);    

    /*SOFT Mute control control 1*/
    data = 0x0  |(0x3F << 24)  //DAC soft-mute MAX digital control(R) [16:21]:(0x3F) 0dB ~ (0x23) -40dB (1dB decay)
                |(0x3F << 16)  //DAC soft-mute MAX digital control(L) [16:21]:(0x3F) 0dB ~ (0x23) -40dB (1dB decay)
                |(0x0  << 11)  //DAC soft-mute gain step              [11:12]:(0x0) 1dB ~(0x3) 8dB
                |(0x0  << 8 )  //DAC soft mute time setp              [ 8:10]:(0x0)1ms ~ 0x7(128ms) (double change value*2)
                |(0x0  << 2 )  //DAC soft-mute zero cross                    :(0x1)zero crocess on
                |(0x0  << 1 )  //DAC fading (in/out)                         :(0x1)setp down to mute (0x0) step up to current digital
                |(0x0  << 0 ); //DAC soft-mute                               :(0x1)mute
    faraday_write_reg(0x0F0,data);        
    
    /*SOFT Mute control control 2*/
    data = 0x0  |(0x0 << 8)  //DAC soft-mute min digital control(R) [8:13]:(0x3F) 0dB ~ (0x23) -40dB (1dB decay)
                |(0x0 << 0); //DAC soft-mute min digital control(L) [0: 5]:(0x3F) 0dB ~ (0x23) -40dB (1dB decay)
    faraday_write_reg(0xF4,data);
    
    //#ifdef CFG_CHIP_PKG_IT976
    //faraday_writeRegMask(0xEC,0x3 << 6,0x3 << 6); //set input as single-end
    //#endif
#ifndef CFG_CHIP_FT
#ifdef CFG_RIGHT_CHANNEL_AS_DEFAULT
    faraday_writeRegMask(0xEC, 0x0 << 6,0x3 << 6);//set micIN differential
#else
    faraday_writeRegMask(0xD4, 0x1<<18 | 0x1<<16, 0x1<<18 | 0x3<<16);//mix RL & invert L(0x1) R(0x2)[17:16]
#endif
#endif
}


/* ************************************************************************** */
/* === common DAC/ADC ops === */
void itp_codec_depop(void){
    ithWriteRegA(0xD800003c,0x02002804); //
    ithWriteRegA(0xD01000D0,0x0000000F); // DAC depop (for DAC faraday)    
}

void itp_codec_wake_up(void)
{
    amp_ctrol(0);
    if(!_faraday_DA_running) {
        ithWriteRegA(0xD800003C, 0x02A88800);//start clock
        ithWriteRegA(0xD8000040, 0x0002a801);//start clock    
    }
}

void itp_codec_standby(void)
{
    ithWriteRegA(0xD800003c,0x02008800); //enable W15CLK
    faraday_writeRegMask(0xE0, MIN_OUT_VOLUME<<8 | MIN_OUT_VOLUME<<0, 0x3F<<8 | 0x3F<<0);//DAC analog(R|L) HP out
    faraday_writeRegMask(0xE4, MIN_IN_VOLUME<<8 | MIN_IN_VOLUME<<0, 0x3F<<8 | 0x3F<<0);//ADC analog gain(R|L)
    faraday_write_reg(0xD0,0x000003FF); //HP out off[10:11] ADC front-end power control off [8:9] DAC power off [6:7] ADC power off [4:5]
    faraday_write_reg(0xD4,0x61F81010); //reference current control [0:2]   
    //faraday_write_reg(0xEC,0x000003C3); //HP power off[0:1]
    _faraday_DA_running = 0;
    _faraday_AD_running = 0;    
    ithWriteRegA(0xD800003C, 0x00008800);//reset clock
    ithWriteRegA(0xD8000040, 0x0000a801);//reset clock
    amp_ctrol(1);
}

/* DAC */
void itp_codec_playback_init(unsigned output)
{
    printf("faraday# %s _faraday_out = %d \n", __func__,output);
    
    _internal_DAC_reset();
    
    _faraday_out = output;
	_output_path_mute(0);
    faraday_writeRegMask(0xE0, current_volstep<<8 | current_volstep<<0, 0x3F<<8 | 0x3F<<0);//DAC analog(R|L) HP out
    current_volstepRL[0] = current_volstepRL[1] = current_volstep;
    current_volpercRL[0] = current_volpercRL[1] = current_volperc;
    _faraday_DA_running = 1;
    amp_ctrol(1);
    //_dump_all_reg();
}

void itp_codec_playback_deinit(void)
{
    if(_faraday_DA_running){
        faraday_writeRegMask(0xE0, MIN_OUT_VOLUME<<8 | MIN_OUT_VOLUME<<0, 0x3F<<8 | 0x3F<<0);//DAC analog(R|L) HP out
        _faraday_DA_running = 0;
    }
    amp_ctrol(0);
}

void itp_codec_playback_set_direct_vol(unsigned int volstep)
{// set volstep to register;

	if(volstep > MAX_OUT_VOLUME) {
		printf("ERROR# invalid target volume step: 0x%08x\n", volstep);
		return;
	}
	if(volstep == current_volstep) {
		return;
	}
    current_volstep = volstep;
    
    if(_faraday_DA_running){  
    /* IPGA_table.inc*/
        faraday_writeRegMask(0xE0, volstep<<8 | volstep<<0, 0x3F<<8 | 0x3F<<0);//DAC analog(R|L) HP out
    //    faraday_writeRegMask(0xE0, volstep<<16, 0x3F<<16);//DAC analog spk out
    }
    
}

void itp_codec_playback_set_direct_volperc(unsigned target_volperc)
{   // precent to step :ex  82% -> 0x90 
	unsigned int volstep;
	
    if(target_volperc >= 100) { target_volperc = 99; }

	volstep = it970DA_perc_to_reg_table[target_volperc];
	itp_codec_playback_set_direct_vol(volstep);    
    current_volperc = target_volperc;
    
    DEBUG_PRINT("FARADAY# %s target_volperc = %d(0x%02x) \n", __func__,target_volperc,volstep);
}

void itp_codec_playback_get_currvol(unsigned *currvolperc)
{
	*currvolperc = current_volperc;
}

void itp_codec_playback_set_direct_RLvol(unsigned int volstep,unsigned char RL)
{
    current_volstepRL[RL] = volstep;  
    if(_faraday_DA_running){  
        switch(RL){
            case   0:faraday_writeRegMask(0xE0, volstep<<8, 0x3F<<8);break;
            case   1:faraday_writeRegMask(0xE0, volstep<<0, 0x3F<<0);break;
            default :faraday_writeRegMask(0xE0, volstep<<8|volstep<<0, 0x3F<<8 | 0x3F<<0);break;
        }
    }
}

void itp_codec_playback_set_direct_RLvolperc(unsigned target_volperc,unsigned char RL)
{   // precent to step :ex  82% -> 0x90 
	unsigned int volstep;
	if( RL!=0 && RL!=1) {printf("error vol RL(%d) set\n",RL); return;}
    if(target_volperc >= 100) { target_volperc = 99; }

	volstep = it970DA_perc_to_reg_table[target_volperc];
	itp_codec_playback_set_direct_RLvol(volstep,RL);    
    current_volpercRL[RL] = target_volperc;
    
    printf("FARADAY# %s target_volperc[%d] = %d(0x%02x) \n", __func__,RL,target_volperc,volstep);
}

void itp_codec_playback_get_RLcurrvol(unsigned *currvolperc,unsigned char RL)
{
	*currvolperc = current_volpercRL[RL];
}

void itp_codec_playback_mute(void)
{
    DEBUG_PRINT("FARADAY# %s \n", __func__);
    if(_faraday_DA_running)
        _output_path_mute(1);
}

void itp_codec_playback_unmute(void)
{
    DEBUG_PRINT("FARADAY# %s \n", __func__);

    if (current_volstep == MIN_OUT_VOLUME)
    {   // vol = min (mute volume)
        _output_path_mute(1);
        return;
    }    
    
    if(_faraday_DA_running)
        _output_path_mute(0);
}

/* ADC */
void itp_codec_rec_init(unsigned input_source)
{
	printf("faraday# %s _faraday_in = %d \n", __func__,input_source);
    
	_internal_DAC_reset();
    
    _faraday_in = input_source;
    _input_path_mute(0);
    faraday_writeRegMask(0xE4, current_micstep<<8 | current_micstep<<0, 0x3F<<8 | 0x3F<<0);//ADC analog gain(R|L)
    current_micstepRL[0] = current_micstepRL[1] = current_micstep;
    current_micpercRL[0] = current_micpercRL[1] = current_micperc;
	_faraday_AD_running = 1;
    
    //_dump_all_reg();
    
}

void itp_codec_rec_deinit(void)
{
    DEBUG_PRINT("FARADAY# %s \n", __func__);
    if(_faraday_AD_running){
        faraday_writeRegMask(0xE4, MIN_IN_VOLUME<<8 | MIN_IN_VOLUME<<0, 0x3F<<8 | 0x3F<<0);//ADC analog gain(R|L)
        _faraday_AD_running = 0;
    }
}

void itp_codec_rec_set_direct_vol(unsigned int micstep)
{// set to register;

	if(micstep > MAX_IN_VOLUME) {
		printf("ERROR# invalid target volume step: 0x%08x\n", micstep);
		return;
	}

	if(micstep == current_micstep) return;
    
    current_micstep = micstep;
    
    if(_faraday_AD_running){
        /*IPGA_table.inc*/
        faraday_writeRegMask(0xE4, micstep<<8 | micstep<<0, 0x3F<<8 | 0x3F<<0);//ADC analog gain(R|L)
    }
}

void itp_codec_rec_set_direct_volperc(unsigned target_micperc)
{   // precent to step :ex  82% -> 0x90 
	unsigned int micstep;
	if(target_micperc >= 100) { target_micperc = 99; }

	micstep = it970AD_perc_to_reg_table[target_micperc];
    itp_codec_rec_set_direct_vol(micstep);
    current_micperc = target_micperc;
    
    DEBUG_PRINT("FARADAY# %s target_micperc = %d(0x%02x) \n", __func__,target_micperc,micstep);
}

void itp_codec_rec_get_currvol(unsigned *currvol)
{
    *currvol = current_micperc;
}

void itp_codec_rec_set_direct_RLvol(unsigned int micstep,unsigned char RL)
{// set to register;
	if(micstep == current_micstepRL[RL]) return;
    
    current_micstepRL[RL] = micstep;
    
    if(_faraday_AD_running){
        switch(RL){
            case   0:faraday_writeRegMask(0xE4, micstep<<8, 0x3F<<8);break;
            case   1:faraday_writeRegMask(0xE4, micstep<<0, 0x3F<<0);break;
            default :faraday_writeRegMask(0xE4, micstep<<8|micstep<<0, 0x3F<<8 | 0x3F<<0);break;
        }
    }
}

void itp_codec_rec_set_direct_RLvolperc(unsigned target_micperc,unsigned char RL)
{   // precent to step :ex  82% -> 0x90 
	unsigned int micstep;
    if( RL!=0 && RL!=1) {printf("error rec RL(%d) set\n",RL); return;}
	if(target_micperc >= 100) { target_micperc = 99; }

	micstep = it970AD_perc_to_reg_table[target_micperc];
    itp_codec_rec_set_direct_RLvol(micstep,RL);
    current_micpercRL[RL] = target_micperc;
    
    printf("FARADAY# %s target_micperc[%d] = %d(0x%02x) \n", __func__,RL,target_micperc,micstep);
}

void itp_codec_rec_get_RLcurrvol(unsigned *currvol,unsigned char RL)
{
    *currvol = current_micpercRL[RL];
}

void itp_codec_rec_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
	*max         = MAX_IN_VOLUME;
	*regular_0db = current_micstep; //current step;
	*min         = MIN_IN_VOLUME;
}

void itp_codec_rec_mute(void)
{
    DEBUG_PRINT("FARADAY# %s \n", __func__);
    if(_faraday_AD_running)
        _input_path_mute(1);
}

void itp_codec_rec_unmute(void)
{
    DEBUG_PRINT("FARADAY# %s \n", __func__);
    if(_faraday_AD_running)
        _input_path_mute(0);
}

void itp_codec_power_on(void)
{
    DEBUG_PRINT("FARADAY# %s \n", __func__);
    faraday_writeRegMask(0xD0,0x0 << 4,0xF << 4);//DA(R/L) AD(R/L) power on
    faraday_writeRegMask(0xEC,0x0 << 0,0x3 << 0);//HP(R/L) power on
}

void itp_codec_power_off(void)
{
    DEBUG_PRINT("FARADAY# %s \n", __func__);
    faraday_writeRegMask(0xD0,0xF << 4,0xF << 4);//DA(R/L) AD(R/L) power down
    faraday_writeRegMask(0xEC,0x3 << 0,0x3 << 0);//HP(R/L) power down
}

void itp_codec_set_i2s_sample_rate(int samplerate){
    
    switch (samplerate)
    {
        case 48000: faraday_writeRegMask(0xD0, 0xF << 0, 0xF << 0); break;//256x
        case 44100: faraday_writeRegMask(0xD0, 0xF << 0, 0xF << 0); break;//256x
        case 32000: faraday_writeRegMask(0xD0, 0x7 << 0, 0xF << 0); break;//384x
        case 24000: faraday_writeRegMask(0xD0, 0x5 << 0, 0xF << 0); break;//500x
        case 22050: faraday_writeRegMask(0xD0, 0x8 << 0, 0xF << 0); break;//544x
        case 16000: faraday_writeRegMask(0xD0, 0x1 << 0, 0xF << 0); break;//750x
        case 12000: faraday_writeRegMask(0xD0, 0xB << 0, 0xF << 0); break;//1000x
        case 11025: faraday_writeRegMask(0xD0, 0x9 << 0, 0xF << 0); break;//1088x
        case  8000: faraday_writeRegMask(0xD0, 0x4 << 0, 0xF << 0); break;//1500x
        default:    faraday_writeRegMask(0xD0, 0x6 << 0, 0xF << 0); break;//250x
    }
	_faraday_sampleRate = samplerate;
}

void itp_codec_get_i2s_sample_rate(int* samplerate){
    *samplerate = _faraday_sampleRate;
}

int itp_codec_get_DA_running(void)
{
    return _faraday_DA_running;
}

int itp_codec_get_AD_running(void)
{
    return _faraday_AD_running;
}
