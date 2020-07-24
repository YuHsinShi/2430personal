#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ite/ith.h"
#include "ite/itp_codec.h"
#include "iic/mmp_iic.h"

#define ALC5628_I2CADR               (0x30 >> 1)

#if CFG_DAC_port_I2C0
#define IIC_PORT IIC_PORT_0
#else
#define IIC_PORT IIC_PORT_1
#endif  

//#define DEBUG_PRINT printf
#define DEBUG_PRINT(...)

/* ************************************************************************** */
/* wrapper */
static inline void i2s_delay_us(unsigned us) { ithDelay(us); }

/* ************************************************************************** */
#define MAX_OUT1_VOLUME            0x00 /*    +0dB */
#define DEFAULT_DB_OUT1_VOLUME     0x10 /* -22.5dB */
#define MIN_OUT1_VOLUME            0x1F /* -46.5dB */
static unsigned curr_out1_volume = DEFAULT_DB_OUT1_VOLUME; /* 0 dB */

#define MAX_INPUT_PGA_VOLUME       0x7F /* +30   dB */
#define ZERO_DB_INPUT_PGA_VOLUME   0x2F /* +0    dB */
#define MIN_INPUT_PGA_VOLUME       0x00 /* -17.625 dB */

static unsigned curr_input_pga_volume = ZERO_DB_INPUT_PGA_VOLUME; /* 0 dB, refer to R0, R1 */

static int _alc5628_DA_running = 0;
static int _alc5628_AD_running = 0;
static pthread_mutex_t ALC5628_MUTEX = PTHREAD_MUTEX_INITIALIZER;

static int _alc5628_linein = 0;
static int _alc5628_i2sin  = 1;
static int _alc5628_spkout = 0;
static int _alc5628_hpout  = 0;

static int _alc5628_cold_start = 1; /* alc5628q should enable power only once */

static int i2s_sample_rate = 48000;

typedef enum _OutPath {
	HPOnly = 0,
	SPKOnly,
	HPSPKBoth,
}OutPath;

/* ************************************************************************** */
/* gamma correction */
//#include "alc5628/digitalGainTable.inc"
#include "alc5628/analogGainTable.inc"

/* ************************************************************************** */
static void I2C_common_write_word(unsigned char RegAddr, unsigned short d)
{
	int success = 0;
	int flag;
	int retry = 0;
    unsigned short tmp = 0;

	if (CFG_CHIP_FAMILY == 9910)
		tmp = d;
	else
    	tmp = (d&0x00FF) << 8 | (d&0xFF00)  >> 8;
    
    //ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (0 << 28), (0x3 << 28)); /* IIC: internal HDMI IIC */
    for(retry=0; retry<50; retry++)
    {
        if(0 == (flag = mmpIicSendData(IIC_PORT, IIC_MASTER_MODE, ALC5628_I2CADR, RegAddr, &tmp, 2)))
        {
        	success = 1;
#ifdef ALC5628_DEBUG_I2C_COMMON_WRITE
        	printf("ALC5628_I2CADR# IIC WriteOK!\n");
#endif
            break;
        }
    }
    if(success == 0) {
    	printf("ALC5628# IIC Write Fail!\n");
    	//while(1) { usleep(500000); }
    }
    //ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (0 << 28), (0x3 << 28)); /* IIC: external APB IIC */
}

static unsigned short I2C_common_read_word(unsigned char RegAddr)
{
    
    int success = 0;
    int flag;
    int retry = 0;
    unsigned short d, tmp = 0;
    
    //ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (0 << 28), (0x3 << 28)); /* IIC: internal HDMI IIC */
    for(retry=0; retry<50; retry++)
    {
        if(0 == (flag = mmpIicReceiveData(IIC_PORT, IIC_MASTER_MODE, ALC5628_I2CADR, &RegAddr, 1, &d, 2)))
        {
            success = 1;
#ifdef ALC5628_DEBUG_I2C_COMMON_WRITE
            printf("ALC5628_I2CADR# IIC Read OK!\n");
#endif
            break;
        }
    }
    if(success == 0) {
        printf("ALC5628# IIC Write Fail!\n");
        while(1) { usleep(500000); }
    }
    //ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (0 << 28), (0x3 << 28)); /* IIC: external APB IIC */

	if (CFG_CHIP_FAMILY == 9910)
		tmp = d;
	else
    	tmp = (d&0x00FF) << 8 | (d&0xFF00)  >> 8;
    
    return tmp ;
}

