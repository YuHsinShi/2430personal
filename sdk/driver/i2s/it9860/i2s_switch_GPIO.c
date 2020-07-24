#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ite/ith.h"
#include "ite/itp.h"
#include "i2s/i2s.h"
#include "i2s_reg_9860.h"

void GPIO_switch_set(AudioInOutCase enable){
    int CHANEL_SEL = CFG_CHANEL_SEL;
    int CHANEL_ENB = CFG_CHANEL_ENB;
    ithGpioSetOut(CHANEL_ENB);
    ithGpioSetOut(CHANEL_SEL);
    switch (enable)
    {
    case HANDFREE:
        {/* hand free */
            itp_codec_playback_init(0);//Faraday DAC ONLY right
            ithWriteRegMaskA(I2S_REG_OUT_CTRL, 1<<6|0<<4, 1<<6|1<<4);//l channels ,right high
            itp_codec_rec_init(0);
            ithWriteRegMaskA(I2S_REG_IN_CTRL , 1<<6|0<<4, 1<<6|1<<4);//l channels ,right high
            ithGpioClear(CHANEL_ENB);
            ithGpioClear(CHANEL_SEL);
            break;
        }
    case HEADSET:
        {/* head set */
            itp_codec_playback_init(1);//Faraday DAC ONLY left
            ithWriteRegMaskA(I2S_REG_OUT_CTRL, 0<<6|0<<4, 1<<6|1<<4);//l channels ,left high
            itp_codec_rec_init(1);
            ithWriteRegMaskA(I2S_REG_IN_CTRL, 0<<6|0<<4, 1<<6|1<<4);//l channels ,left high
            ithGpioSetOut(CHANEL_ENB);
            ithGpioClear(CHANEL_SEL);
            break;
        }
    case TELEPHONETUBE:
        {/* telephone tube */
            itp_codec_playback_init(1);//Faraday DAC ONLY left
            ithWriteRegMaskA(I2S_REG_OUT_CTRL, 0<<6|0<<4, 1<<6|1<<4);//l channels ,left high
            itp_codec_rec_init(1);
            ithWriteRegMaskA(I2S_REG_IN_CTRL , 0<<6|0<<4, 1<<6|1<<4);//l channels ,left high
            ithGpioSetOut(CHANEL_ENB);
            ithGpioSet(CHANEL_SEL);
            break;
        }
    case DOUBLE_HEADSET:
        {
            itp_codec_playback_init(2);//Faraday DAC LEFT/RIGHT
            ithWriteRegMaskA(I2S_REG_OUT_CTRL, 0<<6|0<<4, 1<<6|1<<4);//l channels ,left high
            itp_codec_rec_init(1);
            ithWriteRegMaskA(I2S_REG_IN_CTRL, 0<<6|0<<4, 1<<6|1<<4);//1 channel ,left high
            ithGpioSetOut(CHANEL_ENB);
            ithGpioSet(CHANEL_SEL);
            break;
        }
    case DOUBLE_TELETUBE:
        {
            itp_codec_playback_init(2);//Faraday DAC LEFT/RIGHT
            ithWriteRegMaskA(I2S_REG_OUT_CTRL, 0<<6|0<<4, 1<<6|1<<4);//l channels ,left high
            itp_codec_rec_init(1);
            //ithWriteRegMaskA(I2S_REG_OUT_CTRL, 0<<6||1<<4, 1<<6||1<<4);//2 channel ,left high
            ithWriteRegMaskA(I2S_REG_IN_CTRL, 0<<6|0<<4, 1<<6|1<<4);
            ithGpioSetOut(CHANEL_ENB);
            ithGpioSetOut(CHANEL_SEL);
            break;
        }
    default:
        {
            itp_codec_playback_init(1);
            itp_codec_rec_init(1);
            //ithWriteRegMaskA(I2S_REG_OUT_CTRL, 0<<6, 0<<6);
            ithGpioClear(CHANEL_ENB);
            ithGpioClear(CHANEL_SEL);
            break;
        }
    }
    printf("Gpio%d:%d Gpio%d:%d\n",CHANEL_SEL,ithGpioGet(CHANEL_SEL),CHANEL_ENB,ithGpioGet(CHANEL_ENB));

}