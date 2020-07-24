/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include "sdio.h"
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <hctrl.h>
#include <ssv_hal.h>


static int	s_hSDIO				= 0;		// where is ONLY one SDIO handle in this module!
static ssv_type_u16	s_cur_block_size	= SDIO_DEF_BLOCK_SIZE;
static OsMutex	s_SDIOMutex;
static ssv_type_u32	s_write_data_cnt	= 1;
static ssv_type_u8	s_func_focus	= -1;
static ssv_type_bool s_sdio_data_mode = true; // Power on status is data mode.

const char*	_err_str(ssv_type_u32 err);

ssv_type_bool	sdio_set_function_focus(int func);
ssv_type_bool	sdio_get_function_focus(ssv_type_u8 out_buf[1]);

const char* _err_str(ssv_type_u32 err)
{
    switch (err)
    {
        case 0: return "The operation completed successfully.";
        case 1: return "Incorrect function.";
        case 2: return "The system cannot find the file specified.";
        case 3: return "The system cannot find the path specified.";
		case 4: return "The system cannot open the file.";
		case 5: return "Access is denied.";
        case 6: return "The handle is invalid.";
        default: return "Other. Plz visit MSDN website.";
    }
}

ssv_type_u32	sdio_handle(void)
{
	return s_hSDIO;
}

#define	SDIO_CHECK_OPENED()		{ if (s_hSDIO == 0)	SDIO_FAIL_RET(0, "SDIO device is NOT opened!\n");	}



ssv_inline ssv_type_u16	sdio_get_cur_block_size(void)
{
	return s_cur_block_size;
}

ssv_type_u16		sdio_get_block_size(void)
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u8		in_buf[4], out_buf[2];
	int	status;
	ssv_type_u16		r = 0;

	SDIO_CHECK_OPENED();

	cmd.in_data_len = 4;
	cmd.in_data = in_buf;
	cmd.out_data_len = 2;
	cmd.out_data = out_buf;
	ssv6xxx_memset((void *)out_buf,0x00, cmd.out_data_len);

	OS_MutexLock(s_SDIOMutex);
	status = ioctl(s_hSDIO,
							IOCTL_SSV6XXX_SDIO_GET_BLOCKLEN,&cmd);
	OS_MutexUnLock(s_SDIOMutex);
	if (status < 0)
	{
		SDIO_ERROR("GET_BLOCKLEN\n");
		return false;
	}
	r = (out_buf[0] | (out_buf[1] << 8));
	SDIO_TRACE("%-20s : %d\n", "GET_BLOCKLEN", r);
	return true;
}

// note : this func will automatically update 's_cur_block_size' value after success
ssv_type_bool	sdio_set_block_size(ssv_type_u16 size)
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u8		in_buf[2], out_buf[1];
	int	status;

	SDIO_CHECK_OPENED();

	cmd.in_data_len = 2;
	cmd.in_data = in_buf;
	cmd.out_data_len = 1;
	cmd.out_data = out_buf;

	in_buf[0] = ((ssv_type_u8)(size >> 0) & 0xff);
	in_buf[1] = ((ssv_type_u8)(size >> 8) & 0xff);
	OS_MutexLock(s_SDIOMutex);
	status = ioctl(s_hSDIO, IOCTL_SSV6XXX_SDIO_SET_BLOCKLEN,&cmd);
	OS_MutexUnLock(s_SDIOMutex);
	if (status < 0)
	{
		SDIO_ERROR("SET_BLOCKLEN\n");
		return false;
	}
	SDIO_TRACE("%-20s : %d\n", "SET_BLOCKLEN", size);
	s_cur_block_size = size;
	return true;
}

