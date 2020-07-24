/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef AES_WRAP_H
#define AES_WRAP_H

int __must_check aes_wrap(const ssv_type_u8 *kek, int n, const ssv_type_u8 *plain, ssv_type_u8 *cipher);
int __must_check aes_unwrap(const ssv_type_u8 *kek, int n, const ssv_type_u8 *cipher, ssv_type_u8 *plain);
int __must_check omac1_aes_128_vector(const ssv_type_u8 *key, ssv_type_size_t num_elem,
				      const ssv_type_u8 *addr[], const ssv_type_size_t *len,
				      ssv_type_u8 *mac);
int __must_check omac1_aes_128(const ssv_type_u8 *key, const ssv_type_u8 *data, ssv_type_size_t data_len,
			       ssv_type_u8 *mac);
int __must_check aes_128_encrypt_block(const ssv_type_u8 *key, const ssv_type_u8 *in, ssv_type_u8 *out);
int __must_check aes_128_ctr_encrypt(const ssv_type_u8 *key, const ssv_type_u8 *nonce,
				     ssv_type_u8 *data, ssv_type_size_t data_len);
int __must_check aes_128_eax_encrypt(const ssv_type_u8 *key,
				     const ssv_type_u8 *nonce, ssv_type_size_t nonce_len,
				     const ssv_type_u8 *hdr, ssv_type_size_t hdr_len,
				     ssv_type_u8 *data, ssv_type_size_t data_len, ssv_type_u8 *tag);
int __must_check aes_128_eax_decrypt(const ssv_type_u8 *key,
				     const ssv_type_u8 *nonce, ssv_type_size_t nonce_len,
				     const ssv_type_u8 *hdr, ssv_type_size_t hdr_len,
				     ssv_type_u8 *data, ssv_type_size_t data_len, const ssv_type_u8 *tag);
int __must_check aes_128_cbc_encrypt(const ssv_type_u8 *key, const ssv_type_u8 *iv, ssv_type_u8 *data,
				     ssv_type_size_t data_len);
int __must_check aes_128_cbc_decrypt(const ssv_type_u8 *key, const ssv_type_u8 *iv, ssv_type_u8 *data,
				     ssv_type_size_t data_len);

#endif /* AES_WRAP_H */