static void alc5628_write_reg(unsigned char reg_addr, unsigned short value)
{

#ifdef ALC5628_DEBUG_PROGRAM_REGVALUE
	printf("ALC5628# write reg[0x%02x] = 0x%04x\n", reg_addr, value);
#endif
	I2C_common_write_word(reg_addr, value);
}

static unsigned short alc5628_read_reg(unsigned char reg_addr)
{    
    return I2C_common_read_word(reg_addr);
}

static inline void alc5628_writeRegMask(uint16_t addr, uint16_t data, uint16_t mask)
{
    alc5628_write_reg(addr, (alc5628_read_reg(addr) & ~mask) | (data & mask));
}

static void Dump_Register(void)
{

	unsigned short tempRegisterValue;
	unsigned char addr; 

	for (addr=0; addr <= 0xFF; addr += 1)
	{
		tempRegisterValue = alc5628_read_reg(addr);
		printf("reg[0x%02x] = 0x%04x\n", addr, tempRegisterValue);
		if (addr == 0xFF)
		{
		    tempRegisterValue = alc5628_read_reg(addr);
		    printf("reg[0x%02x] = 0x%04x\n", addr, tempRegisterValue);
		    break;
		}    
	}

}

static void init_alc5628_common(void)
{

	DEBUG_PRINT("ALC5628# %s\n", __func__);

	if(_alc5628_DA_running)
	{
		DEBUG_PRINT("ALC5628# DAC is running, skip re-init process !\n");
		return;
	}
    pthread_mutex_lock(&ALC5628_MUTEX);
    
	_alc5628_cold_start = 0;

	/* start programming ALC5628 */
	{
        unsigned short data16;
        
        //alc5628_write_reg(0x00, 0x0003); /* reset ALC5628 */
		
        /*SOFT Delay volume Control time*/
        data16 = 0x00 |(0xA << 0);   //[ 0: 3] soft volm delay time (2^n A:1024 SVSYNC)
		alc5628_write_reg(0x16, 0x000A);
        
        /*Output Mixer Ctrl*/
        data16 = 0x00 |(0x0 <<  1 )  //[    1] DAC output HP AMP ctrl
                      |(0x1 <<  2 )  //[    2] SPK diff Negative singnal output 
                      |(0x3 <<  8 )  //[ 8: 9] HPL vol source select(L/R)
                      |(0x2 << 10 )  //[10:11] SPK vol socrce selec 00:VMIN(no) 01:HP Mixer 10:SPK(diff out) 11:reserve
                      |(0x2 << 14 ); //[14:15] Any mixer to SPKout 
        alc5628_write_reg(0x1C, data16);  

        /*stero DAC Cloc ctrl*/
        data16 = 0x00 |(0x0 <<  1 )  //[    2] stereo DAC filter clk 0:256Fs 1:384Fs
                      |(0x0 << 12 )  //[   12] master mode clock relatice to BCLK & LRCK 0:32bit(64FS) 1:16bit(32FS)
                      |(0x0 <<  8 ); //[13:15] i2s pre-divider   000:1 ~ 101:32 (2^n)
        alc5628_write_reg(0x38, data16);  

         /*stero DAC Cloc ctrl*/
        data16 = 0x00 |(0x5 <<  9 )  //[9:11] class-D amp (output gain) 0x0:2.25(Vdd) ~ 0x5:1(Vdd) :[0.25decay]
                      |(0x1 <<  8 ); //[   8] Stero DAC High pass 
        alc5628_write_reg(0x40, data16);          

        /*MISC2 ctrl*/
        data16 = 0x00 |(0x0  <<  2 )  //[2:3] mute DAC (L/R) 0x1 :mute
                      |(0x1  <<  5 )  //  [5] mute-unmute Depop 0x1:enable
                      |(0x3  <<  6 )  //[6:7] mute-unmute Depop HP (L/R) 0x3:enable
                      |(0x1  <<  8 )  //  [8] De-pop mode 1 HP 
                      |(0x1  <<  9 )  //  [9] De-pop mode 2 HP
                      |(0x0  << 14 )  // [14] thermal shut down enable 0x1:enable 
                      |(0x1  << 15 ); // [15] enable Fast Vref 0x1:disable
        alc5628_write_reg(0x5E, data16);             
        
        /*MISC1 ctrl*/        
        data16 = 0x00 |(0x0  <<  2 )  //     [2] DAC Digtal soft vol
                      |(0x0  <<  3 )  //     [3] DAC Digtal soft vol zero cross detect
                      |(0x0  <<  8 )  // [ 8:11] HP  zero cross(L/R) sotf volume(L/R)  
                      |(0x0  << 12 ); // [12:15] spk zero cross(L/R) soft volume(L/K)
        alc5628_write_reg(0x5C, data16);                 
        
        /*LINE Input Volume*/
        data16 = 0x00 |(0x08 <<  0 ) //[0: 4] LINE IN(R) digtal volume 0x00:12dB 0x08:0dB 0x3F:34.5dB (1.5attenuation) 
                      |(0x0  <<  5 ) //[   5] Line-In diff inpute
                      |(0x1  <<  6 ) //[   6] Line-In(R) mute spk mixer
                      |(0x1  <<  7 ) //[   7] Line-In(R) mute HP  mixer
                      |(0x08 <<  8 ) //[8:12] LINE IN(L) digtal volume 0x00:12dB 0x10:0dB 0x3F:34.5dB (1.5attenuation) 
                      |(0x1  << 14 ) //[  14] Line-In(L) mute spk mixer
                      |(0x1  << 15 );//[  15] Line-In(L) mute HP  mixer
        alc5628_write_reg(0x0A, data16);    
        
         /*stero DAC Digital ctrl vol*/
        data16 = 0x00 |(0x10  <<  0 ) //[ 0: 5] DAC(R) digtal volume 0x00:12dB 0x10:0dB 0x3F:35.25dB (0.75attenuation) 
                      |(0x1   <<  6 ) //[    6] SPK(R) mute 0x1:mute
                      |(0x1   <<  7 ) //[    7] HP (R)      0x1:mute
                      |(0x10  <<  8 ) //[ 8:13] DAC(L) digtal volume 0x00:12dB 0x10:0dB 0x3F:35.25dB (0.75attenuation) 
                      |(0x1   << 14 ) //[   14] SPK(L) mute 0x1:mute
                      |(0x1   << 15 );//[   15] HP (L) mute 0x1:mute
        alc5628_write_reg(0x0C, data16);         
        
        /*SPEAKER out volume analog*/        
        data16 = 0x00 |(0x1F  <<  0 )  // [ 0: 4]SPK out analog vol (R)  (0x0 :0dB~ 0x1F:46.5 dB 1.5dB decady)
                      |(0x1   <<  7 )  // [    7]mute SPK (R) (reg1C[15:14]=01 mute by 15)  
                      |(0x1F  <<  8 )  // [ 8:12]SPK out analog vol (L)  (0x0 :0dB~ 0x1F:46.5 dB 1.5dB decady_
                      |(0x1   << 15 ); // [   15]mute speaker out posituve/Negative 0x1 mute
        alc5628_write_reg(0x02, data16);    
        
        /*HP out volume analog*/        
        data16 = 0x00 |(0x1F  <<  0 )  // [ 0: 4]HP out analog vol (R)  (0x0 :0dB~ 0x1F:46.5 dB 1.5dB decady)
                      |(0x1   <<  7 )  // [    7]mute HP (R)  0x1 mute
                      |(0x1F  <<  8 )  // [ 8:12]HP out analog vol (L)  (0x0 :0dB~ 0x1F:46.5 dB 1.5dB decady)
                      |(0x1   << 15 ); // [   15]mute HP (L)  0x1 mute
        alc5628_write_reg(0x04, data16);       
        
        /* power on amplifer*/   
        data16 = 0x00 |(0x3 <<  6 )  //[ 6: 7] power line in ctrl volume(L/R)
                      |(0x3 <<  9 )  //[ 9:10] power hp out volume & HP AMP (L/R)
                      |(0x1 << 12 )  //[   12] power SPK OUT volume
                      |(0x1 << 15 ); //[   15] power main bias
        alc5628_write_reg(0x3E, data16);  

        /* Power on I2S Digital Interface Enable Ctrl */
        data16 = 0x00 |(0x3 <<  4 )  //[ 4: 5]HP amplifer               
                      |(0x1 <<  8 )  //[    8]pow_softgen                  
                      |(0x1 << 14 )  //[   14]zero cross power able        
                      |(0x1 << 15 ); //[   15]i2s interface able           
        alc5628_write_reg(0x3A, data16);
        
        /* Power on Mixer*/
        data16 = 0x00 |(0x1 <<  3 )  //[    3]SPK mix                     
                      |(0x3 <<  4 )  //[ 4: 5]HP mix (L/R)             
                      |(0x3 <<  6 )  //[ 6: 7]power DAC to mix (L/R)    
                      |(0x3 <<  8 )  //[ 8: 9]power DAC (L/R)           
                      |(0x1 <<  10)  //[   10]DAC (Vref+ Vref-)   
                      |(0x1 <<  11) 
                      |(0x1 <<  12)  //[   12]PLL enable                 
                      |(0x1 <<  13)  //[   13]power vref(ctr Vref pin)   
                      |(0x1 <<  14); //[   14]power Class-D SPK          
        alc5628_write_reg(0x3C, data16);
    }
    pthread_mutex_unlock(&ALC5628_MUTEX);
}