ssv_type_bool	sdio_set_function_focus(int func)
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u8		in_buf[1],out_buf[1];
	int		status;

	SDIO_CHECK_OPENED();

	if ( s_func_focus == func )
	{
		return;
	}

	s_func_focus = func;

	in_buf[0] = func;

	cmd.in_data_len = 1;
	cmd.in_data = in_buf;
	cmd.out_data_len = 1;
	cmd.out_data = out_buf;

	OS_MutexLock(s_SDIOMutex);
	status = ioctl(s_hSDIO, IOCTL_SSV6XXX_SDIO_SET_FUNCTION_FOCUS,&cmd);
	OS_MutexUnLock(s_SDIOMutex);
	if (status < 0)
	{
		SDIO_ERROR("SET_FUNCTION_FOCUS\n");
		return false;
	}
	SDIO_TRACE("%-20s : %d\n", "SET_FUNCTION_FOCUS", func);
	return true;
}

ssv_type_bool	sdio_get_function_focus(ssv_type_u8 out_buf[1])
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u8		in_buf[4];
	int		status;

	SDIO_CHECK_OPENED();

	cmd.in_data_len = 4;
	cmd.in_data = in_buf;
	cmd.out_data_len = 1;
	cmd.out_data = out_buf;

	OS_MutexLock(s_SDIOMutex);
	status = ioctl(s_hSDIO, IOCTL_SSV6XXX_SDIO_GET_FUNCTION_FOCUS,&cmd);
	OS_MutexUnLock(s_SDIOMutex);
	if (status < 0)
	{
		SDIO_ERROR("GET_FUNCTION_FOCUS\n");
		return false;
	}
	SDIO_TRACE("%-20s : %d (0x%02x)\n", "GET_FUNCTION_FOCUS", out_buf[0], out_buf[0]);
	return true;
}

ssv_type_bool		sdio_read_reg(ssv_type_u32 addr, ssv_type_u32 *data)
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u8		in_buf[4], out_buf[4];
	int	status;
	ssv_type_u32		r;

	SDIO_CHECK_OPENED();

	cmd.in_data_len = 4;
	cmd.in_data = in_buf;
	cmd.out_data_len = 4;
	cmd.out_data = out_buf;

	in_buf[0] = ((ssv_type_u8)(addr >> 0) & 0xff);
	in_buf[1] = ((ssv_type_u8)(addr >> 8) & 0xff);
	in_buf[2] = ((ssv_type_u8)(addr >> 16) & 0xff);
	in_buf[3] = ((ssv_type_u8)(addr >> 24) & 0xff);
	OS_MutexLock(s_SDIOMutex);
	status = ioctl(s_hSDIO, IOCTL_SSV6XXX_SDIO_READ_REG,&cmd);
	OS_MutexUnLock(s_SDIOMutex);
	if (status < 0)
	{
		SDIO_ERROR("READ_REG, status = %d\n", status);
		return FALSE;
	}

	r = (((ssv_type_u32)(out_buf[0]) << 0) |
		 ((ssv_type_u32)(out_buf[1]) << 8 ) |
		 ((ssv_type_u32)(out_buf[2]) << 16) |
		 ((ssv_type_u32)(out_buf[3]) << 24));
	SDIO_TRACE("%-20s() : 0x%08x, 0x%08x\n", "READ_REG", addr, r);
    *data = r;

	return TRUE;
}

ssv_type_bool	sdio_write_reg(ssv_type_u32 addr, ssv_type_u32 data)
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u8		in_buf[8], out_buf[4];
	int	status;

	SDIO_CHECK_OPENED();

	cmd.in_data_len = 8;
	cmd.in_data = in_buf;
	cmd.out_data_len = 4;
	cmd.out_data = out_buf;

	in_buf[0] = ((ssv_type_u8)(addr >> 0) & 0xff);
	in_buf[1] = ((ssv_type_u8)(addr >> 8) & 0xff);
	in_buf[2] = ((ssv_type_u8)(addr >> 16) & 0xff);
	in_buf[3] = ((ssv_type_u8)(addr >> 24) & 0xff);
	in_buf[4] = ((ssv_type_u8)(data >> 0) & 0xff);
	in_buf[5] = ((ssv_type_u8)(data >> 8) & 0xff);
	in_buf[6] = ((ssv_type_u8)(data >> 16) & 0xff);
	in_buf[7] = ((ssv_type_u8)(data >> 24) & 0xff);
	OS_MutexLock(s_SDIOMutex);
	status = ioctl(s_hSDIO,IOCTL_SSV6XXX_SDIO_WRITE_REG,&cmd);
	OS_MutexUnLock(s_SDIOMutex);
	if (status < 0 )
	{
		SDIO_ERROR("WRITE_REG\n");
		return false;
	}

	SDIO_TRACE("%-20s : 0x%08x, 0x%08x\n", "WRITE_REG", addr, data);
	return true;
}

