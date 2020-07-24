/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef SHA1_H
#define SHA1_H

#define SHA1_MAC_LEN 20

int hmac_sha1_vector(const ssv_type_u8 *key, ssv_type_size_t key_len, ssv_type_size_t num_elem,
		     const ssv_type_u8 *addr[], const ssv_type_size_t *len, ssv_type_u8 *mac);
int hmac_sha1(const ssv_type_u8 *key, ssv_type_size_t key_len, const ssv_type_u8 *data, ssv_type_size_t data_len,
	       ssv_type_u8 *mac);
int sha1_prf(const ssv_type_u8 *key, ssv_type_size_t key_len, const char *label,
	     const ssv_type_u8 *data, ssv_type_size_t data_len, ssv_type_u8 *buf, ssv_type_size_t buf_len);
int sha1_t_prf(const ssv_type_u8 *key, ssv_type_size_t key_len, const char *label,
	       const ssv_type_u8 *seed, ssv_type_size_t seed_len, ssv_type_u8 *buf, ssv_type_size_t buf_len);
int tls_prf(const ssv_type_u8 *secret, ssv_type_size_t secret_len,
			 const char *label, const ssv_type_u8 *seed, ssv_type_size_t seed_len,
			 ssv_type_u8 *out, ssv_type_size_t outlen);
int pbkdf2_sha1(const char *passphrase, const char *ssid, ssv_type_size_t ssid_len,
		int iterations, ssv_type_u8 *buf, ssv_type_size_t buflen);
int pbkdf2_sha1_flatten(const char *passphrase, const char *ssid, ssv_type_size_t ssid_len,
		int iterations, ssv_type_u8 *buf, ssv_type_size_t buflen);
int exec_hmac_sha1_vector(ssv_type_u8 (*k_pad[2])[64], ssv_type_size_t num_elem,
                          const ssv_type_u8 *addr[], const ssv_type_size_t *len, ssv_type_u8 *mac);
int prepare_hmac_sha1_vector(const ssv_type_u8 *key, ssv_type_size_t key_len, ssv_type_u8 (*k_pad[2])[64]);

// For PSK's repeat loop.
typedef struct __HMAC_SHA1_LOOP_DATA {
    ssv_type_u8 *mac; // result;
} HMAC_SHA1_LOOP_DATA_S;

ssv_type_size_t get_hmac_sha1_loop_data_size (void);

#ifdef ENABLE_DYNAMIC_HMAC_SHA1_BUF
int init_hmac_sha1_loop (HMAC_SHA1_LOOP_DATA_S *hmac_sha1_loop_data, 
                         const ssv_type_u8 *pass, ssv_type_u32 passphrase_len, ssv_type_size_t num_elem,
                         const ssv_type_u8 *addr[], const ssv_type_size_t *len);
#else // ENABLE_DYNAMIC_HMAC_SHA1_BUF
HMAC_SHA1_LOOP_DATA_S *init_hmac_sha1_loop (const ssv_type_u8 *pass, ssv_type_u32 passphrase_len, 
                                            ssv_type_size_t num_elem,
                                            const ssv_type_u8 *addr[], const ssv_type_size_t *len);
#endif // ENABLE_DYNAMIC_HMAC_SHA1_BUF

int loop_hmac_sha1 (HMAC_SHA1_LOOP_DATA_S *sha1_data);
int stop_hmac_sha1_loop (HMAC_SHA1_LOOP_DATA_S *sha1_data);

#if 1 // def ENABLE_BACKGROUND_PMK_CALC
typedef struct _PMK_CALC_DATA_S {
    union {
    ssv_type_u8      pmk[SHA1_MAC_LEN*2]; // PMK requires two PBKDF2 operations.
    ssv_type_u32     pmk_u32_block[2][SHA1_MAC_LEN/4];
    }un;
    void   *pbkdf2_sha1_data;
    void   *custom_data;
    
    #ifdef PROFILE_SUPPLICANT
    Time_T  pmk_begin_time;
    #endif // PROFILE_SUPPLICANT
} PMK_CALC_DATA_S;

PMK_CALC_DATA_S *init_pbkdf2_sha1_calc (const ssv_type_u8 *passphrase, const ssv_type_u8 *ssid,
                                        ssv_type_u32 ssid_len, ssv_type_u32 iterations);
void reinit_hmac_sha1_loop (HMAC_SHA1_LOOP_DATA_S *sha1_data,  ssv_type_size_t num_elem,
                            const ssv_type_u8 *addr[], const ssv_type_size_t *len);
int pbkdf2_sha1_calc_func (PMK_CALC_DATA_S * pmk_calc_data, ssv_type_u32 iterations);
void deinit_pbkdf2_sha1_calc (PMK_CALC_DATA_S * pmk_calc_data);
#endif // ENABLE_BACKGROUND_PMK_CALC

#endif /* SHA1_H */