static void deinit_alc5628_common(void)
{
	DEBUG_PRINT("ALC5628# %s\n", __func__);

	if(_alc5628_DA_running)
	{
		DEBUG_PRINT("ALC5628# DAC is running, skip deinit !\n");
		return;
	}
    
	pthread_mutex_lock(&ALC5628_MUTEX);
    
    alc5628_write_reg(0x02, 0x9F9F); // SPK OUT VOL min dB  & mute
	alc5628_write_reg(0x04, 0x9F9F); // HP  OUT VOL min dB  & mute  
    alc5628_write_reg(0x0A, 0xC8C8); // Line in VOL min dB  & mute 
    alc5628_write_reg(0x0C, 0xFFFF); // mute digtal DA     
    
	pthread_mutex_unlock(&ALC5628_MUTEX);
	return;
}

void itp_codec_wake_up(void)
{
    unsigned short data16;
	data16 = ithReadRegH(0x003E);

	data16 |= (1 << 1); /* enable AMCLK (system clock for wolfson chip) */
	data16 |= (1 << 3); /* enable ZCLK  (IIS DAC clock)                 */
	data16 |= (1 << 5); /* enable M8CLK (memory clock for DAC)          */
	data16 |= (1 << 7); /* enable M9CLK (memory clock for ADC)          */
	ithWriteRegH(0x003E, data16);
    
	alc5628_write_reg(0x3E, 0xF6F0);/* power on amplifer*/   
	alc5628_write_reg(0x3A, 0xC130);/* Power on I2S Digital Interface Enable Ctrl */
	alc5628_write_reg(0x3C, 0x7FF8);/* Power on Mixer*/ 
	//pthread_mutex_lock(&ALC5628_MUTEX);
	//printf("ALC5628# %s\n", __func__);
	//init_alc5628_common();
	//_alc5628_DA_running = 1;
	//init_alc5628_common();
	//_alc5628_AD_running = 1;
	//pthread_mutex_unlock(&ALC5628_MUTEX);
}
void itp_codec_standby(void)
{
    unsigned short data16;
	data16 = ithReadRegH(0x003E);

	data16 |= (1 << 1); /* enable AMCLK (system clock for wolfson chip) */
	data16 |= (1 << 3); /* enable ZCLK  (IIS DAC clock)                 */
	data16 |= (1 << 5); /* enable M8CLK (memory clock for DAC)          */
	data16 |= (1 << 7); /* enable M9CLK (memory clock for ADC)          */
	ithWriteRegH(0x003E, data16);    
    
	printf("ALC5628# %s\n", __func__);
	pthread_mutex_lock(&ALC5628_MUTEX);
	itp_codec_playback_deinit();
	itp_codec_rec_deinit();
	alc5628_write_reg(0x00, 0x00);
	pthread_mutex_unlock(&ALC5628_MUTEX);
}

