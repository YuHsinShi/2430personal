/* sy.chuang, 2012-0622, ITE Tech. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "ite/ith.h"
#include "ite/itp_codec.h"
#include "iic/mmp_iic.h"

#define WM8960_I2CADR   (0x34 >> 1)

#if CFG_DAC_port_I2C0
    #define IIC_PORT    IIC_PORT_0
#else
    #define IIC_PORT    IIC_PORT_1
#endif

//#define DEBUG_PRINT printf
#define DEBUG_PRINT(...)

//#define WM8960_DEBUG_I2C_COMMON_WRITE
//#define WM8960_DEBUG_PROGRAM_REGVALUE

/* ************************************************************************** */
/* wrapper */
static inline void i2s_delay_us(unsigned us) { ithDelay(us); }

/* ************************************************************************** */
#define MAX_OUT1_VOLUME             0x7F                                    /* +6 dB */
#define ZERO_DB_OUT1_VOLUME         0x79                                    /*  0 dB */
#define MIN_OUT1_VOLUME             0x30                                    /* -73dB */
static unsigned curr_out1_volume = ZERO_DB_OUT1_VOLUME;                     /* 0 dB, refer to R2, R3 */

#define MAX_INPUT_PGA_VOLUME        0x3F                                    /* +30   dB */
#define ZERO_DB_INPUT_PGA_VOLUME    0x17                                    /* +0    dB */
#define MIN_INPUT_PGA_VOLUME        0x00                                    /* -17.25dB */
static unsigned         curr_input_pga_volume   = ZERO_DB_INPUT_PGA_VOLUME; /* 0 dB, refer to R0, R1 */
static pthread_mutex_t  WM8960_MUTEX            = PTHREAD_MUTEX_INITIALIZER;

static uint8_t  _wm8960_in  = -1;
static uint8_t  _wm8960_out = -1;
static bool     _wm8960_DA_running      = 0;
static bool     _wm8960_AD_running      = 0;
static int      _wm8960_sampleRate      = 48000;

typedef enum _OutPath {
	LineOutOnly = 0,
	SpkOutOnly,
	LineSpkOutBoth,
}OutPath;

typedef enum _InPath {
	LineInOnly = 0,
	MicInOnly,
	LineMicInBoth,
}InPath;

void itp_codec_playback_mute(void);
/* ************************************************************************** */
#include "wm8960/DA_table.inc"
#include "wm8960/AD_table.inc"

/* ************************************************************************** */
static void I2C_common_write_byte(unsigned char RegAddr, unsigned char d)
{
    int success = 0;
    int flag;
    int retry   = 0;
    for (retry = 0; retry < 50; retry++)
    {
        if (0 == (flag = mmpIicSendData(IIC_PORT, IIC_MASTER_MODE, WM8960_I2CADR, RegAddr, &d, 1)))
        {
            success = 1;
#ifdef WM8960_DEBUG_I2C_COMMON_WRITE
            printf("WM8960# IIC WriteOK!\n");
#endif
            break;
        }
    }
    if (success == 0)
    {
        printf("WM8960# IIC Write Fail!\n");
        while (1) { usleep(500000); }
    }
}
static void wm8960_write_reg(unsigned char reg_addr, unsigned short value)
{
    unsigned char   A;
    unsigned char   B;

#ifdef WM8960_DEBUG_PROGRAM_REGVALUE
    printf("WM8960# reg[0x%02x] = 0x%04x\n", reg_addr, value);
#endif

    A   = ((reg_addr & 0x7F) << 1) | ((value >> 8) & 0x1);
    B   = (value & 0xFF);

    I2C_common_write_byte(A, B);
}

