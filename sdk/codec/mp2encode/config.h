
//#define WIN32
#if !defined(WIN32) && !defined(__CYGWIN__) && !defined(__OR32__)
#  define __OR32__
#endif

#define LOOKUP_TABLE

#define ITE_RISC
//#define MP2_ENABLE_INTERNAL_SD 
//#define INTERNAL_SD 0x40000000

//#define MP2_ENCODE_PERFORMANCE_TEST_BY_TICK

#if defined(WIN32) || defined(__CYGWIN__)
#  include "win32.h"
#elif defined(__OR32__)
#  include "mmio.h"
#endif

#if defined(WIN32)
typedef __int64                 int64_t;
typedef unsigned __int64        uint64_t;
#else
typedef long long               int64_t;
typedef unsigned long long      uint64_t;
#endif

typedef signed char             int8_t;
typedef short                   int16_t;
typedef long                    int32_t;
typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned long           uint32_t;

/******************************
 The Buffer size of input stream
 ******************************/
#define READBUF_SIZE        (4608*12)

/******************************
 The Buffer size of output stream
 ******************************/
 #define OUTPUT_ONE_FRAME 
 #ifdef OUTPUT_ONE_FRAME
 #define OUTFRAME_SIZE 1000 
 #else
 #define OUTFRAME_SIZE 3000
 #endif
 
#define OUTBUF_SIZE     (3000*16)  //(1024*16)


