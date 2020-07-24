/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <log.h>
#include "../ssv_drv_config.h"
#include "../ssv_drv_if.h"
#include "sdio.h"


// return 
//  < 0 : fail
// >= 0 : # of bytes recieve
ssv_type_s32 	recv_sdio_impl(ssv_type_u8 *dat, ssv_type_size_t len)
{
#if 0
	u32			rx_ask_len, rx_recv_len;
	const u32	polling_thd = 5000;
	u32			i;
#ifdef PERFORMANCE_MEASURE
	unsigned __int64 freq;
	unsigned __int64 startTime;
	unsigned __int64 endTime;
	double timerFrequency;
#endif
	
	// SDIO_TRACE("%s <= \n", __FUNCTION__);
	rx_ask_len = rx_recv_len = i = 0;
	while (1)
	{
#ifdef PERFORMANCE_MEASURE
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		timerFrequency = (1.0/freq);

		QueryPerformanceCounter((LARGE_INTEGER *)&startTime);
#endif
		// polling the sdio INT_STATUS register
		if ((rx_ask_len = sdio_ask_rx_data_len()) == 0)
		{

			return -1;
			//continue;
		}
#ifdef PERFORMANCE_MEASURE
		QueryPerformanceCounter((LARGE_INTEGER *)&endTime);
		//if(((endTime-startTime) * timerFrequency) > 0.001)
			LOG_PRINTF("time-1 %f\n", ((endTime-startTime) * timerFrequency));
		
		QueryPerformanceCounter((LARGE_INTEGER *)&startTime);
#endif
		rx_recv_len = sdio_read_data(dat, rx_ask_len);
#ifdef PERFORMANCE_MEASURE
		QueryPerformanceCounter((LARGE_INTEGER *)&endTime);
		//if(((endTime-startTime) * timerFrequency) > 0.001)
			LOG_PRINTF("time-2 %f\n", ((endTime-startTime) * timerFrequency));
#endif
		if (rx_recv_len  == -1)
		{
			// SDIO_FAIL_RET(-1, "sdio_read_data(0x%08x, %d)  = -1 !\n", dat, rx_recv_len);
			return -1;
		}
		if (rx_recv_len != rx_ask_len)
		{
			// SDIO_FAIL_RET(-1, "sdio_read_data(0x%08x, %d) != %d !\n", dat, rx_recv_len, rx_ask_len);
			return -1;
		}
		// otherwise, success!, exit the while loop
		break;
	}
	return rx_recv_len;
#else
	return sdio_read_dataEx(dat, len);
#endif
}

ssv_type_s32 	send_sdio_impl(void *dat, ssv_type_size_t len)
{
	SDIO_TRACE("%s <= \n", __FUNCTION__);

	return 0;
}

ssv_type_bool	get_name_sdio_impl(char name[32])
{
	SDIO_TRACE("%s <= : name = %s\n", __FUNCTION__, name);

	strcpy(name, DRV_NAME_SDIO);
	return true;
}

ssv_type_bool	ioctl_sdio_impl(ssv_type_u32 ctl_code, 
					void *in_buf, ssv_type_size_t in_size,	
					void *out_buf, ssv_type_size_t out_size, 
					ssv_type_size_t *bytes_ret)
{
	SDIO_TRACE("%s <= \n", __FUNCTION__);
	return true;
}


const struct ssv6xxx_drv_ops	g_drv_sdio =
{
	DRV_NAME_SDIO,
	sdio_open,
	sdio_close,
	sdio_init,
	recv_sdio_impl,     //recv
	sdio_write_data,	//send 

    get_name_sdio_impl, //get_name
	ioctl_sdio_impl,    //ioctl
    sdio_handle,
    sdio_ack_int,

    sdio_write_sram,    //write sram
    sdio_read_sram,     //read sram

    sdio_write_reg,     //write reg
    sdio_read_reg       //read reg
};