static void init_wm8960_common(void)
{
    unsigned short  data16;
    int             sample_rate;
    DEBUG_PRINT("WM8960# %s\n", __func__);

    if (_wm8960_DA_running)
    {
        DEBUG_PRINT("WM8960# DAC is running, skip re-init process !\n");
        return;
    }
    if (_wm8960_AD_running)
    {
        DEBUG_PRINT("WM8960# ADC is running, skip re-init process !\n");
        return;
    }

    pthread_mutex_lock(&WM8960_MUTEX);

    itp_codec_get_i2s_sample_rate(&sample_rate);
        
        /*reset register*/
        data16 = 0x0;
        wm8960_write_reg(15, data16);  //reset WM8960
        
        /*ADC DAC control 1*/
        data16 = 0x0 |(0x0  <<  7 )  //[    7] DAC 6db attenuate 0x0:0db 0x1:-6db
                     |(0x0  <<  5 )  //[ 6: 5] ADC polarity 0x0:not invert 0x1:ADCL invert 0x2:ADCLR invert 0x3:ADCLR invert 
                     |(0x0  <<  3 )  //[    3] DAC digital soft mute:0x0 : no mute
                     |(0x0  <<  1 )  //[ 2: 1] DE-emphasis 0x0:disable 0x1:32k 0x2:44k 0x3:48k
                     |(0x0  <<  0 ); //[    0] ADC HPF :0x0 enable
        wm8960_write_reg(5 , data16);  
        
        /*ADC DAC control 2*/
        data16 = 0x0 |(0x0  <<  5 )  //[ 6: 5] DAC polarity 0x0:not invert 0x1:ADCL invert 0x2:ADCLR invert 0x3:ADCLR invert :
                     |(0x0  <<  3 )  //[    3] DACsoft mute mode 0x0:immediately 0x1:gradually
                     |(0x0  <<  1 )  //[    2] DAC soft mute ramp 0x0:fast 0x1:slow
                     |(0x0  <<  0 ); //[    1] DAC filter 0x0:normal 0x1:sloping stop band
        wm8960_write_reg(6 , data16); 
        
        /*audio interface */
        data16 = 0x0 |(0x0  <<  8 )  //[    8] ADC RL channel swap 0x0:disable
                     |(0x0  <<  7 )  //[    7] BCLK invert bit 0x0:disable
                     |(0x0  <<  6 )  //[    6] wm8960 master/slave control 0x0:slave 0x1:master 
                     |(0x0  <<  5 )  //[    5] DAC RL channel swap 0x0:disable
                     |(0x0  <<  4 )  //[    4] RL i2s mode-LRCLK polarity 0x0:normal
                     |(0x3  <<  2 )  //[ 3: 2] audio word 0x0:16 0x1:20 0x2:23 0x3:32(bit)
                     |(0x2  <<  0 ); //[ 1: 0] 0x0:R justified 0x1:L justified 0x2 i2s format 0x3:DSP mode  
        wm8960_write_reg(7 , data16);     
        
        /*clock setting*/
        data16 = 0x0 |(0x6  <<  6 )  //[ 8: 6] Class D clk divider: 0x0:1.5 0x1:2 0x2:3 0x3:4 0x4:6 0x5:8 0x6:12 0x7:16
                     |(0x0  <<  0 ); //[ 3: 0] BCLK frequency(Master) 0x0 = sysclk  
        wm8960_write_reg(8 , data16);              

        /*Audio Interface*/
        data16 = 0x0 |(0x1  <<  6 )  //[    6] ADCLRC/GPIO1 pin select 0x0:ADCLRC cll for ADC ,0x1:GPIO 
                     |(0x0  <<  6 )  //[    5] 8-bit word length select 0x0:disable
                     |(0x0  <<  0 )  //[ 4: 3] DAC companding 0x0:off 0x2:u-law 0x3:a-law
                     |(0x0  <<  6 )  //[ 2: 1] ADC companding 0x0:off 0x2:u-law 0x3:a-law
                     |(0x0  <<  0 ); //[    0] loopback 0:disable ADC data output is fed to DAC input 
        wm8960_write_reg(9 , data16);
        
        /*Additional ctrl (1)*/
        data16 = 0x0 |(0x1  <<  8 )  //[    8] thermal shutdown 0x1:enable
                     |(0x3  <<  6 )  //[ 7: 6] analogue bias optimal 0x0:reserved 0x1:2.7V 0x3:3.3V 
                     |(0x0  <<  4 )  //[    4] DAC Mono Mix 0x0:stereo
                     |(0x0  <<  2 )  //[ 3: 2] ADC(R) output select 0x0:(L-L,R-R) 0x1:(L-L,R-L) 0x2:(L-R,R-R) 0x2:(L-R,R-L)
                     |(0x0  <<  1 )  //[    1] slow clock select 0x0:slow 0x1:fast
                     |(0x0  <<  0 ); //[    0] slow clock for vol update 0x0:disable 
        wm8960_write_reg(23 , data16);

        /*Additional ctrl (2)*/
        data16 = 0x0 |(0x0  <<  6 )  //[    6] HP switch enable 0x0:disable
                     |(0x0  <<  5 )  //[    5] HP switch polarity 0x0:headphone 0x1:speaker
                     |(0x0  <<  3 )  //[    3] TRIS control
                     |(0x0  <<  2 ); //[    2] LRC control
        wm8960_write_reg(24 , data16); 
        
        /*additional control*/
        data16 = 0x0 |(0x0  <<  6 )  //[    6] 0x0:500(ohm) VMID to output 0x1:20k(ohm)
                     |(0x0  <<  3 )  //[    3] 0x0:out3 unaffected by jack detect events 
                     |(0x0  <<  0 ); //[ 2: 0] ALC sr 0x0:44,48 0x1:32 0x2:22,24 0x3:16 0x4:11,12 0x5:8k 
        wm8960_write_reg(27 , data16); 
        
        /*MIC POWER CTRL*/
        data16 = 0x0 |(0x1  <<  5 )  //[    5] LMIC into FPGA   0x1:enable
                     |(0x1  <<  4 )  //[    4] RMIC into FPGA   0x1:enable
                     |(0x1  <<  3 )  //[    3] LOMIX out to MIX 0x1:enable
                     |(0x1  <<  2 ); //[    2] ROMIX out to MIX 0x1:enable
        wm8960_write_reg(47 , data16);
        
        /*ADCL INPUT PATH*/
        data16 = 0x0 |(0x1  <<  8 )  //[    8] LINPUT1 to left PGA 0x1:connect
                     |(0x0  <<  7 )  //[    7] LINPUT3 to left PGA 0x1:connect
                     |(0x1  <<  6 )  //[    6] LINPUT2 to left PGA 0x1:connect
                     |(0x1  <<  4 )  //[ 5: 4] LMIC boost gain 0x0:0db 0x1:13db 0x2:20db 0x3:29db
                     |(0x1  <<  3 ); //[    3] LMIC boost 0x1:connect
        wm8960_write_reg(32 , data16);       
        
        /*ADCR INPUT PATH*/
        data16 = 0x0 |(0x1  <<  8 )  //[    8] RINPUT1 to right PGA 0x1:connect
                     |(0x0  <<  7 )  //[    7] RINPUT3 to right PGA 0x1:connect
                     |(0x1  <<  6 )  //[    6] RINPUT2 to right PGA 0x1:connect
                     |(0x1  <<  4 )  //[ 5: 4] RMIC boost gain 0x0:0db 0x1:13db 0x2:20db 0x3:29db
                     |(0x1  <<  3 ); //[    3] RMIC boost 0x1:connect
        wm8960_write_reg(33 , data16);
        
        /*Left input volume*/
        data16 = 0x0 |(0x0  <<  8 )  //[    8] L PGA update 0x1:update RL
                     |(0x1  <<  7 )  //[    7] L input mute 0x1:mute
                     |(0x0  <<  6 )  //[    6] L zero cross 0x1 enable
                     |(0x17 <<  4 ); //[ 5: 0] L analog input gain 0x00:-17.25dB 0x17:0dB 0x3F:30dB
        wm8960_write_reg(0 , data16);
        
        /*Right input volume*/
        data16 = 0x0 |(0x0  <<  8 )  //[    8] R PGA update 0x1:update RL 
                     |(0x1  <<  7 )  //[    7] R input mute 0x1:mute
                     |(0x0  <<  6 )  //[    6] R zero cross 0x1 enable
                     |(0x17 <<  4 ); //[ 5: 0] R analog input gain 0x00:-17.25dB 0x17:0dB 0x3F:30dB
        wm8960_write_reg(1 , data16);  
		
        /*Left input booster mixer*/
        data16 = 0x0 |(0x0  <<  4 )  //[ 6: 4] LINPUT3 booster gain 0x0:mute 0x1:-12dB ~ 0x7:6dB (3dB increase) 
                     |(0x0  <<  1 ); //[ 3: 1] LINPUT2 booster gain 0x0:mute 0x1:-12dB ~ 0x7:6dB (3dB increase)
        wm8960_write_reg(43 , data16);
        
        /*Right input booster mixer*/
        data16 = 0x0 |(0x0  <<  4 )  //[ 6: 4] RINPUT3 booster gain 0x0:mute 0x1:-12dB ~ 0x7:6dB (3dB increase) 
                     |(0x0  <<  1 ); //[ 3: 1] RINPUT2 booster gain 0x0:mute 0x1:-12dB ~ 0x7:6dB (3dB increase)
        wm8960_write_reg(44 , data16);

         /*Left ADC vol*/
        data16 = 0x0 |(0x1  <<  8 )  //[    8] ADC(L) vol update
                     |(0xC3 <<  0 ); //[ 7: 0] ADC(L) digital vol 0x0:mute 0x1:-97db ~ 0xFF:30db (0.5db increase) 0xC3:0dB
        wm8960_write_reg(21 , data16);
        
        /*Right ADC vol*/
        data16 = 0x0 |(0x1  <<  8 )  //[    8] ADC(R) vol update
                     |(0xC3 <<  0 ); //[ 7: 0] ADC(R) digital vol 0x0:mute 0x1:-97db ~ 0xFF:30db (0.5db increase) 0xC3:0dB
        wm8960_write_reg(22 , data16); 
        
        /*input power*/
        data16 = 0x0 |(0x1  <<  7 )  //[ 8: 7] Vmid 0x0:disable 0x1:2*50k(ohm) 0x2:2*250k(ohm) 0x3:2*5k(ohm)
                     |(0x1  <<  6 )  //[    6] VREF 0x1 : up
                     |(0x3  <<  4 )  //[ 5: 4] analog PGA L/R 0x3:up 0x0:down
                     |(0x3  <<  2 )  //[ 3: 2] ADC L/R power  0x3:up 0x0:down
                     |(0x1  <<  1 )  //[    1] MICBIAS 0x1:enable 
                     |(0x0  <<  0 ); //[    0] master clock 0x0:disable
        wm8960_write_reg(25 , data16); 
 
        /*anti pop (1)*/
        data16 = 0x0 |(0x0  <<  7 )  //[    7] 0x0:VMID/R bias , 0x1:VGS/R bias
                     |(0x1  <<  4 )  //[    4] VGS/R current generator 0x1:enable
                     |(0x0  <<  3 )  //[    3] VGS/R current generator & analog in/out bias 0x0:disable
                     |(0x0  <<  2 )  //[    2] VMID soft start 0x0:disable
                     |(0x0  <<  0 ); //[    0] HP amp standby 0x0:disable
        wm8960_write_reg(28 , data16);  
        
        /*anti-pop (2)*/
        data16 = 0x0 |(0x1  <<  6 )  //[    6] discharge DC-blocking on HP L/R 0x1:Enable
                     |(0x0  <<  4 ); //[ 5: 4] resistance 0x0:400 0x1:200 0x2:600 0x3:150(ohm)
        wm8960_write_reg(29 , data16);    
        
        /*output power*/
        data16 = 0x0 |(0x3  <<  7 )  //[ 8: 7] DAC L/R 0x3:on
                     |(0x3  <<  5 )  //[ 6: 5] OUT L/R 0x3:on
                     |(0x3  <<  3 )  //[ 4: 3] SPK L/R 0x3:on
                     |(0x0  <<  1 )  //[    1] out3    0x1:on
                     |(0x0  <<  0 ); //[    0] PLL     0x1:enable
        wm8960_write_reg(26 , data16); 
        
        /*Left out mix*/
        data16 = 0x0 |(0x1  <<  8 )  //[    8] left DAC to left mixer 0x1:enable
                     |(0x0  <<  7 )  //[    7] Linput3  to left mixer 0x0:disable
                     |(0x5  <<  4 ); //[ 6: 4] Linput3  to left mixer vol 0x0:0db ~ 0x7:-21db (3db decay)
        wm8960_write_reg(34 , data16);  
        
        /*Right out mix*/
        data16 = 0x0 |(0x1  <<  8 )  //[    8] Right DAC to Right mixer 0x1:enable
                     |(0x0  <<  7 )  //[    7] Linput3  to Right mixer 0x0:disable
                     |(0x5  <<  4 ); //[ 6: 4] Linput3  to Right mixer vol 0x0:0db ~ 0x7:-21db (3db decay)
        wm8960_write_reg(37 , data16); 
        
        /*Lout1 vol*/
        data16 = 0x0 |(0x1  <<  8 )  //[    8] HP PGA update vol 0x1:
                     |(0x0  <<  7 )  //[    7] HP(L) zero cross 0x1:enable
                     |(0x79 <<  0 ); //[ 6: 0] Lout1 vol 0x3F:6db ~ 0x30:-73db (1db decay) else 0x2F~0x0:mute
        wm8960_write_reg(2 , data16); 
        
        /*Rout1 vol*/
        data16 = 0x0 |(0x1  <<  8 )  //[    8] HP PGA update vol 0x1:
                     |(0x0  <<  7 )  //[    7] HP(R) zero cross 0x1:enable
                     |(0x79 <<  0 ); //[ 6: 0] Rout1 vol 0x3F:6db ~ 0x30:-73db (1db decay) else 0x2F~0x0:mute
        wm8960_write_reg(3 , data16); 

        /*SPK L vol*/
        data16 = 0x0 |(0x1  <<  8 )  //[    8] SPK(L) PGA update vol 0x1:
                     |(0x0  <<  7 )  //[    7] SPK(L) zero cross 0x1:enable
                     |(0x79 <<  0 ); //[ 6: 0] SPK(L) vol 0x3F:6db ~ 0x30:-73db (1db decay) else 0x2F~0x0:mute
        wm8960_write_reg(40 , data16); 
        
        /*SPK L vol*/
        data16 = 0x0 |(0x1  <<  8 )  //[    8] SPK(R) PGA update vol 0x1:
                     |(0x0  <<  7 )  //[    7] SPK(R) zero cross 0x1:enable
                     |(0x79 <<  0 ); //[ 6: 0] SPK(R) vol 0x3F:6db ~ 0x30:-73db (1db decay) else 0x2F~0x0:mute
        wm8960_write_reg(41 , data16); 

        /*Class D control 1*/        
        data16 = 0x0 |(0x3  <<  6 )  //[ 7: 6] 0x0:off 0x1:L only 0x2:R only 0x3:RL
                     |(0x37 <<  0 ); //[ 5: 0] reserve 
        wm8960_write_reg(49 , data16); 
        
        /*Class D control 2*/
        data16 = 0x0 |(0x2  <<  6 )  //[ 8: 6] reserve 
                     |(0x4  <<  7 )  //[ 5: 3] DC booster 0x0:0 0x1:(+2.1db) 0x2(+2.9db) 0x3(+3.6db) 0x4(+4.5db) 0x5(+5.1db)
                     |(0x4  <<  0 ); //[ 2: 0] AC booster 0x0:0 0x1:(+2.1db) 0x2(+2.9db) 0x3(+3.6db) 0x4(+4.5db) 0x5(+5.1db)
        wm8960_write_reg(51 , data16); 

    pthread_mutex_unlock(&WM8960_MUTEX);
}