static void _output_path_slect(OutPath enRL){
    
    _alc5628_spkout=0;
    _alc5628_hpout=0;
    switch (enRL)
    {
    case HPOnly:     
        {/* HP  only */
            _alc5628_hpout = 1;
            break;
        }
    case SPKOnly:
        {/*SPK only */
            _alc5628_spkout = 1;
            break;
        }
    case HPSPKBoth:     
        {/* HeadPhone + SPK path */
            _alc5628_spkout=1;
            _alc5628_hpout=1;
            break;
        }

    default:
        {/* HeadPhone SPK path disable */
            _alc5628_spkout=0;
            _alc5628_hpout=0;
            break;
        }
    }
    if(_alc5628_spkout) {
            alc5628_writeRegMask(0x02, 0<<15 | 0<<7, 0x1<<15 | 0x1<<7);//SPK out on
            alc5628_writeRegMask(0x02, curr_out1_volume<<8 | curr_out1_volume<<0, 0x1F<<8 | 0x1F<<0);//DAC analog(R|L) SPK vol    
            alc5628_writeRegMask(0x0C, 0<<14 | 0<<6, 0x1<<14 | 0x1<<6);//DAC SPK mix           
    }
    if(_alc5628_hpout) {
            alc5628_writeRegMask(0x04, 0<<15 | 0<<7, 0x1<<15 | 0x1<<7);//HP out on
            alc5628_writeRegMask(0x04, curr_out1_volume<<8 | curr_out1_volume<<0, 0x1F<<8 | 0x1F<<0);//DAC analog(R|L) HP vol    
            alc5628_writeRegMask(0x0C, 0<<15 | 0<<7, 0x1<<15 | 0x1<<7);//DAC HP mixer           
    }
    usleep(50000);/* Delay (50 ms) to allow HP amps to settle */
    printf("_alc5628_spkout=%d _alc5628_hpout=%d\n",_alc5628_spkout,_alc5628_hpout);
}

