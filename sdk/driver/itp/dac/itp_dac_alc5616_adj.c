#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ite/ith.h"
#include "ite/itp_codec.h"
#include "iic/mmp_iic.h"

#define ALC5616_I2CADR               (0x36 >> 1)

#if CFG_DAC_port_I2C0
#define IIC_PORT IIC_PORT_0
#else
#define IIC_PORT IIC_PORT_1
#endif

//#define DEBUG_PRINT printf
#define DEBUG_PRINT(...)

/* ************************************************************************** */
//alc5616
#include "alc5616/DA_analog_table.inc"
#include "alc5616/AD_analog_table.inc"

#define MAX_OUT1_VOLUME          0x00                                    /* +12 dB */
#define DEFAULT_DB_OUT1_VOLUME   0x08                                    /* 0 dB */
#define MIN_OUT1_VOLUME          0x27                                    /* -46.5 dB */
static unsigned curr_out1_volume = DEFAULT_DB_OUT1_VOLUME;               /* 0 dB */

#define MAX_INPUT_PGA_VOLUME     0x08                                    /* +52   dB */
#define ZERO_DB_INPUT_PGA_VOLUME 0x01                                    /* +20    dB */
#define MIN_INPUT_PGA_VOLUME     0x00                                    /* 0 dB */

static unsigned curr_input_pga_volume = ZERO_DB_INPUT_PGA_VOLUME; /* 0 dB, refer to R0, R1 */

static pthread_mutex_t ALC5616_MUTEX         = PTHREAD_MUTEX_INITIALIZER;
static int             _alc5616_DA_running   = 0;
static int             _alc5616_AD_running   = 0;
static int             _alc5616_in        = -1;
static int             _alc5616_out       = -1;
static int             _alc5616_slave     =  1;

static int             _alc5616_cold_start   = 1; /* alc5616q should enable power only once */
static int             _alc5616_mute         = 0;

static int             i2s_sample_rate       = 48000;

typedef enum _OutPath {
	HPOnly = 0,
	SPKOnly,
	HPSPKBoth,
}OutPath;

typedef enum _InPath {
	LineInOnly = 0,
	MicInOnly,
	MicLineBoth,
}InPath;

/* ************************************************************************** */

/* ************************************************************************** */
static void I2C_common_write_word(unsigned char RegAddr, unsigned short d)
{
    int            success = 0;
    int            flag;
    int            retry   = 0;
    unsigned short tmp     = 0;

    if (CFG_CHIP_FAMILY == 9910)
        tmp = d;
    else
        tmp = (d & 0x00FF) << 8 | (d & 0xFF00) >> 8;

    for (retry = 0; retry < 50; retry++)
    {
        if (0 == (flag = mmpIicSendData(IIC_PORT, IIC_MASTER_MODE, ALC5616_I2CADR, RegAddr, (uint8_t *)&tmp, 2)))
        {
            success = 1;
            break;
        }
    }
    if (success == 0)
    {
        printf("ALC5616# IIC Write Fail!\n");
        while (1)
        {
            usleep(500000);
        }
    }
}

static unsigned short I2C_common_read_word(unsigned char RegAddr)
{
    int            success = 0;
    int            flag;
    int            retry = 0;
    unsigned short d, tmp = 0;

    for (retry = 0; retry < 50; retry++)
    {
        if (0 == (flag = mmpIicReceiveData(IIC_PORT, IIC_MASTER_MODE, ALC5616_I2CADR, &RegAddr, 1, (uint8_t *)&d, 2)))
        {
            success = 1;
            break;
        }
    }
    if (success == 0)
    {
        printf("ALC5616# IIC Write Fail!\n");
        while (1)
        {
            usleep(500000);
        }
    }

    if (CFG_CHIP_FAMILY == 9910)
        tmp = d;
    else
        tmp = (d & 0x00FF) << 8 | (d & 0xFF00) >> 8;

    return tmp;
}

static void alc5616_write_reg(unsigned char reg_addr, unsigned short value)
{
#ifdef ALC5616_DEBUG_PROGRAM_REGVALUE
    printf("ALC5616# write reg[0x%02x] = 0x%04x\n", reg_addr, value);
#endif
    I2C_common_write_word(reg_addr, value);
}

static unsigned short alc5616_read_reg(unsigned char reg_addr)
{
    unsigned short value = I2C_common_read_word(reg_addr);
#ifdef ALC5616_DEBUG_PROGRAM_REGVALUE
    printf("ALC5616# read reg[0x%02x] = 0x%04x\n", reg_addr, value);
#endif
    return value;
}

static inline void alc5616_writeRegMask(uint16_t addr, uint16_t data, uint16_t mask)
{
    alc5616_write_reg(addr, (alc5616_read_reg(addr) & ~mask) | (data & mask));
}