static void deinit_wm8960_common(void)
{

    DEBUG_PRINT("WM8960# %s\n", __func__);

    if (_wm8960_DA_running)
    {
        DEBUG_PRINT("WM8960# DAC is running, skip deinit !\n");
        return;
    }
    if (_wm8960_AD_running)
    {
        DEBUG_PRINT("WM8960# ADC is running, skip deinit !\n");
        return;
    }
    pthread_mutex_lock(&WM8960_MUTEX);
    /* digital mute flow */
    {
        unsigned short data16;

        data16  = (1 << 3) | (0 << 2);
        wm8960_write_reg(   6,  data16);

        data16  = (1 << 3);
        wm8960_write_reg(   5,  data16);
    }
    
    itp_codec_playback_mute();
    
    /* Prevent Pop Noise*/
    if (_wm8960_out == SpkOutOnly || _wm8960_out == LineSpkOutBoth)
    {
        wm8960_write_reg(   49, 0x037); /* Disable Class D Speaker Outputs */
        wm8960_write_reg(   51, 0x080); /* or 0x080, DC Speaker Boost(gain +0.0dB)*/
    }
    pthread_mutex_unlock(&WM8960_MUTEX);
    return;
}

/* ************************************************************************** */
/* === common DAC/ADC ops === */
void itp_codec_wake_up(void) {}

