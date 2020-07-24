#ifndef CODEC_EAC3DECODE_DOLBY_IMDCT_H
#define CODEC_EAC3DECODE_DOLBY_IMDCT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t  DLB_fract32;  /*!< 32-bit fractional data (Q31) */
typedef int32_t  DLB_int32;    /*!< 32-bit signed integer */
typedef int16_t  DLB_int16;    /*!< 16-bit signed integer */
typedef uint16_t DLB_uint16;   /*!< 16-bit unsigned integer */
typedef uint32_t DLB_uint32;   /*!< 32-bit unsigned integer */
typedef int16_t  DSPshort;     /*!< DSP integer */

/*! \brief Window-Overlap-Add module PCM pointer structure */
typedef struct
{
    DLB_fract32 *p_pcmbuf;  /*!< Pointer to PCM buffer */
    DSPshort    chanoffset; /*!< Channel offset */
    DSPshort    modulo;     /*!< Modulo value */
} WOAD_PCMPTRS;

short woad_decode_dol(
    const DSPshort    scalefactor,    /* input  */
    const DLB_fract32 *p_dnmix_buf,   /* input  */
    DLB_fract32       *p_delaybuf,    /* modify */
    DLB_fract32       *p_working_buf, /* modify */
    WOAD_PCMPTRS      *p_pcmptrs);

short xfmd_imdct(
    const DSPshort bswitch,                 /* input    */
    DLB_fract32    *p_fftbuf,               /* modify   */
    DLB_fract32    *p_tcbuf);               /* modify   */

#ifdef __cplusplus
}
#endif

#endif