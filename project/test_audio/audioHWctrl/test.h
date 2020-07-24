#define ADC_BUFFER_SIZE (256*1024)
#define DAC_BUFFER_SIZE (256*1024)

typedef signed   long long s64;//int64_t
typedef signed   int       s32;//int32_t
typedef signed   short     s16;//int16_t
typedef signed   char      s8; //int8_t
typedef unsigned long long u64;//uint64_t
typedef unsigned int       u32;//uint32_t
typedef unsigned short     u16;//uint16_t
typedef unsigned char      u8; //uint8_t

static STRC_I2S_SPEC spec_daI = {0};
static STRC_I2S_SPEC spec_adI = {0};

uint8_t *dac_buf;
uint8_t *adc_buf;
uint8_t *HDMI_Ibuf[4];
uint8_t *HDMI_Obuf[4];

