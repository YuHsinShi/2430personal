

#define DEV_DBG     0
#define DEV_ERR     1
#define DEV_INFO    0
#define DEV_WARN    1
#define PR_ERR      1
#define PR_DEBUG    0
#define PR_INFO     0
#define PR_WARN     1
#define DBG_FUNC    0


#if DEV_DBG
#define dev_dbg(ddev, string, args...)    do { ithPrintf("[Option][dev_dbg] "); ithPrintf(string, ## args); } while (0)
#else
#define dev_dbg(ddev, string, args...)
#endif

#if DEV_ERR
#define dev_err(ddev, string, args...)    do { ithPrintf("[Option][dev_err] "); ithPrintf(string, ## args); } while (0)
#else
#define dev_err(ddev, string, args...)
#endif

#if DEV_INFO
#define dev_info(ddev, string, args...)    do { ithPrintf("[Option][dev_info] "); ithPrintf(string, ## args); } while (0)
#else
#define dev_info(ddev, string, args...)
#endif

#if DEV_WARN
#define dev_warn(ddev, string, args...)    do { ithPrintf("[Option][dev_warn] "); ithPrintf(string, ## args); } while (0)
#else
#define dev_warn(ddev, string, args...)
#endif

#if PR_ERR
#define pr_err(string, args...)    do { ithPrintf("[USerial][pr_err] "); ithPrintf(string, ## args); } while (0)
#else
#define pr_err(string, args...)
#endif

#if PR_DEBUG
#define pr_debug(string, args...)    do { ithPrintf("[USerial][pr_debug] "); ithPrintf(string, ## args); } while (0)
#else
#define pr_debug(string, args...)
#endif

#if PR_INFO
#define pr_info(string, args...)    do { ithPrintf("[USerial][pr_info] "); ithPrintf(string, ## args); } while (0)
#else
#define pr_info(string, args...)
#endif

#if PR_WARN
#define WARN(n, string, args...)           do { ithPrintf("[USerial][WARN] "); ithPrintf(string, ## args); } while (0)
#else
#define WARN(n, string, args...)
#endif

#if DBG_FUNC
#define __func_enter__ ithPrintf("%s enter \n", __func__);
#define __func_leave__ ithPrintf("%s leave \n", __func__);
#else
#define __func_enter__ 
#define __func_leave__
#endif
