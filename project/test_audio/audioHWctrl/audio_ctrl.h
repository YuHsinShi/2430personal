#ifndef I2S_H
#define I2S_H

#include "i2s/i2s.h"

#ifdef __cplusplus
extern "C" {
#endif

void audio_init_DA(STRC_I2S_SPEC *i2s_spec);
void audio_init_AD(STRC_I2S_SPEC *i2s_spec);

void audio_deinit_DA();
void audio_deinit_AD();

void audio_pause_DA(int pause);
void audio_pause_AD(int pause);

void audio_mute_DA(int mute);
void audio_mute_AD(int mute);

void audio_DA_volume_set(unsigned level);
void audio_AD_volume_set(unsigned level);

int audio_TX_Data_Send(STRC_I2S_SPEC *i2s_spec,unsigned char *ptr,int size);
int audio_RX_Data_Send(STRC_I2S_SPEC *i2s_spec,unsigned char *ptr,int size);

void audio_DA_wait_time(STRC_I2S_SPEC *i2s_spec,int size);
void audio_AD_wait_time(STRC_I2S_SPEC *i2s_spec,int size);

#ifdef __cplusplus
}
#endif

#endif //I2S_H