static void Dump_Register(void)
{
    unsigned short tempRegisterValue;
    unsigned char  addr;

    for (addr = 0; addr <= 0xFF; addr += 1)
    {
        tempRegisterValue = alc5616_read_reg(addr);
        printf("reg[0x%02x] = 0x%04x\n", addr, tempRegisterValue);
        if (addr == 0xFF)
        {
            tempRegisterValue = alc5616_read_reg(addr);
            printf("reg[0x%02x] = 0x%04x\n", addr, tempRegisterValue);
            break;
        }
    }
}

static void _output_path_slect(OutPath enRL){ 

    switch (enRL)
    {
    case HPOnly:     
        {/* HPOnly */
            /*HP open*/
            alc5616_writeRegMask(0x45, (0x0 << 13) ,(0x3 << 13));
            alc5616_writeRegMask(0x02, (0x0 << 14) | (0x0 << 6), (0x3 << 14) | (0x3 <<  6));      
            break;
        }
    case SPKOnly:     
        {
            /*SPK open*/
            alc5616_writeRegMask(0x53, (0x0 << 12) ,(0x3 << 12));
            alc5616_writeRegMask(0x03, (0x0 << 14) | (0x0 << 6), (0x3 << 14) | (0x3 <<  6));
            break;
        }
    case HPSPKBoth:     
        {/* HPSPKBoth */
        }
    default:
        {/*HPSPKBoth*/
            /*HP open*/
            alc5616_writeRegMask(0x45, (0x0 << 13) ,(0x3 << 13));
            alc5616_writeRegMask(0x02, (0x0 << 14) | (0x0 << 6), (0x3 << 14) | (0x3 <<  6)); 
            /*SPK open*/
            alc5616_writeRegMask(0x53, (0x0 << 12) ,(0x3 << 12));
            alc5616_writeRegMask(0x03, (0x0 << 14) | (0x0 << 6), (0x3 << 14) | (0x3 <<  6));
            break;
        }
    }
    alc5616_writeRegMask(0x02, curr_out1_volume << 8, 0x3F << 8);
    alc5616_writeRegMask(0x02, curr_out1_volume << 0, 0x3F << 0);
    
    alc5616_writeRegMask(0x03, curr_out1_volume << 8, 0x3F << 8);
    alc5616_writeRegMask(0x03, curr_out1_volume << 0, 0x3F << 0);
}

static void _input_path_slect(InPath enRL){

    switch (enRL)
    {
    case LineInOnly:     
        {/* Line only*/
            alc5616_writeRegMask(0x3C, (0x0 << 5), (0x1 << 5)); //INL to RECMIXL
            alc5616_writeRegMask(0x3E, (0x0 << 5), (0x1 << 5)); //INR to RECMIXR
            break;
        }
    case MicInOnly:     
        {/* mic only */
            alc5616_writeRegMask(0x3C, (0x0 << 1), (0x1 << 1));//BST1 to RECMIXL
            alc5616_writeRegMask(0x3E, (0x0 << 2), (0x1 << 2));//BST2 to RECMIXR
            break;
        }
    case MicLineBoth:     
        {/* MicLineBoth */
        }
    default:
        {//MicLineBoth
            alc5616_writeRegMask(0x3C, (0x0 << 1), (0x1 << 1));//BST1 to RECMIXL
            alc5616_writeRegMask(0x3E, (0x0 << 2), (0x1 << 2));//BST2 to RECMIXR         
            break;
        }
    }
    
    alc5616_writeRegMask(0x0D, curr_input_pga_volume << 12, 0xF << 12);
    alc5616_writeRegMask(0x0D, curr_input_pga_volume <<  8, 0xF <<  8);
    /*
    alc5616_writeRegMask(0x0F, curr_input_pga_volume << 12, 0xF << 12);
    alc5616_writeRegMask(0x0F, curr_input_pga_volume <<  8, 0xF <<  8);
    */
    
}

