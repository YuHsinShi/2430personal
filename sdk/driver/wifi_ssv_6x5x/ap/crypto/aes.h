/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef AES_H
#define AES_H

#define AES_BLOCK_SIZE 16

void * aes_encrypt_init(const ssv_type_u8 *key, ssv_type_size_t len);
void aes_encrypt(void *ctx, const ssv_type_u8 *plain, ssv_type_u8 *crypt);
void aes_encrypt_deinit(void *ctx);
void * aes_decrypt_init(const ssv_type_u8 *key, ssv_type_size_t len);
void aes_decrypt(void *ctx, const ssv_type_u8 *crypt, ssv_type_u8 *plain);
void aes_decrypt_deinit(void *ctx);

#endif /* AES_H */
