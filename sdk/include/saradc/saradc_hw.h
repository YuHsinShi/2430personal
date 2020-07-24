#if (CFG_CHIP_FAMILY == 970)
    #include "it970/saradc_hw.h"
#elif (CFG_CHIP_FAMILY == 9860)
    #include "it9860/saradc_hw.h"
#else
    #error "not defined"
#endif

