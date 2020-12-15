
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define SD_SECTOR_SIZE (512)
#define SD_HEADER_SIZE (128*SD_SECTOR_SIZE)
#define SD_HEADER_V2    "SDBOOT02"
#define ROM_HEADER_V1   "SMEDIA01"
#define ROM_HEADER_V2   "SMEDIA02"
#define MAX_CMD_SIZE    (4*1024*1024*sizeof(int))
#define MAX_REM_SIZE    (4096)
#define MAX_STACK_SIZE  (4096)

#endif // __CONFIG_H__