void itp_codec_standby(void)
{
    pthread_mutex_lock(&WM8960_MUTEX);
    printf("WM8960# %s\n", __func__);

    /* disable CLASS D output */
    if (_wm8960_out == SpkOutOnly || _wm8960_out == LineSpkOutBoth)
    {
        wm8960_write_reg(   51, 0x080); /* or 0x080, DC Speaker Boost(gain +0.0dB)*/
        wm8960_write_reg(   49, 0x037); /* Disable Class D Speaker Outputs */
        usleep(100000);
    }

    /* really turn-off WM8960 */
    wm8960_write_reg(   5,  0x008); /* DAC Digital Soft Mute = Mute */
    wm8960_write_reg(   28, 0x094); /* Enable POBCTRL and BUFDCOPEN and SOFT_ST */
    wm8960_write_reg(   25, 0x000); /* Disable VMID and VREF, (allow output to discharge with S-Curve) */
    usleep(600000);                 /* Delay 600 ms to discharge HP outputs */
    wm8960_write_reg(   26, 0x000); /* Disable DACL, DACR, LOUT and ROUT1 */
    wm8960_write_reg(   15, 0x000); /* Reset Device (default registers) */

    pthread_mutex_unlock(&WM8960_MUTEX);
}

/* DAC */
void itp_codec_playback_init(unsigned output)
{
    DEBUG_PRINT("WM8960# %s\n", __func__);
    _wm8960_out = output;
    init_wm8960_common();
    switch (output)
    {
    case SpkOutOnly    : {break;}         
    case LineSpkOutBoth: {break;}
    case LineOutOnly   :     
    default            : {break;}
    }
    _wm8960_DA_running = 1;
}