void    sdio_set_data_mode(ssv_type_bool use_data_mode)
{
	if (use_data_mode)
	{
		sdio_write_byte(1,0x0c,0);
		s_sdio_data_mode = true;
	}
	else
	{
        sdio_write_byte(1,0x0c,0x2);
		s_sdio_data_mode = false;
	}
}

//Only for SDIO register access
ssv_type_u8	sdio_read_byte(ssv_type_u8 func,ssv_type_u32 addr)
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u8		in_buf[4], out_buf[4];
	int	status;

	SDIO_CHECK_OPENED();

	sdio_set_function_focus(func);

	cmd.in_data_len = 4;
	cmd.in_data = in_buf;
	cmd.out_data_len = 4;
	cmd.out_data = out_buf;

	in_buf[0] = ((ssv_type_u8)(addr >> 0) & 0xff);
	in_buf[1] = ((ssv_type_u8)(addr >> 8) & 0xff);
	in_buf[2] = ((ssv_type_u8)(addr >> 16) & 0xff);
	in_buf[3] = ((ssv_type_u8)(addr >> 24) & 0xff);

	OS_MutexLock(s_SDIOMutex);
	status = ioctl(s_hSDIO,IOCTL_SSV6XXX_SDIO_READ_BYTE,&cmd);
	OS_MutexUnLock(s_SDIOMutex);

	if (status < 0)
	{
		SDIO_ERROR("READ_BYTE, status = %d\n", status);
		return 0x00;
	}

	SDIO_TRACE("%-20s : 0x%08x, 0x%02x\n", "READ_BYTE", addr, out_buf[0]);
	return out_buf[0];
}

//Only for SDIO register access
ssv_type_bool	sdio_write_byte(ssv_type_u8 func,ssv_type_u32 addr, ssv_type_u8 data)
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u8		in_buf[5], out_buf[4];
	ssv_type_u32		in_len, out_len, dwRet;
	int	status;

	SDIO_CHECK_OPENED();

	sdio_set_function_focus(func);

	cmd.in_data_len = 5;
	cmd.in_data = in_buf;
	cmd.out_data_len = 4;
	cmd.out_data = out_buf;

	in_buf[0] = ((ssv_type_u8)(addr >> 0) & 0xff);
	in_buf[1] = ((ssv_type_u8)(addr >> 8) & 0xff);
	in_buf[2] = ((ssv_type_u8)(addr >> 16) & 0xff);
	in_buf[3] = ((ssv_type_u8)(addr >> 24) & 0xff);
	in_buf[4] = data;
	OS_MutexLock(s_SDIOMutex);
	status = ioctl(s_hSDIO, IOCTL_SSV6XXX_SDIO_WRITE_BYTE,&cmd);
	OS_MutexUnLock(s_SDIOMutex);
	if (status < 0 )
	{
		SDIO_ERROR("WRITE_BYTE\n");
		return false;
	}

	SDIO_TRACE("%-20s : 0x%08x, 0x%02x\n", "WRITE_BYTE", addr, data);
	return true;
}