/* DAC */
void itp_codec_playback_init(unsigned output)
{
	DEBUG_PRINT("ALC5628# %s\n", __func__);
	
	init_alc5628_common();
    _output_path_slect(output);
	_alc5628_DA_running = 1;
}

void itp_codec_playback_deinit(void)
{
	DEBUG_PRINT("ALC5628# %s\n", __func__);

	_alc5628_DA_running = 0; /* put before  () */
	//deinit_alc5628_common();
    alc5628_write_reg(0x00, 0x0003); //reset all reg
	//alc5628_write_reg(0x3C, 0x4000);/* Power down Mixer*/ 
	//alc5628_write_reg(0x3A, 0x0000);/* Power down I2S Digital Interface Enable Ctrl */
	//alc5628_write_reg(0x3E, 0x0000);/* power down amplifer*/   

}

void itp_codec_playback_amp_volume_down(void)
{
    printf("no use\n");
}

void itp_codec_playback_amp_volume_up(void)
{
    printf("no use\n");
}

void itp_codec_playback_set_direct_vol(unsigned target_vol)
{
	int direction;
	unsigned short data16;

	if(target_vol < MAX_OUT1_VOLUME){
		 printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
		 return;
	}
	if(target_vol == curr_out1_volume) {
		return;
	}
    
	pthread_mutex_lock(&ALC5628_MUTEX);

    curr_out1_volume = target_vol;


	if(_alc5628_DA_running){
        if(_alc5628_spkout) alc5628_writeRegMask(0x02, curr_out1_volume<<8 | curr_out1_volume<<0, 0x1F<<8 | 0x1F<<0);//DAC analog(R|L) SPK vol      
           
        if(_alc5628_hpout ) alc5628_writeRegMask(0x04, curr_out1_volume<<8 | curr_out1_volume<<0, 0x1F<<8 | 0x1F<<0);//DAC analog(R|L) HP vol       

        if(_alc5628_linein) alc5628_writeRegMask(0x0A, curr_out1_volume<<8 | curr_out1_volume<<0, 0x1F<<8 | 0x1F<<0);//Line in volume           
        //if(_alc5628_i2sin)  alc5628_writeRegMask(0x0C, 0x0<<8 | 0x0<<0, 0x3F<<8 | 0x3F<<0);//DAC digital
	}      
	pthread_mutex_unlock(&ALC5628_MUTEX);
}

