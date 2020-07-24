#ifndef _NET_APP_H_
#define _NET_APP_H_


#define IPV4_ADDR(a)    (((ssv_type_u8*)(a))[0]) & 0xff, (((ssv_type_u8*)(a))[1]) & 0xff, (((ssv_type_u8*)(a))[2]) & 0xff, (((ssv_type_u8*)(a))[3]) & 0xff

#if NETAPP_SUPPORT

typedef void (*net_app_func)(ssv_type_s32, char **);


typedef struct net_app_info_st
{
    char            *app_name;
    net_app_func    app_func;
    ssv_type_bool            need_to_restrict; /* Restrict task just used one thread*/
} net_app_info;


/**
* The arguments for the net_app task. 
*/
typedef struct net_app_args_st 
{
    char            *cmd_buf; /* to hold command line buffer */
    net_app_info    *app_info;
} net_app_args;



ssv_type_s32 net_app_init(void);
ssv_type_s32 net_app_run(ssv_type_s32 argc, char *argv[]);
ssv_type_s32 net_app_show(void);
void net_app_task(void *args);


#endif //#if NETAPP_SUPPORT


#endif /* _NET_APP_H_ */