static void init_alc5616_common(void)
{
    pthread_mutex_lock(&ALC5616_MUTEX);

    if (_alc5616_DA_running)
    {
        DEBUG_PRINT("ALC5616# DAC is running, skip re-init process !\n");
        pthread_mutex_unlock(&ALC5616_MUTEX);
        return;
    }
    if (_alc5616_AD_running)
    {
        DEBUG_PRINT("ALC5616# ADC is running, skip re-init process !\n");
        pthread_mutex_unlock(&ALC5616_MUTEX);
        return;
    }

    if(1){
        unsigned short data16;
        
        /*Power Management  analog DA AD I2S power*/
        data16 = 0x00 |(0x1 << 15 )  //[   15] i2s digital interface power 0x1:on
                      |(0x3 << 11 )  //[12:11] analog DAC power(L/R)       0x3:on
                      |(0x3 <<  1 ); //[ 2: 1] analog ADC power(L/R)       0x3:on
        alc5616_write_reg(0x61, data16);  
        
        /*Power Management stero DA AD filter power*/
        data16 = 0x00 |(0x1 << 15 )  //[   15] stereo ADC digtal filter power 0x1:on
                      |(0x1 << 11 ); //[   11] stereo DAC digtal filter power 0x1:on
        alc5616_write_reg(0x62, data16);  
        
        /*Power Management Enable Vref voltage, enable all analog Circuit bias*/
        data16 = 0x00 |(0x1 << 15 )  //[   15] VREF1 power ctrl 0x1:on
                      |(0x0 << 14 )  //[   14] VREF1 fast mode ctrl 0x0:faster
                      |(0x1 << 13 )  //[   13] bias power ctrl
                      |(0x1 << 12 )  //[   12] LOUTMIX power ctrl
                      |(0x1 << 11 )  //[   11] bias bandgap power ctrl
                      |(0x3 <<  6 )  //[ 7: 6] HP Amp power ctrl(LR) 0x3:on
                      |(0x1 <<  5 )  //[    5] HP Amp Driving 0x1:enable
                      |(0x1 <<  4 )  //[    4] VREF2 power ctrl 0x0:down 0x1:on
                      |(0x0 <<  3 )  //[    3] enable VREF2 faster mode ctrl 0x1:faster 0x1 :slow
                      |(0x0 <<  0 ); //[ 1: 0] LDO output ctrl 0x0:1.1V 0x1:1.2V 0x2:1.3V 0x3:1.4V 
        alc5616_write_reg(0x63, data16);

        /*Power Management OUTMIXR/L RECMIXR/L power*/
        data16 = 0x00 |(0x3 << 14 )  //[15:14] OUTmix power(L/R)
                      |(0x3 << 10 ); //[11:10] RECmix power(L/R)
        alc5616_write_reg(0x65, data16);  
        
        /*Power Management IN HP out power*/
        data16 = 0x00 |(0x3 << 12 )  //[13:12] OUT VOL power ctrl(L/R)
                      |(0x3 << 10 )  //[11:10] HP  VOL power ctrl(L/R)
                      |(0x0 <<  8 ); //[ 9: 8] IN  VOL power ctrl(L/R)
        alc5616_write_reg(0x66, data16);  
        
        /*RECMIXL contrl gain*/
        data16 = 0x00 |(0x0 << 10 )   //[12:10] Gain ctrl BST2 to RECMIXL 0x0:0dB ~0x6:-18dB (3dB decay)
                      |(0x0 <<  1 );  //[ 3: 1] Gain ctrl INL  to RECMIXL 0x0:0dB ~0x6:-18dB (3dB decay)
        alc5616_write_reg(0x3B, data16);  
        
        /*RECMIXR contrl gain*/
        data16 = 0x00 |(0x0 << 10 )  //[12:10] Gain ctrl BST2 to RECMIXR 0x0:0dB ~0x6:-18dB (3dB decay)
                      |(0x0 <<  1 ); //[ 3: 1] Gain ctrl INR  to RECMIXR 0x0:0dB ~0x6:-18dB (3dB decay)
        alc5616_write_reg(0x3D, data16);  


        /*AD/DA clk ctrl 1*/
        data16 = 0x00 |(0x0 << 12 )   //[14:12] I2S clk pre-divider 0x0:[1 2 3 4 6 8 12 16]:0x7
                      |(0x0 <<  2 )   //[ 3: 2] Stero DAC over sample rate 0x0:128 Fs 0x1:64Fs 0x2:32Fs 0x3:(128/3)Fs
                      |(0x0 <<  0 );  //[ 1: 0] Stero ADC over sample rate 0x0:128 Fs 0x1:64Fs 0x2:32Fs 0x3:(128/3)Fs 
        alc5616_write_reg(0x73, data16);   //alc5616_write_reg(0x73, 0x0100); 
        
        /*AD/DA cloc ctrl 2 HPF filter*/
        data16 = 0x00 |(0x1 << 11 )  //[   11] DAC HPF filter power on 0x1:enable
                      |(0x1 << 10 ); //[   10] ADC HPF filter power on
        alc5616_write_reg(0x74, data16);   
        
        /*Line output ctrl*/
        data16 = 0x00 |(0x1 << 15 );  //[   15] Enable differential Line output
        alc5616_write_reg(0x05, data16);  

        /*GPIO ctrl 1*/
        data16 = 0x00 |(0x0   << 15 ); //[   15] GPIO1 Pin function select
        alc5616_write_reg(0xC0, data16);  
        
        /*GPIO ctrl 2*/
        data16 = 0x00 |(0x1 <<  2 )  // [2] GPIO1 pin configuration 0x0:input 0x1:output
                      |(0x1 <<  1 )  // [1] GPIO1 Output Pin ctrl 0x0:drive low 0x1:drive high
                      |(0x0 <<  0 ); // [0] GPIO1 Pin polarity
        alc5616_write_reg(0xC1, data16);

        /*DA Digital vol (L/R) 2*/
        data16 = 0x00 |(0xAF <<  8 )  // DAC Digital(L) vol 0x0(-65.625dB)~0xAF(0dB)
                      |(0xAF <<  0 ); // DAC Digital(R) vol 0x0(-65.625dB)~0xAF(0dB)
        alc5616_write_reg(0x19, data16);
        
        /*HP Amp Control 1*/
        data16 = 0x00 |(0x0 << 15 )  // [   15] enable softgen trigger for soft mute depop
                      |(0x0 <<  8 )  // [ 9: 8] Enable HP mute/un-mute Depop(L/R) 
                      |(0x0 <<  7 )  // [    7] capless Depop power down ctrl
                      |(0x0 <<  6 )  // [    6] reset softgen to initialize SOFTP = 1
                      |(0x0 <<  5 )  // [    5] reset softgen to initialize SOFTP = 0
                      |(0x1 <<  4 )  // [    4] Enable HP out
                      |(0x1 <<  3 )  // [    3] chare pump power ctrl
                      |(0x1 <<  2 )  // [    2] Power in soft generator
                      |(0x1 <<  0 ); // [    0] HP Amp all power on ctrl 
        alc5616_write_reg(0x8E, data16);        

        /*HP Amp Control 2*/
        data16 = 0x00 |(0x1  << 13)  // [   13] Select HP Depop mode 
                      |(0x0  <<  6); // [    6] HP depop mode1 ctrl
        alc5616_write_reg(0x8F, data16);
        
        { //Input
        /*IN1/2 Input Control*/
        data16 = 0x00 |(0x2  << 12 )  // [15:12] IN 1 booster 0x0:0db 0x1:20dB ..0x8:52dB
                      |(0x2  <<  8 )  // [11: 8] IN 2 booster 0x0:0db 0x1:20dB ..0x8:52dB
                      |(0x0  <<  7 )  // [    7] IN 1 mode :0x0:single in ,0x1:differential in
                      |(0x1  <<  6 ); // [    6] IN 2 mode :0x0:single in ,0x1:differential in
        alc5616_write_reg(0x0D, data16);           
        
        /*INR/L Input volume Control*/
        data16 = 0x00 |(0x8  <<  8 )  // [12: 8] INL volume 0x0(12dB)~0x1F(-34.5dB)
                      |(0x8  <<  0 ); // [ 4: 0] INR volume 0x0(12dB)~0x1F(-34.5dB)
        alc5616_write_reg(0x0F, data16);    
        
        /*RECMIXR ctrl 1*/
        data16 = 0x00 |(0x0  << 13 )  // [15:13] BST1 to RECMIXL gain 0x0:0dB~0x6:-18dB (3db decay)
                      |(0x1  <<  5 )  // [    5] INL  to RECMIXL 0x1:mute
                      |(0x1  <<  2 )  // [    2] BST2 to RECMIXL 0x1:mute
                      |(0x0  <<  1 ); // [    1] BST1 to RECMIXL 0x1:mute
        alc5616_write_reg(0x3C, data16);
        
        /*RECMIXR ctrl 1*/
        data16 = 0x00 |(0x0  << 13 )  // [15:13] BST1 to RECMIXR gain 0x0:0dB~0x6:-18dB (3db decay)
                      |(0x1  <<  5 )  // [    5] INR  to RECMIXR 0x1:mute
                      |(0x0  <<  2 )  // [    2] BST2 to RECMIXR 0x1:mute
                      |(0x1  <<  1 ); // [    1] BST1 to RECMIXR 0x1:mute
        alc5616_write_reg(0x3E, data16);
        
        /*Power Management Input ctrl*/
        data16 = 0x00 |(0x1  << 15 )  // [   15] MIC BST1 power 0x1:on
                      |(0x0  << 14 )  // [   14] MIC BST2 power 0x1:on
                      |(0x1  << 11 )  // [   11] MICBIAS1 power 0x1:on
                      |(0x0  <<  9 )  // [    9] PLL      power 0x1:on
                      |(0x0  <<  5 )  // [    5] MIC1 SE Mode control :0x0:differential 0x1:single-end
                      |(0x0  <<  4 )  // [    4] MIC2 SE Mode control :0x0:differential 0x1:single-end
                      |(0x0  <<  2 )  // [    2] JD       power 0x1:on
                      |(0x0  <<  1 ); // [    1] JD2      power 0x1:in
        alc5616_write_reg(0x64, data16);
        
        /*ADC Digital volume*/
        data16 = 0x00 |(0x0  << 15 )  // [   15] ADC L Digital Channel 0x1:mute
                      |(0x2F <<  8 )  // [14: 8] ADC L Volume ctrl 0x0(-17.625dB)~0x7F(+30dB) 0x2F(0dB) 0.375db/step
                      |(0x0  <<  7 )  // [    7] ADC R Digital Channel 0x1:mute
                      |(0x2F <<  0 ); // [ 6: 0] ADC L Volume ctrl 0x0(-17.625dB)~0x7F(+30dB) 0x2F(0dB) 0.375db/step
        alc5616_write_reg(0x1C, data16);
        
        /*ADC Digital Mixer vtrl*/
        data16 = 0x00 |(0x0  << 14 )  // [   14] ADC L channel 0x1:mute
                      |(0x70 <<  7 )  // [13: 7] reserved
                      |(0x0  <<  6 ); // [    6] ADC R channel 0x1:mute
        alc5616_write_reg(0x27, data16);
        
        }
    
        { //Output
        
        /*I2S DIgital Interface ctrl*/
        data16 = 0x00 |(_alc5616_slave << 15 )  // [   15] DAC mode 0x0:master 0x1 slave
                      |(0x0 << 10 )  // [11:10] I2S Output compress 0x0:off 0x1:u-law 0x2:alaw 0x3:reserved
                      |(0x0 <<  8 )  // [ 9: 8] I2S Input  compress 0x0:off 0x1:u-law 0x2:alaw 0x3:reserved
                      |(0x0 <<  7 )  // [    7] I2S BCLK polarity ctrl 0x1:Invert
                      |(0x0 <<  2 )  // [ 3: 2] I2s Data Length 0x0:16b' 0x1:20b' 0x2:24b' 0x3:8b'
                      |(0x0 <<  0 ); // [ 1: 0] I2S PCM Format 0x0:I2S format 0x1 Left justified 0x2:(..) 0x3:(..)
        alc5616_write_reg(0x70, data16);
        
        /*DAC Digital Mixer ctrl*/
        data16 = 0x00 |(0x0 << 14 )  // [   14] DAC L to stereo DAC L Mixer      0x1:mute
                      |(0x0 << 13 )  // [   13] DAC L to stereo DAC L Mixer gain 0x0:0db 0x1:-6dB
                      |(0x1 <<  9 )  // [    9] DAC R to stereo DAC L Mixer      0x1:mute
                      |(0x0 <<  8 )  // [    8] DAC R to stereo DAC L Mixer gain 0x0:0db 0x1:-6dB
                      |(0x0 <<  6 )  // [    6] DAC R to stereo DAC R Mixer      0x1:mute
                      |(0x0 <<  5 )  // [    5] DAC R to stereo DAC R Mixer gain 0x0:0db 0x1:-6dB
                      |(0x1 <<  1 )  // [    1] DAC L to stereo DAC R Mixer      0x1:mute
                      |(0x0 <<  0 ); // [    0] DAC L to stereo DAC R Mixer gain 0x0:0db 0x1:-6dB
        alc5616_write_reg(0x2A, data16);
        
        /*stereo ADC to DAC digtal mixer ctrl*/
        data16 = 0x00 |(0x1 << 15 )  // [   15] ADC L to DAC         0x1:mute 
                      |(0x0 << 14 )  // [   14] I2S to DAC L channel 0x1:mute
                      |(0x1 <<  7 )  // [    7] ADC R to DAC         0x1:mute 
                      |(0x0 <<  6 ); // [    6] I2S to DAC R channel 0x1:mute
        alc5616_write_reg(0x29, data16);

        alc5616_write_reg(0xFA, 0x0011);     /*General ctrl*/
        alc5616_write_reg(0x6A, 0x003D);     /*private rigister index*/
        alc5616_write_reg(0x6C, 0x3E00);     /*private register data*/

        /*soft volume&ZCD ctrl*/
        data16 = 0x00 |(0x0 << 15 )  // [   15] Digital soft vol delay ctrl
                      |(0x0 << 13 )  // [   13] OUTVOLL/R soft vol delay ctrl
                      |(0x0 << 12 )  // [   12] HPOVOL:/R soft vol delay ctrl
                      |(0x1 << 11 )  // [   11] Zero cross detection ctrl
                      |(0x1 << 10 )  // [   10] Zeor cross power
                      |(0x1 <<  7 )  // [    7] OUTMIXR mute/unmute ZCD ctrl
                      |(0x1 <<  6 )  // [    6] OUTMIXL mute/unmute ZCD ctrl
                      |(0x1 <<  5 )  // [    5] RECMIXR mute/unmute ZCD ctrl
                      |(0x1 <<  4 )  // [    4] RECMIXL mute/unmute ZCD ctrl
                      |(0x9 <<  0 ); // [ 3: 0] soft vol change delay time
        alc5616_write_reg(0xD9, data16);        
        
        
        /*OUTMIXR ctrl 1*/
        data16 = 0x00 |(0x0 << 10 )  // [12:10] BST2    to OUTMIXR 0x0(0db)~0x6(-18dB)
                      |(0x0 <<  7 )  // [ 9: 7] BST1    to OUTMIXR
                      |(0x0 <<  4 )  // [ 6: 4] INR     to OUTMIXR
                      |(0x0 <<  1 ); // [ 3: 1] RECMIXR to OUTMIXR
        alc5616_write_reg(0x50, data16);
        
        /*OUTMIXR ctrl 2*/
        data16 = 0x00 |(0x0 <<  7 ); // [ 9: 7] DACR to OUTMIXR 0x0(0db)~0x6(-18dB)
        alc5616_write_reg(0x51, data16);
        
        /*OUTMIXR ctrl 3*/
        data16 = 0x00 |(0x1 <<  6 )  // [    6] BST2    to OUTMIXR 0x1:mute
                      |(0x1 <<  5 )  // [    5] BST1    to OUTMIXR 0x1:mute
                      |(0x1 <<  4 )  // [    4] INR     to OUTMIXR 0x1:mute
                      |(0x1 <<  3 )  // [    3] RECMIXR to OUTMIXR 0x1:mute
                      |(0x0 <<  0 ); // [    0] DACR    to OUTMIXR 0x1:mute
        alc5616_write_reg(0x52, data16);
        
        /*OUTMIXL ctrl 1*/
        data16 = 0x00 |(0x0 << 10 )  // [12:10] BST2    to OUTMIXL 0x0(0db)~0x6(-18dB)
                      |(0x0 <<  7 )  // [ 9: 7] BST1    to OUTMIXL
                      |(0x0 <<  4 )  // [ 6: 4] INL     to OUTMIXL
                      |(0x0 <<  1 ); // [ 3: 1] RECMIXL to OUTMIXL
        alc5616_write_reg(0x4D, data16);
        
        /*OUTMIXL ctrl 2*/
        data16 = 0x00 |(0x0 <<  7 ); // [ 9: 7] DACL to OUTMIXL 0x0(0db)~0x6(-18dB)
        alc5616_write_reg(0x4E, data16);
        
        /*OUTMIXL ctrl 3*/
        data16 = 0x00 |(0x1 <<  6 )  // [    6] BST2    to OUTMIXL 0x1:mute
                      |(0x1 <<  5 )  // [    5] BST1    to OUTMIXL 0x1:mute
                      |(0x1 <<  4 )  // [    4] INL     to OUTMIXL 0x1:mute
                      |(0x1 <<  3 )  // [    3] RECMIXL to OUTMIXL 0x1:mute
                      |(0x0 <<  0 ); // [    0] DACL    to OUTMIXL 0x1:mute
        alc5616_write_reg(0x4F, data16);         
        
        /*LineOUTMIX ctrl*/
        data16 = 0x00 |(0x1 << 15 )  // [   15] DACL    to LOUTMIX 0x1:mute
                      |(0x1 << 14 )  // [   14] DACR    to LOUTMIX
                      |(0x1 << 13 )  // [   13] OUTVOLL to LOUTMIX
                      |(0x1 << 12 )  // [   12] OUTVOLR to LOUTMIX
                      |(0x0 << 11 ); // [   11] LOUTMIX gain 0x1:(-6dB) 0x0:0dB
        alc5616_write_reg(0x53, data16);

        /*LineOUT analog vol ctrl*/
        data16 = 0x00 |(0x1 << 15 )  // [   15] LOUTL   mute 0x1:mute
                      |(0x1 << 14 )  // [   14] OUTVOLL mute 0x1:mute 
                      |(0x8 <<  8 )  // [13: 8] OUTVOLL gain 0x0(12dB)~0x27(-46.5dB) 0x8(0dB)
                      |(0x1 <<  7 )  // [    7] LOUTR   mute 0x1:mute
                      |(0x1 <<  6 )  // [    6] OUTVOLR mute 0x1:mute 
                      |(0x8 <<  0 ); // [ 5: 0] OUTVOLR gain 0x0(12dB)~0x27(-46.5dB) 0x8(0dB)
        alc5616_write_reg(0x03, data16);

        /*HPOMIX ctrl*/
        data16 = 0x00 |(0x1 << 14 )  // [   14] DAC(R/L)    to HPOMIX(RL) 0x1:mute 
                      |(0x1 << 13 )  // [   13] HPOVOL(R/L) to HPOMIX(RL) 0x1:mute
                      |(0x0 << 12 ); // [   12] HPOMIX gain 0x0:0dB 0x1:-6dB
        alc5616_write_reg(0x45, data16);
         

        /*Headphone out ctrl*/
        data16 = 0x00 |(0x1 << 15 )  // [   15] HPOL   mute 0x1:mute
                      |(0x1 << 14 )  // [   14] HPOVOLL mute 0x1:mute 
                      |(0x8 <<  8 )  // [13: 8] HPOVOLL gain 0x0(12dB)~0x27(-46.5dB) 0x8(0dB)
                      |(0x1 <<  7 )  // [    7] HPOR   mute 0x1:mute
                      |(0x1 <<  6 )  // [    6] HPOVOLR mute 0x1:mute 
                      |(0x8 <<  0 ); // [ 5: 0] HPOVOLR gain 0x0(12dB)~0x27(-46.5dB) 0x8(0dB)
        alc5616_write_reg(0x02, data16);
        }
    }
    
    _alc5616_cold_start = 0;
    pthread_mutex_unlock(&ALC5616_MUTEX);
}