void itp_codec_playback_set_direct_volperc(unsigned target_volperc)
{
	unsigned char volstep;

	if(target_volperc >= 100) { target_volperc = 99; }

	volstep = alc5628_perc_to_reg_table[target_volperc];
	itp_codec_playback_set_direct_vol(volstep);

}

void itp_codec_playback_get_currvol(unsigned *currvol)
{
	*currvol = curr_out1_volume;
}

void itp_codec_playback_get_currvolperc(unsigned *currvolperc)
{
	unsigned i;

	for(i=0; i<99; i++)
	{
		if((alc5628_perc_to_reg_table[i] >= curr_out1_volume)
		&& (curr_out1_volume > alc5628_perc_to_reg_table[i+1])) {
			*currvolperc = i;
			return;
		}
	}
	*currvolperc = 99;
}

void itp_codec_playback_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
	*max         = MAX_OUT1_VOLUME;
	*regular_0db = DEFAULT_DB_OUT1_VOLUME;
	*min         = MIN_OUT1_VOLUME;
}

void itp_codec_playback_mute(void)
{	
	pthread_mutex_lock(&ALC5628_MUTEX);

    DEBUG_PRINT("%s (%d)\n", __FUNCTION__, __LINE__);
	    		
    if(_alc5628_DA_running){
        //if(_alc5628_spkout) alc5628_writeRegMask(0x02, 1<<15 | 1<<7, 0x1<<15 | 0x1<<7);//SPK out mute      
        //if(_alc5628_hpout ) alc5628_writeRegMask(0x04, 1<<15 | 1<<7, 0x1<<15 | 0x1<<7);//HP out mute   
        if(_alc5628_linein) alc5628_writeRegMask(0x0A, 3<<14 | 3<<6, 0x3<<14 | 0x3<<6);//0xC8C8
        if(_alc5628_i2sin ) alc5628_writeRegMask(0x0C, 3<<14 | 3<<6, 0x3<<14 | 0x3<<6);
        if(_alc5628_spkout) alc5628_writeRegMask(0x3E, 0<<12, 0x1<<12);
    }
	pthread_mutex_unlock(&ALC5628_MUTEX);
}

