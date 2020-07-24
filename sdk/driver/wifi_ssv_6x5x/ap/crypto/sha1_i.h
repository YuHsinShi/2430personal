/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef SHA1_I_H
#define SHA1_I_H

struct SHA1Context {
	ssv_type_u32 state[5];
	ssv_type_u32 count[2];
	unsigned char buffer[64];
};

void SHA1Init(struct SHA1Context *context);
void SHA1Update(struct SHA1Context *context, const void *data, ssv_type_u32 len);
void SHA1Final(unsigned char digest[20], struct SHA1Context *context);
#endif /* SHA1_I_H */