ssv_type_s32	sdio_ask_rx_data_len(void)
{
	ssv_type_u8		int_status;
	ssv_type_u32		r = 0;

#if (CONFIG_RX_POLL == 1)
	do
	{
		int_status = sdio_read_byte(1,REG_INT_STATUS);
		if (int_status & 0x01)
		{
			r = sdio_read_byte(1,REG_CARD_PKT_LEN_0) & 0xff;
			r = r | ((sdio_read_byte(1,REG_CARD_PKT_LEN_1) & 0xff) << 8);
			return r;
		}
	}while(r==0);
#else
	{
		int_status = sdio_read_byte(1,REG_INT_STATUS);
		if (int_status & 0x01)
		{
			r = sdio_read_byte(1,REG_CARD_PKT_LEN_0) & 0xff;
			r = r | ((sdio_read_byte(1,REG_CARD_PKT_LEN_1) & 0xff) << 8);
			return r;
		}
	}
#endif
	return r;
}

ssv_type_s32		sdio_read_data(ssv_type_u8 *dat, ssv_type_size_t size)
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u8		in_buf[4], *out_buf = NULL;
	int	status;


	SDIO_CHECK_OPENED();
	SDIO_TRACE("%s() <= : size = %d\n", __FUNCTION__, size);

	//SDIO_TRACE("out_buf = 0x%08x, out_len = %d\n", out_buf, out_len);
	//SDIO_TRACE("block_count = %d, block_size = %d\n", block_count, block_size);

	// Note :
	//	- the in_len & in_buf is 'fake'
	//	- the underlining IOCTL_SSV6XXX_SDIO_WRITE_MULTI_BYTE opeation will use 'out_len' to decide how many data should be read
	//
	cmd.in_data_len = 4;
	cmd.in_data = in_buf;
	cmd.out_data_len = size;
	cmd.out_data = dat;

	OS_MutexLock(s_SDIOMutex);
	status = ioctl(s_hSDIO, IOCTL_SSV6XXX_SDIO_READ_MULTI_BYTE,&cmd);
	OS_MutexUnLock(s_SDIOMutex);
	if (status < 0)
	{
		// SDIO_FAIL_RET(-1, "READ_MULTI_BYTE, status = FALSE\n");
		return -1;
	}

	// SDIO_TRACE("%-20s : len = %d (0x%08x)\n", "READ_MULTI_BYTE", out_len, out_len);
	return size;
}

ssv_type_s32		sdio_read_dataEx(ssv_type_u8 *dat, ssv_type_size_t size)
{
	return read(s_hSDIO,dat,size);
}

#define SDOI_DUMMY_SIZE	(SDIO_DEF_BLOCK_SIZE>>SDIO_TX_ALLOC_SIZE_SHIFT)


ssv_type_s32	sdio_write_data(void *dat, ssv_type_size_t size)
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u32		result32=0;
	ssv_type_u32 free_tx_page=0, free_tx_id=0;
    struct ssv6xxx_hci_txq_info *txq_info;
	ssv_type_u32		retry_count=0;
	ssv_type_u8		result;

	ssv_type_u8		out_buf[4];
	int	status;
	ssv_type_u32     alloc_size;

	SDIO_CHECK_OPENED();
	SDIO_TRACE("%s() <= : size = %d\n", __FUNCTION__, size);

	if (s_sdio_data_mode)
	{
		//Page offset 80  HCI reserve 16 * 3 ( 80 + 48)
		alloc_size = size + 128;

		//MMU 1 page = 256 bytes
		if(alloc_size % 256)
			alloc_size = (alloc_size>>8)+1;
		else
			alloc_size = (alloc_size>>8);

		do{
			sdio_read_reg(ADR_TX_ID_ALL_INFO, &result32);
			txq_info = (struct ssv6xxx_hci_txq_info*)(&result32);

			free_tx_page = SSV6200_PAGE_TX_THRESHOLD - txq_info->tx_use_page;
			free_tx_id	 = SSV6200_ID_TX_THRESHOLD - txq_info->tx_use_id;
			if((free_tx_page >= alloc_size) && free_tx_id)
			{
				if((SSV6200_ID_AC_BK_OUT_QUEUE == txq_info->txq0_size)||
				(SSV6200_ID_AC_BE_OUT_QUEUE == txq_info->txq1_size)||
				(SSV6200_ID_AC_VI_OUT_QUEUE == txq_info->txq2_size)||
				(SSV6200_ID_AC_VO_OUT_QUEUE == txq_info->txq3_size))
					;
				else
					break;
			}
		}while(1);
	}

	cmd.in_data_len = size;
	cmd.in_data = dat;
	cmd.out_data_len = 4;
	cmd.out_data = out_buf;
    s_write_data_cnt++;
	// LOG_PRINTF("sdio_write_data : # %d\n", s_write_data_cnt);

	OS_MutexLock(s_SDIOMutex);

	status = ioctl(s_hSDIO,IOCTL_SSV6XXX_SDIO_WRITE_MULTI_BYTE,&cmd);

	OS_MutexUnLock(s_SDIOMutex);

	if (status < 0)
		SDIO_FAIL_RET(-1, "WRITE_MULTI_BYTE, status = FALSE\n");

	return size;
}

