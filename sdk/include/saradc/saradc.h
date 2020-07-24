#if (CFG_CHIP_FAMILY == 970)
    #include "it970/saradc.h"
#elif (CFG_CHIP_FAMILY == 9860)
    #include "it9860/saradc.h"
#else
    #error "not defined"
#endif