void itp_codec_playback_deinit(void)
{
    DEBUG_PRINT("WM8960# %s\n", __func__);

    _wm8960_DA_running = 0; /* put before deinit_wm8960_common() */
    deinit_wm8960_common();
}

void itp_codec_playback_set_direct_vol(unsigned target_vol)
{
    unsigned short  data16;

    if (target_vol > MAX_OUT1_VOLUME){
        printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
        return;
    }

    if (target_vol == curr_out1_volume) {return; }
    
    pthread_mutex_lock(&WM8960_MUTEX);

    curr_out1_volume = target_vol;

    if (_wm8960_DA_running)
    {
        data16 = (1 << 8) | (1 << 7) | curr_out1_volume;
        if (_wm8960_out == LineOutOnly || _wm8960_out == LineSpkOutBoth)//line out L/R
        {
            wm8960_write_reg(   2,  data16);
            wm8960_write_reg(   3,  data16);
        }

        if (_wm8960_out == SpkOutOnly  || _wm8960_out == LineSpkOutBoth)//SPK out L/R
        {
            wm8960_write_reg(   40, data16);
            wm8960_write_reg(   41, data16);
        }
    }   
    pthread_mutex_unlock(&WM8960_MUTEX);
}

void itp_codec_playback_set_direct_volperc(unsigned target_volperc)
{
    unsigned char volstep;

    if (target_volperc >= 100) { target_volperc = 99; }

    volstep = wm8960_perc_to_reg_DAtable[target_volperc];
    itp_codec_playback_set_direct_vol(volstep);
}