ssv_type_bool	sdio_get_multi_byte_io_port(ssv_type_u32 *port)
{
	ssv_type_u8	buf[3];

	SDIO_CHECK_OPENED();

	*port = 0;
	//memset(buf, 0x00, 3);
	buf[0] = sdio_read_byte(1,REG_DATA_IO_PORT_0);
	buf[1] = sdio_read_byte(1,REG_DATA_IO_PORT_1);
	buf[2] = sdio_read_byte(1,REG_DATA_IO_PORT_2);
	*port = (((buf[0] & 0xff) << 0) |
			 ((buf[1] & 0xff) << 8) |
			 ((buf[2] & 0x01) << 16));
	SDIO_TRACE("%-20s : %d (0x%08x)\n", "GET_MULTI_BYTE_IO_PORT", *port, *port);
	return true;
}

ssv_type_bool	sdio_set_multi_byte_io_port(ssv_type_u32 port)
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u8		in_buf[4], out_buf[4];
	int	status;

	SDIO_CHECK_OPENED();

	cmd.in_data_len = 4;
	cmd.in_data = in_buf;
	cmd.out_data_len = 4;
	cmd.out_data = out_buf;

	in_buf[0] = ((ssv_type_u8)(port >> 0) & 0xff);
	in_buf[1] = ((ssv_type_u8)(port >> 8) & 0xff);
	in_buf[2] = ((ssv_type_u8)(port >> 16) & 0xff);
	in_buf[3] = ((ssv_type_u8)(port >> 24) & 0xff);
	OS_MutexLock(s_SDIOMutex);
	status = ioctl(s_hSDIO,IOCTL_SSV6XXX_SDIO_SET_MULTI_BYTE_IO_PORT,&cmd);
	OS_MutexUnLock(s_SDIOMutex);
	if (status < 0)
	{
		SDIO_ERROR("SET_MULTI_BYTE_IO_PORT\n");
		return false;
	}

	SDIO_TRACE("%-20s : %d (0x%08x)\n", "SET_MULTI_BYTE_IO_PORT", port, port);
	return true;
}

ssv_type_bool	sdio_get_multi_byte_reg_io_port(ssv_type_u32 *port)
{
	ssv_type_u8	buf[3];

	SDIO_CHECK_OPENED();

	*port = 0;
	//memset(buf, 0x00, 3);
	buf[0] = sdio_read_byte(1,REG_REG_IO_PORT_0);
	buf[1] = sdio_read_byte(1,REG_REG_IO_PORT_1);
	buf[2] = sdio_read_byte(1,REG_REG_IO_PORT_2);
	*port = (((buf[0] & 0xff) << 0) |
			 ((buf[1] & 0xff) << 8) |
			 ((buf[2] & 0x01) << 16));
	SDIO_TRACE("%-20s : %d (0x%08x)\n", "GET_MULTI_BYTE_REG_IO_PORT", *port, *port);
	return true;
}

