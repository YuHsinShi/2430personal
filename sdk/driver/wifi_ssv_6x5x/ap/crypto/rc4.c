/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


//#include "../supplicant_config.h"

//#include "../utils/common.h"
#include "crypto.h"
#include "host_config.h"

#if (AP_MODE_ENABLE == 1)

#define S_SWAP(a,b) do { ssv_type_u8 t = S[a]; S[a] = S[b]; S[b] = t; } while(0)

int rc4_skip(const ssv_type_u8 *key, ssv_type_size_t keylen, ssv_type_size_t skip,
	     ssv_type_u8 *data, ssv_type_size_t data_len)
{
	ssv_type_u32 i, j, k;
	ssv_type_u8 S[256], *pos;
	ssv_type_size_t kpos;

	/* Setup RC4 state */
	for (i = 0; i < 256; i++)
		S[i] = i;
	j = 0;
	kpos = 0;
	for (i = 0; i < 256; i++) {
		j = (j + S[i] + key[kpos]) & 0xff;
		kpos++;
		if (kpos >= keylen)
			kpos = 0;
		S_SWAP(i, j);
	}

	/* Skip the start of the stream */
	i = j = 0;
	for (k = 0; k < skip; k++) {
		i = (i + 1) & 0xff;
		j = (j + S[i]) & 0xff;
		S_SWAP(i, j);
	}

	/* Apply RC4 to data */
	pos = data;
	for (k = 0; k < data_len; k++) {
		i = (i + 1) & 0xff;
		j = (j + S[i]) & 0xff;
		S_SWAP(i, j);
		*pos++ ^= S[(S[i] + S[j]) & 0xff];
	}

	return 0;
}
#endif