void itp_codec_playback_get_currvol(unsigned *currvol)
{
    *currvol = curr_out1_volume;
}

void itp_codec_playback_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
    *max            = MAX_OUT1_VOLUME;
    *regular_0db    = ZERO_DB_OUT1_VOLUME;
    *min            = MIN_OUT1_VOLUME;
}

void itp_codec_playback_mute(void)
{
    unsigned short data16;
    pthread_mutex_lock(&WM8960_MUTEX);

    {
        data16  = (1 << 3) | (0 << 2);
        wm8960_write_reg(   6,  data16);

        data16  = (1 << 3);
        wm8960_write_reg(   5,  data16);
    }

    i2s_delay_us(25000); /* FIXME: dummy loop */

    {
        data16 = (1 << 8) | (0x00 & 0x7F);

        if (_wm8960_out == LineOutOnly || _wm8960_out == LineSpkOutBoth)
        {
            wm8960_write_reg(   2,  data16);    /* adjust LOUT1 Vol */
            wm8960_write_reg(   3,  data16);    /* adjust ROUT1 Vol */
        }

        if (_wm8960_out == SpkOutOnly || _wm8960_out == LineSpkOutBoth)
        {
            wm8960_write_reg(   40, data16);    /* adjust SPK_L Vol */
            wm8960_write_reg(   41, data16);    /* adjust SPK_R Vol */
        }
    }

    pthread_mutex_unlock(&WM8960_MUTEX);
}

