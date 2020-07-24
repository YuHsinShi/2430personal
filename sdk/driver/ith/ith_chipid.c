/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL Chip ID functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static ITHPackageId packageId = -1;

ITHPackageId ithGetPackageId(void)
{
    if (packageId != -1)
        return packageId;

#if CFG_CHIP_FAMILY == 970
    // TODO

#elif CFG_CHIP_FAMILY == 9860
    // TODO

#else
#error "No project defined"
#endif

    return packageId;
}
