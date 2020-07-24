/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include "rtos.h"
#include <ssv_lib.h>

#include "../common/wpa_common.h"
#include "sha1.h"
#include "crypto.h"
#include "log.h"
#include "wpa_debug.h"

#if (AP_MODE_ENABLE == 1)

#ifndef CONFIG_NO_WPA2
/* the HMAC_SHA1 transform looks like:
 *
 * SHA1(K XOR opad, SHA1(K XOR ipad, text))
 *
 * where K is an n byte key
 * ipad is the byte 0x36 repeated 64 times
 * opad is the byte 0x5c repeated 64 times
 * and text is the data being protected.
 * prepare_hmac_sha1_vector generates ipad and opad which can be 
 * reused if HMAC-SHA1 is repeated with the same key.
 * exec_hmac_sha1_vector execute the SHA1 formula.
 */

// Generate k_pad, which is the first data block for sha1_vector from key and the xor pattern.
static void gen_k_pad (const ssv_type_u8 *key, ssv_type_u32 key_len, ssv_type_u8 *k_pad, ssv_type_u32 pattern)
{
    ssv_type_u32 *k_pad_u32 = (ssv_type_u32 *)k_pad;
    const ssv_type_u32 *key_u32 = (const ssv_type_u32 *)key;
    ssv_type_u32 i;
    ssv_type_u32 word_cnt = key_len / 4;
    ssv_type_u8 pattern_u8 = (ssv_type_u8)pattern;
    /* start out by storing key in ipad */
    //os_memset(k_pad, 0, 64);
    for (i = word_cnt; i < 16; i++)
        k_pad_u32[i] = pattern;
    if (((ssv_type_u32)key % 4))
        i = 0;
    else 
    {
        for (i = 0; i < word_cnt; i++)
           k_pad_u32[i] = pattern ^ key_u32[i];
        i = word_cnt * 4;
    }
    for (; i < key_len; i++)
        k_pad[i] = key[i] ^ pattern_u8;
}
/**
 * prepare_hmac_sha1_vector - prepare k_pad(s) for HMAC-SHA1 calculation.
 * @key: Key for HMAC operations
 * @key_len: Length of the key in bytes
 * @k_pad: result of the k_pad(s) for HMAC-SHA1 process.
  * Returns: 0 on success, -1 on failure
 */
int prepare_hmac_sha1_vector(const ssv_type_u8 *key, ssv_type_size_t key_len, ssv_type_u8 (*k_pad[2])[64])
{
    unsigned char tk[20];

    /* if key is longer than 64 bytes reset it to key = SHA1(key) */
    if (key_len > 64) {
        if (sha1_vector(1, &key, &key_len, tk))
            return -1;
        key = tk;
        key_len = 20;
    }

    gen_k_pad(key, key_len, *k_pad[0], 0x36363636);

    gen_k_pad(key, key_len, *k_pad[1], 0x5c5c5c5c);

    return 0;
}

/**
 * exec_hmac_sha1_vector - execute HMAC-SHA1 over data vector (RFC 2104)
 * @k_pad: k_pads from prepare_hmac_sha1_vector
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for the hash (20 bytes)
 * Returns: 0 on success, -1 on failure
 */
int exec_hmac_sha1_vector(ssv_type_u8 (*k_pad[2])[64], ssv_type_size_t num_elem,
                          const ssv_type_u8 *addr[], const ssv_type_size_t *len, ssv_type_u8 *mac)
{
    const ssv_type_u8 *_addr[6];
    ssv_type_size_t _len[6], i;
    int ret;

    /* perform inner SHA1 */
    _addr[0] = *k_pad[0];
    _len[0] = 64;
    for (i = 0; i < num_elem; i++) {
        _addr[i + 1] = addr[i];
        _len[i + 1] = len[i];
    }
    if (sha1_vector(1 + num_elem, _addr, _len, mac))
        return -1;

    /* perform outer SHA1 */
    _addr[0] = *k_pad[1];
    _len[0] = 64;
    _addr[1] = mac;
    _len[1] = SHA1_MAC_LEN;
    ret = sha1_vector(2, _addr, _len, mac);
    return ret;
}


/**
 * hmac_sha1_vector - HMAC-SHA1 over data vector (RFC 2104)
 * @key: Key for HMAC operations
 * @key_len: Length of the key in bytes
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for the hash (20 bytes)
 * Returns: 0 on success, -1 on failure
 */