void itp_codec_playback_unmute(void)
{
    unsigned short  data16;

    if (curr_out1_volume == MIN_OUT1_VOLUME)
    {  
        itp_codec_playback_mute();
        return;
    }
    pthread_mutex_lock(&WM8960_MUTEX);
    
    data16 = (1 << 8) | (curr_out1_volume & 0x7F);
    if (_wm8960_out == LineOutOnly || _wm8960_out == LineSpkOutBoth)
    {
        wm8960_write_reg(   2,  data16);    /* adjust LOUT1 Vol */
        wm8960_write_reg(   3,  data16);    /* adjust ROUT1 Vol */
    }

    if (_wm8960_out == SpkOutOnly || _wm8960_out == LineSpkOutBoth)
    {
        wm8960_write_reg(   40, data16);    /* adjust SPK_L Vol */
        wm8960_write_reg(   41, data16);    /* adjust SPK_R Vol */
    }

    {
        data16  = (1 << 3) | (0 << 2);
        wm8960_write_reg(   6,  data16);

        data16  = (0 << 3); /* DACMU */
        wm8960_write_reg(   5,  data16);
    }
    
    pthread_mutex_unlock(&WM8960_MUTEX);
}

/* ADC */
void itp_codec_rec_init(unsigned input_source)
{
    DEBUG_PRINT("WM8960# %s\n", __func__);
    _wm8960_in = input_source;
    init_wm8960_common();
    
    switch (input_source)
    {
    case LineInOnly   :{break;}
    case MicInOnly    :{break;}
    case LineMicInBoth:   
    default           :{break;}
    }
    _wm8960_AD_running = 1;
}

void itp_codec_rec_deinit(void)
{
    DEBUG_PRINT("WM8960# %s\n", __func__);

    _wm8960_AD_running = 0; /* put before deinit_wm8960_common() */
    deinit_wm8960_common();
}

void itp_codec_rec_set_direct_vol(unsigned target_vol)
{
    unsigned short  data16;
    if (target_vol > MAX_INPUT_PGA_VOLUME)
    {
        printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
        return;
    }
    if (target_vol == curr_input_pga_volume) { return; }
    
    pthread_mutex_lock(&WM8960_MUTEX);
    
    curr_input_pga_volume = target_vol;
    if (_wm8960_AD_running)
    {
        data16 = (1 << 8) | curr_input_pga_volume;
        wm8960_write_reg(   0,  data16);
        wm8960_write_reg(   1,  data16);
    }
    
    pthread_mutex_unlock(&WM8960_MUTEX);
}

void itp_codec_rec_set_direct_volperc(unsigned target_micperc)
{   
	unsigned int micstep;
	if(target_micperc >= 100) { target_micperc = 99; }

	micstep = wm8960_perc_to_reg_ADtable[target_micperc];
    itp_codec_rec_set_direct_vol(micstep);
}

void itp_codec_rec_get_currvol(unsigned *currvol)
{
    *currvol = curr_input_pga_volume;
}

void itp_codec_rec_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
    *max            = MAX_INPUT_PGA_VOLUME;
    *regular_0db    = ZERO_DB_INPUT_PGA_VOLUME;
    *min            = MIN_INPUT_PGA_VOLUME;
}

void itp_codec_rec_mute(void)
{
    unsigned short  data16;
    pthread_mutex_lock(&WM8960_MUTEX);
    
    if (_wm8960_AD_running)
    {
        data16 = (1 << 8) | (1 << 7) | MIN_INPUT_PGA_VOLUME;
        wm8960_write_reg(   0,  data16);
        wm8960_write_reg(   1,  data16);
    }

    pthread_mutex_unlock(&WM8960_MUTEX);
}

void itp_codec_rec_unmute(void)
{
    unsigned short  data16;
    
    pthread_mutex_lock(&WM8960_MUTEX);
    
    if (_wm8960_AD_running)
    {
        data16 = (1 << 8) | (0 << 7) | curr_input_pga_volume;
        wm8960_write_reg(   0,  data16);
        wm8960_write_reg(   1,  data16);
    }

    pthread_mutex_unlock(&WM8960_MUTEX);
}

void itp_codec_get_i2s_sample_rate(int *samplerate)
{
    pthread_mutex_lock(&WM8960_MUTEX);
    *samplerate = _wm8960_sampleRate;
    pthread_mutex_unlock(&WM8960_MUTEX);
}

void itp_codec_set_i2s_sample_rate(int samplerate)
{
    pthread_mutex_lock(&WM8960_MUTEX);
    _wm8960_sampleRate = samplerate;
    pthread_mutex_unlock(&WM8960_MUTEX);
}

int itp_codec_get_DA_running(void)
{
    return _wm8960_DA_running;
}

int itp_codec_get_AD_running(void)
{
    return _wm8960_AD_running;
}