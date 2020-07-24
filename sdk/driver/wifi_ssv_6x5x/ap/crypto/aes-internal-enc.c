/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/

#include <ssv_types.h>
#include "rtos.h"
#include "log.h"
#include "../common/wpa_common.h"
#include "crypto.h"
#include "aes_i.h"

#if (AP_MODE_ENABLE == 1)

#ifndef CONFIG_NO_WPA2
void rijndaelEncrypt(const ssv_type_u32 rk[/*44*/], const ssv_type_u8 pt[16], ssv_type_u8 ct[16])
{
	ssv_type_u32 s0, s1, s2, s3, t0, t1, t2, t3;
	const int Nr = 10;
#ifndef FULL_UNROLL
	int r;
#endif /* ?FULL_UNROLL */

	/*
	 * map byte array block to cipher state
	 * and add initial round key:
	 */
	s0 = GETU32(pt     ) ^ rk[0];
	s1 = GETU32(pt +  4) ^ rk[1];
	s2 = GETU32(pt +  8) ^ rk[2];
	s3 = GETU32(pt + 12) ^ rk[3];

#define ROUND(i,d,s) \
d##0 = TE0(s##0) ^ TE1(s##1) ^ TE2(s##2) ^ TE3(s##3) ^ rk[4 * i]; \
d##1 = TE0(s##1) ^ TE1(s##2) ^ TE2(s##3) ^ TE3(s##0) ^ rk[4 * i + 1]; \
d##2 = TE0(s##2) ^ TE1(s##3) ^ TE2(s##0) ^ TE3(s##1) ^ rk[4 * i + 2]; \
d##3 = TE0(s##3) ^ TE1(s##0) ^ TE2(s##1) ^ TE3(s##2) ^ rk[4 * i + 3]

#ifdef FULL_UNROLL

	ROUND(1,t,s);
	ROUND(2,s,t);
	ROUND(3,t,s);
	ROUND(4,s,t);
	ROUND(5,t,s);
	ROUND(6,s,t);
	ROUND(7,t,s);
	ROUND(8,s,t);
	ROUND(9,t,s);

	rk += Nr << 2;

#else  /* !FULL_UNROLL */

	/* Nr - 1 full rounds: */
	r = Nr >> 1;
	for (;;) {
		ROUND(1,t,s);
		rk += 8;
		if (--r == 0)
			break;
		ROUND(0,s,t);
	}

#endif /* ?FULL_UNROLL */

#undef ROUND

	/*
	 * apply last round and
	 * map cipher state to byte array block:
	 */
	s0 = TE41(t0) ^ TE42(t1) ^ TE43(t2) ^ TE44(t3) ^ rk[0];
	PUTU32(ct     , s0);
	s1 = TE41(t1) ^ TE42(t2) ^ TE43(t3) ^ TE44(t0) ^ rk[1];
	PUTU32(ct +  4, s1);
	s2 = TE41(t2) ^ TE42(t3) ^ TE43(t0) ^ TE44(t1) ^ rk[2];
	PUTU32(ct +  8, s2);
	s3 = TE41(t3) ^ TE42(t0) ^ TE43(t1) ^ TE44(t2) ^ rk[3];
	PUTU32(ct + 12, s3);
}


void * aes_encrypt_init(const ssv_type_u8 *key, ssv_type_size_t len)
{
	ssv_type_u32 *rk;
	if (len != 16)
		return NULL;
	rk = (ssv_type_u32 *)os_malloc(AES_PRIV_SIZE);
	if (rk == NULL)
		return NULL;
	rijndaelKeySetupEnc(rk, key);
	return rk;
}


void aes_encrypt(void *ctx, const ssv_type_u8 *plain, ssv_type_u8 *crypt)
{
	rijndaelEncrypt((const ssv_type_u32 *)ctx, plain, crypt);
}


void aes_encrypt_deinit(void *ctx)
{
	os_memset(ctx, 0, AES_PRIV_SIZE);
	os_free(ctx);
}
#endif
#endif