ssv_type_bool	sdio_set_multi_byte_reg_io_port(ssv_type_u32 port)
{
	struct ssv6xxx_sdiobridge_cmd cmd;
	ssv_type_u8		in_buf[4], out_buf[4];
	int	status;

	SDIO_CHECK_OPENED();

	cmd.in_data_len = 4;
	cmd.in_data = in_buf;
	cmd.out_data_len = 4;
	cmd.out_data = out_buf;

	in_buf[0] = ((ssv_type_u8)(port >> 0) & 0xff);
	in_buf[1] = ((ssv_type_u8)(port >> 8) & 0xff);
	in_buf[2] = ((ssv_type_u8)(port >> 16) & 0xff);
	in_buf[3] = ((ssv_type_u8)(port >> 24) & 0xff);
	OS_MutexLock(s_SDIOMutex);
	status = ioctl(s_hSDIO,IOCTL_SSV6XXX_SDIO_SET_MULTI_BYTE_REG_IO_PORT,&cmd);
	OS_MutexUnLock(s_SDIOMutex);
	if (status < 0 )
	{
		SDIO_ERROR("SET_MULTI_BYTE_REG_IO_PORT\n");
		return false;
	}

	SDIO_TRACE("%-20s : %d (0x%08x)\n", "SET_MULTI_BYTE_REG_IO_PORT", port, port);
	return true;
}

ssv_type_bool	sdio_open(void)
{
	SDIO_TRACE("%-20s%s : 0x%08x\n", __FUNCTION__, " <= ", s_hSDIO);

	if (s_hSDIO != 0)
	{
		SDIO_WARN("Try to open a already opened SDIO device!\n");
		return false;
	}

	s_hSDIO = open(SDIO_DEVICE_NAME, O_RDWR);
	if (s_hSDIO < 0)
	{
		s_hSDIO = 0;
		SDIO_FAIL_RET(0, "open(), error\n");
	}

	SDIO_TRACE("%-20s%s : 0x%08x\n", __FUNCTION__, " => ", s_hSDIO);
	return (s_hSDIO != 0);
}

ssv_type_bool	sdio_close(void)
{
	SDIO_CHECK_OPENED();

	if (close(s_hSDIO))
	{
		s_hSDIO = 0;
		SDIO_TRACE("%-20s :\n", __FUNCTION__);
		return true;
	}

	SDIO_WARN("sdio_close(0x%08x) fail!\n", s_hSDIO);
	return false;
}