static void deinit_alc5616_common(void)
{

}

void itp_codec_wake_up(void)
{
    pthread_mutex_lock(&ALC5616_MUTEX);
    printf("ALC5616# %s\n", __func__);

   // init_alc5616_common();
   // _output_path_slect(_alc5616_out); 
   // _alc5616_DA_running = 1;
//#if (CFG_DOORBELL_INDOOR) || (CFG_DOORBELL_ADMIN) || (CFG_DOORBELL_LOBBY)
   // init_alc5616_common();
   // _input_path_slect(_alc5616_in); 
   // _alc5616_AD_running = 1;
//#endif
   // if (_alc5616_mute)
   //     itp_codec_playback_mute();
    pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_standby(void)
{
    pthread_mutex_lock(&ALC5616_MUTEX);
    printf("ALC5616# %s\n", __func__);
//#if (CFG_DOORBELL_INDOOR) || (CFG_DOORBELL_ADMIN) || (CFG_DOORBELL_LOBBY)
    itp_codec_rec_deinit();
//#endif
    itp_codec_playback_deinit();

    pthread_mutex_unlock(&ALC5616_MUTEX);
}

/* DAC */
void itp_codec_playback_init(unsigned output)
{
    DEBUG_PRINT("ALC5616# %s output = %d\n", __func__,output);
    init_alc5616_common();
    
    _alc5616_out = output;
    _output_path_slect(output); 
    usleep(10000); //10(ms) wait
    
    _alc5616_DA_running = 1;
}

void itp_codec_playback_deinit(void)
{
    DEBUG_PRINT("ALC5616# %s\n", __func__);

    _alc5616_DA_running = 0; /* put before deinit_alc5616_common() */
    
    alc5616_write_reg(0x0, 0x0); /*reset alc5616*/
}

void itp_codec_playback_set_direct_vol(unsigned target_vol)
{
    
    if (target_vol < MAX_OUT1_VOLUME){
        printf("ERROR play# invalid target volume step: 0x%08x\n", target_vol);
        return;
    }

    if (target_vol == curr_out1_volume) {return; }

    pthread_mutex_lock(&ALC5616_MUTEX);
    
    curr_out1_volume = target_vol;
    if (_alc5616_DA_running){
        
        alc5616_writeRegMask(0x02, curr_out1_volume << 8, 0x3F << 8);
        alc5616_writeRegMask(0x02, curr_out1_volume << 0, 0x3F << 0);
    
        alc5616_writeRegMask(0x03, curr_out1_volume << 8, 0x3F << 8);
        alc5616_writeRegMask(0x03, curr_out1_volume << 0, 0x3F << 0);      
    }

    pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_playback_set_direct_volperc(unsigned target_volperc)
{
    unsigned char volstep;

    if (target_volperc >= 100) { target_volperc = 99; }

    volstep = alc5616DA_perc_to_reg_table[target_volperc];
    itp_codec_playback_set_direct_vol(volstep);
}

void itp_codec_playback_get_currvol(unsigned *currvol)
{
    *currvol = curr_out1_volume;
}

void itp_codec_playback_get_currvolperc(unsigned *currvolperc)
{
    unsigned i;

    for (i = 0; i < 99; i++)
    {
        if ((alc5616DA_perc_to_reg_table[i] >= curr_out1_volume)
            && (curr_out1_volume > alc5616DA_perc_to_reg_table[i + 1]))
        {
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
    DEBUG_PRINT("%s (%d)\n", __FUNCTION__, __LINE__);
    
    pthread_mutex_lock(&ALC5616_MUTEX);
    /* just decrease output volume */
    alc5616_write_reg(0x2A, 0x6363);                                          /*Stereo DAC digital mixer control ,note:if comment, farend have no voice when communicating press mute */
    alc5616_writeRegMask(0x65, (0 << 14) | (0 << 15), (1 << 14) | (1 << 15)); /*power down OUTMIXL/R note:prevent switch pop sound*/
    
    _alc5616_mute = 1;
    pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_playback_unmute(void)
{
    if (curr_out1_volume == MIN_OUT1_VOLUME)
    {  
        itp_codec_playback_mute();
        return;
    }
    pthread_mutex_lock(&ALC5616_MUTEX);

    DEBUG_PRINT("%s (%d)\n", __FUNCTION__, __LINE__);

    alc5616_write_reg(0x2A, 0x0202);                                          /*Stereo DAC digital mixer control ,note:if comment, farend have no voice when communicating press mute */
    alc5616_writeRegMask(0x65, (1 << 14) | (1 << 15), (1 << 14) | (1 << 15)); /*power on OUTMIXL/R note:prevent switch pop sound*/

    _alc5616_mute = 0;
    pthread_mutex_unlock(&ALC5616_MUTEX);
}

/* line-in bypass to line-out directly */
void itp_codec_playback_linein_bypass(unsigned bypass)
{
//NULL
}

/* ADC */
void itp_codec_rec_init(unsigned input_source)
{
    DEBUG_PRINT("ALC5616# %s\n", __func__);
    
    init_alc5616_common();
    
    _alc5616_in = input_source;
    _input_path_slect(input_source); 
    
    _alc5616_AD_running = 1;
}

void itp_codec_rec_deinit(void)
{
    DEBUG_PRINT("ALC5616# %s\n", __func__);

    _alc5616_AD_running = 0;
    alc5616_write_reg(0x0, 0x0); /*reset alc5616*/

}

void itp_codec_rec_set_direct_vol(unsigned target_vol)
{
    if (target_vol > MAX_INPUT_PGA_VOLUME){
        printf("ERROR rec# invalid target volume step: 0x%08x\n", target_vol);
        return;
    }

    if (target_vol == curr_input_pga_volume) { return; }

    pthread_mutex_lock(&ALC5616_MUTEX);
    
    curr_input_pga_volume = target_vol;
    if (_alc5616_AD_running){
        alc5616_writeRegMask(0x0D, curr_input_pga_volume << 12, 0xF << 12);
        alc5616_writeRegMask(0x0D, curr_input_pga_volume <<  8, 0xF <<  8);
    }
    
    pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_rec_set_direct_volperc(unsigned target_micperc)
{   
	unsigned int micstep;
	if(target_micperc >= 100) { target_micperc = 99; }   
	micstep = alc5616AD_perc_to_reg_table[target_micperc];
    itp_codec_rec_set_direct_vol(micstep);
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
    pthread_mutex_lock(&ALC5616_MUTEX);
  
    if (_alc5616_AD_running)
    {
        alc5616_writeRegMask(0x0D, MIN_INPUT_PGA_VOLUME << 12, 0xF << 12);
        alc5616_writeRegMask(0x0D, MIN_INPUT_PGA_VOLUME << 8 , 0xF <<  8);
        alc5616_writeRegMask(0x3C, (1 << 2), (1 << 2));
        alc5616_writeRegMask(0x3E, (1 << 2), (1 << 2));
    }

    pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_rec_unmute(void)
{
    pthread_mutex_lock(&ALC5616_MUTEX);
    
    if (_alc5616_AD_running){
        alc5616_writeRegMask(0x0D, curr_input_pga_volume << 12, 0xF << 12);
        alc5616_writeRegMask(0x0D, curr_input_pga_volume << 8 , 0xF <<  8);
        alc5616_writeRegMask(0x3C, (0 << 2), (1 << 2));
        alc5616_writeRegMask(0x3E, (0 << 2), (1 << 2));
    }
    
    pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_power_on(void)
{

}

void itp_codec_power_off(void)
{

}

void itp_codec_get_i2s_sample_rate(int *samplerate)
{
    pthread_mutex_lock(&ALC5616_MUTEX);
    *samplerate = i2s_sample_rate;
    pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_set_i2s_sample_rate(int samplerate)
{
    pthread_mutex_lock(&ALC5616_MUTEX);
    i2s_sample_rate = samplerate;
    pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_set_DAC_slave_mode(int mode)
{
    //_alc5616_slave =1 :Codec slave  mode(i2s master mode):mormal  use
    //_alc5616_slave =0 :Codec master mode(i2s slave  mode):special case
    _alc5616_slave = mode;
}

int itp_codec_get_DA_running(void)
{
    return _alc5616_DA_running;
}

int itp_codec_get_AD_running(void)
{
    return _alc5616_AD_running;
}