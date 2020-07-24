#if 0
//=============================================================================
//							LOG
//=============================================================================
#define LOG_DBG     1
#define LOG_VDBG    1
#define LOG_INFO    1
#define LOG_ERROR   1

#if LOG_DBG
#define DBG(ddev, string, args...)    do { ithPrintf("[NCM][DBG] "); ithPrintf(string, ## args); } while (0)
#else
#define DBG(ddev, string, args...)
#endif

#if LOG_VDBG
#define VDBG(ddev, string, args...)    do { ithPrintf("[NCM][VDBG] "); ithPrintf(string, ## args); } while (0)
#else
#define VDBG(ddev, string, args...)
#endif

#if LOG_INFO
#define INFO(ddev, string, args...)    do { ithPrintf("[NCM][INFO] "); ithPrintf(string, ## args); } while (0)
#else
#define INFO(ddev, string, args...)
#endif

#if LOG_ERROR
#define ERROR(ddev, string, args...)    do { ithPrintf("[NCM][ERROR] "); ithPrintf(string, ## args); } while (0)
#else
#define ERROR(ddev, string, args...)
#endif


#define check_result(rc) do { if (rc) ERROR(NULL, "[%s] res = %d(0x%08X) \n", __FUNCTION__, rc, rc); } while (0)
#endif