void itp_codec_playback_unmute(void)
{
    DEBUG_PRINT("%s (%d)\n", __FUNCTION__, __LINE__);
    
    if(curr_out1_volume == MIN_OUT1_VOLUME){
        itp_codec_playback_mute();
        return;
    }
    pthread_mutex_lock(&ALC5628_MUTEX);    
    
    i2s_delay_us(1000); /* FIXME: dummy loop */
    
	if(_alc5628_DA_running){
        //if(_alc5628_spkout) alc5628_writeRegMask(0x02, 0<<15 | 0<<7, 0x1<<15 | 0x1<<7);//SPK out on      
        //if(_alc5628_hpout ) alc5628_writeRegMask(0x04, 0<<15 | 0<<7, 0x1<<15 | 0x1<<7);//HP out on   
        if(_alc5628_linein) alc5628_writeRegMask(0x0A, 0<<14 | 0<<6, 0x3<<14 | 0x3<<6);
        if(_alc5628_i2sin ) alc5628_writeRegMask(0x0C, 0<<14 | 0<<6, 0x3<<14 | 0x3<<6);
        if(_alc5628_spkout) alc5628_writeRegMask(0x3E, 1<<12, 0x1<<12);
    }  
    
    i2s_delay_us(1000); /* FIXME: dummy loop */
	pthread_mutex_unlock(&ALC5628_MUTEX);	
}

/* line-in bypass to line-out directly */
void itp_codec_playback_linein_bypass(unsigned bypass)
{

}

/* ADC */
void itp_codec_rec_init(unsigned input_source)
{
	DEBUG_PRINT("ALC5628# %s\n", __func__);
	
    printf("alc5628 AD invalid no use\n");
	init_alc5628_common();
	_alc5628_AD_running = 1;
}

void itp_codec_rec_deinit(void)
{
	DEBUG_PRINT("ALC5628# %s\n", __func__);
    printf("alc5628 AD invalid no use\n");
}

void itp_codec_rec_set_direct_vol(unsigned target_vol)
{
	return;
}

void itp_codec_rec_get_currvol(unsigned *currvol)
{
	*currvol = curr_input_pga_volume;
}

void itp_codec_rec_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
	*max         = MAX_INPUT_PGA_VOLUME;
	*regular_0db = ZERO_DB_INPUT_PGA_VOLUME;
	*min         = MIN_INPUT_PGA_VOLUME;
}

void itp_codec_rec_mute(void)
{
	return;
}

void itp_codec_rec_unmute(void)
{
	return;
}

void itp_codec_power_on(void)
{
    return;
}

void itp_codec_power_off(void)
{
    return;
}

void itp_codec_get_i2s_sample_rate(int* samplerate)
{
	pthread_mutex_lock(&ALC5628_MUTEX);
	*samplerate = i2s_sample_rate;
	pthread_mutex_unlock(&ALC5628_MUTEX);
}

void itp_codec_set_i2s_sample_rate(int samplerate)
{
	pthread_mutex_lock(&ALC5628_MUTEX);
	i2s_sample_rate = samplerate;	
	pthread_mutex_unlock(&ALC5628_MUTEX);
}

void itp_codec_set_linein_enable(int yesno)
{
    _alc5628_linein = yesno;
    if(_alc5628_linein)        
    {
        unsigned short adj = curr_out1_volume;
        adj>>=1;
        adj = (adj << 8)|adj;            
	    alc5628_write_reg(0x0A, 0x0808);/* LINE_IN 0db*/
    }
    else
    {
        alc5628_write_reg(0x0A, 0xC8C8);/* LINE_IN Disable */
    }    
}

void itp_codec_set_i2sin_enable(int yesno)
{
    _alc5628_i2sin = yesno;
    if(_alc5628_i2sin)
    {
        unsigned short adj = curr_out1_volume;
        adj = (adj << 8)|adj;
        alc5628_write_reg(0x0C, adj); //DAC Digital Volume mute
        alc5628_writeRegMask(0x3A,(1<<15),(1<<15));
    }
    else
    {
        alc5628_write_reg(0x0C, 0xFFFF);
        alc5628_writeRegMask(0x3A,(0<<15),(0<<15));
    }            
    
}

int itp_codec_get_DA_running(void){
    return _alc5628_DA_running;
}

int itp_codec_get_AD_running(void)
{
    return _alc5628_AD_running;
}