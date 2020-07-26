#ifndef SHT20_SENSOR_H
#define SHT20_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

void SHT20_Init(void);

float SHT20_Detect(void);

#ifdef __cplusplus
}
#endif

#endif /* SHT20_SENSOR_H */