int hmac_sha1_vector(const ssv_type_u8 *key, ssv_type_size_t key_len, ssv_type_size_t num_elem,
                     const ssv_type_u8 *addr[], const ssv_type_size_t *len, ssv_type_u8 *mac)
{
    static ssv_type_u8 k_pad_buf[2][64]; /* padding - key XORd with ipad/opad */
    ssv_type_u8 (*k_pad[2])[64] = {&k_pad_buf[0], &k_pad_buf[1]};
    
    if (key)
        prepare_hmac_sha1_vector(key, key_len, k_pad);

    return exec_hmac_sha1_vector(k_pad, num_elem, addr, len, mac);
}


/**
 * hmac_sha1 - HMAC-SHA1 over data buffer (RFC 2104)
 * @key: Key for HMAC operations
 * @key_len: Length of the key in bytes
 * @data: Pointers to the data area
 * @data_len: Length of the data area
 * @mac: Buffer for the hash (20 bytes)
 * Returns: 0 on success, -1 of failure
 */
int hmac_sha1(const ssv_type_u8 *key, ssv_type_size_t key_len, const ssv_type_u8 *data, ssv_type_size_t data_len,
              ssv_type_u8 *mac)
{
    return hmac_sha1_vector(key, key_len, 1, &data, &data_len, mac);
}


/**
 * sha1_prf - SHA1-based Pseudo-Random Function (PRF) (IEEE 802.11i, 8.5.1.1)
 * @key: Key for PRF
 * @key_len: Length of the key in bytes
 * @label: A unique label for each purpose of the PRF
 * @data: Extra data to bind into the key
 * @data_len: Length of the data
 * @buf: Buffer for the generated pseudo-random key
 * @buf_len: Number of bytes of key to generate
 * Returns: 0 on success, -1 of failure
 *
 * This function is used to derive new, cryptographically separate keys from a
 * given key (e.g., PMK in IEEE 802.11i).
 */
int sha1_prf(const ssv_type_u8 *key, ssv_type_size_t key_len, const char *label,
             const ssv_type_u8 *data, ssv_type_size_t data_len, ssv_type_u8 *buf, ssv_type_size_t buf_len)
{
    ssv_type_u8 counter = 0;
    ssv_type_size_t pos, plen;
    ssv_type_u8 hash[SHA1_MAC_LEN];
    ssv_type_size_t label_len = os_strlen(label) + 1;
    const unsigned char *addr[3];
    ssv_type_size_t len[3];

    addr[0] = (const ssv_type_u8 *) label;
    len[0] = label_len;
    addr[1] = data;
    len[1] = data_len;
    addr[2] = &counter;
    len[2] = 1;
    wpa_hexdump_key(MSG_DEBUG, "addr",
			addr, 3);
    wpa_hexdump_key(MSG_DEBUG, "len",
			len, 3);
    pos = 0;
    while (pos < buf_len) {
        plen = buf_len - pos;
        if (plen >= SHA1_MAC_LEN) {
            if (hmac_sha1_vector(key, key_len, 3, addr, len,
                         &buf[pos]))                         
            {
                LOG_DEBUG("%s:%d",__FUNCTION__,__LINE__);
                return -1;
            }
            pos += SHA1_MAC_LEN;
        } else {
            if (hmac_sha1_vector(key, key_len, 3, addr, len,
                         hash))
            {
                LOG_DEBUG("%s:%d",__FUNCTION__,__LINE__);
                return -1;
            }
            os_memcpy(&buf[pos], hash, plen);
            break;
        }
        counter++;
    }

    return 0;
}


typedef struct __SHA1_LOOP_DATA_S {
    ssv_type_u8      k_pad[64];      // 64B k_pad
    ssv_type_u8      mac[20];        // 20B MAC result
    ssv_type_u8      padding[36];    // 36B padding
    ssv_type_u8      finalcount[8];  // 8B finalcount
} SHA1_LOOP_DATA_S;

typedef union __SHA1_LOOP_DATA {
    SHA1_LOOP_DATA_S    loop_data;
    ssv_type_u8                  block[2][64];
    ssv_type_u32                 data32[32]; // For word data processing.
} SHA1_LOOP_BUF;

typedef struct __SHA1_INTERNAL_DATA_S {
    HMAC_SHA1_LOOP_DATA_S   out_data;
    ssv_type_u32                     result_idx;
    ssv_type_u32                     source_idx;
    SHA1_LOOP_BUF           loop_buf[2];
} SHA1_INTERNAL_DATA_S;
// 12 + 4 + 4 + 256 = 276bytes
#endif
#endif

