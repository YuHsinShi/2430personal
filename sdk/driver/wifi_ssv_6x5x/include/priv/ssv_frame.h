/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SSV_FRAME_H_
#define _SSV_FRAME_H_

//-----------------------------------------------------------------------------------------------------------------------------------------------
//																frame buffer related
//-----------------------------------------------------------------------------------------------------------------------------------------------

#define os_frame_free(ptr) if((ptr)==NULL){ \
        LOG_PRINTF("Error %s @line %d os_frame_free null pointer\r\n",__func__,__LINE__); \
    }else{ \
        _os_frame_free((ptr)); \
    }

void* os_frame_alloc_fn(ssv_type_u32 size, ssv_type_bool SecPool, const char* file, const int line);
#define os_frame_alloc(_size,SecPool) os_frame_alloc_fn(_size,SecPool, __FILE__, __LINE__)

void _os_frame_free(void *frame);
void* os_frame_dup(void *frame);


//increase space to set header
void* os_frame_push(void *frame, ssv_type_u32 len);

//decrease data space.
void* os_frame_pull(void *frame, ssv_type_u32 size);
ssv_type_u8* os_frame_get_data_addr(void *_frame);
ssv_type_u32 os_frame_get_data_len(void *_frame);
void os_frame_set_data_len(void *_frame, ssv_type_u32 _len);

void os_frame_set_debug_flag(void *frame, ssv_type_u32 flag);

#define OS_FRAME_GET_DATA(_frame)               os_frame_get_data_addr(_frame)
#define OS_FRAME_GET_DATA_LEN(_frame)           os_frame_get_data_len(_frame)
#define OS_FRAME_SET_DATA_LEN(_frame, _len)     os_frame_set_data_len(_frame, _len)


#endif /* _SSV_FRAME_H_ */