ssv_type_bool	sdio_init(void)
{
	ssv_type_s32		current_sdio_clock_rate = 0;
//	u32		n32;
	ssv_type_u32		port_data, port_reg;
	ssv_type_u8      u8data;

	SDIO_CHECK_OPENED();

	if (s_hSDIO == 0)
	{
		SDIO_ERROR("SDIO is NOT opened!\n", s_hSDIO);
		return false;
	}
	// mutex init
	OS_MutexInit(&s_SDIOMutex);
	LOG_PRINTF("<sdio init> : init sdio mutex ...\n");

	// set clock
	//LOG_PRINTF("<sdio init> : current sdio clock rate 1 : %d\n", sdio_get_bus_clock());
	//if (!sdio_set_bus_clock(SDIO_DEF_CLOCK_RATE))
	//	SDIO_FAIL_RET(0, "sdio_set_bus_clock(%d)\n", SDIO_DEF_CLOCK_RATE);
	//LOG_PRINTF("<sdio init> : set bus clock (%d) -> after (%d)\n", SDIO_DEF_CLOCK_RATE, sdio_get_bus_clock());

	// set off block mode
	//if (!sdio_set_block_mode(false))
	//	SDIO_FAIL_RET(0, "sdio_set_block_mode(false)\n");
	//LOG_PRINTF("<sdio init> : set off block mode\n");

	// set block size
	if (!sdio_set_block_size(SDIO_DEF_BLOCK_SIZE))
		SDIO_FAIL_RET(0, "sdio_set_block_size(%d)\n", SDIO_DEF_BLOCK_SIZE);
	LOG_PRINTF("<sdio init> : set block size to %d\n", SDIO_DEF_BLOCK_SIZE);

//	// get dataIOPort
//	if (!sdio_get_multi_byte_io_port(&port_data))
//		SDIO_FAIL_RET(0, "sdio_get_multi_byte_io_port()\n");
//	LOG_PRINTF("<sdio init> : get data i/o port %d (0x%08x)\n", port_data, port_data);

	// get regIOPort
//	if (!sdio_get_multi_byte_reg_io_port(&port_reg))
//		SDIO_FAIL_RET(0, "sdio_get_multi_byte_reg_io_port()\n");
//	LOG_PRINTF("<sdio init> : get  reg i/o port %d (0x%08x)\n", port_reg, port_reg);

//	// set dataIOPort
//	if (!sdio_set_multi_byte_io_port(port_data))
//		SDIO_FAIL_RET(0, "sdio_set_multi_byte_io_port(%d)\n", port_data);
//	LOG_PRINTF("<sdio init> : set data i/o port %d (0x%08x)\n", port_data, port_data);

//	// set regIOPort
//	if (!sdio_set_multi_byte_reg_io_port(port_reg))
//		SDIO_FAIL_RET(0, "sdio_set_multi_byte_reg_io_port(%d)\n", port_reg);
//	LOG_PRINTF("<sdio init> : set reg  i/o port %d (0x%08x)\n", port_reg, port_reg);

	// mask rx/tx complete int
	// 0: rx int
	// 1: tx complete int
#if (CONFIG_RX_POLL == 0)
	if (!sdio_write_byte(1,0x04, 0x02))
		SDIO_FAIL_RET(0, "sdio_write_byte(0x04, 0x02)\n");
	LOG_PRINTF("<sdio init> : mask rx/tx complete int (0x04, 0x02)\n");
#else
	if (!sdio_write_byte(1,0x04, 0x02))
		SDIO_FAIL_RET(0, "sdio_write_byte(0x04, 0x03)\n");
	LOG_PRINTF("<sdio init> : mask rx/tx complete int (0x04, 0x03)\n");
#endif

	// output timing
	if (!sdio_write_byte(1,0x55, SDIO_DEF_OUTPUT_TIMING))
		SDIO_FAIL_RET(0, "sdio_write_byte(0x55, %d)\n", SDIO_DEF_OUTPUT_TIMING);
	LOG_PRINTF("<sdio init> : output timing to %d (0x%08x)\n", SDIO_DEF_OUTPUT_TIMING, SDIO_DEF_OUTPUT_TIMING);
	// switch to normal mode
	// bit[1] , 0:normal mode, 1: Download mode
	if (!sdio_write_byte(0,0x0c, 0x00))
		SDIO_FAIL_RET(0, "sdio_write_byte(0x0c, 0x00)\n");
	LOG_PRINTF("<sdio init> : switch to normal mode (0x0c, 0x00)\n");

	//to check if support tx alloc mechanism
	//sdio_write_byte(1,REG_SDIO_TX_ALLOC_SHIFT,SDIO_TX_ALLOC_SIZE_SHIFT|SDIO_TX_ALLOC_ENABLE);

	LOG_PRINTF("<sdio init> : success!! \n");
	return true;
}


ssv_type_bool sdio_ack_int(void)
{
    return false;
} // end of - sdio_ack_int -


ssv_type_bool sdio_read_sram(ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size)
{
    return false;
} // end of - sdio_read_sram -

ssv_type_bool sdio_write_sram(ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size)
{
	//u16   bsize = sdio_get_cur_block_size();
    //sdio_set_block_size(512);

    #if 0
    {
    u32 i;
    size = (size + (sizeof(u32) - 1)) / sizeof(u32);
    for (i = 0; i< size; i++, sram_addr += sizeof(u32))
    {
        sdio_write_reg(u32 addr,u32 data)(sram_addr, buffer[i]);
	}
    #endif
	// Set SDIO DMA start address
	sdio_write_reg(0xc0000860, addr);

    sdio_set_data_mode(false);

	sdio_write_data((void *)data, size);

    sdio_set_data_mode(true);

    //sdio_set_block_size(bsize);

    return true;
}// end of - sdio_write_sram -


