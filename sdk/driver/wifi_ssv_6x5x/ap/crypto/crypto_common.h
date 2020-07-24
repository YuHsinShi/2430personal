/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef CRYPTO_COMMON_H
#define CRYPTO_COMMON_H
#ifndef __must_check

#if ((defined(__GNUC__)) && (defined(__GNUC_MINOR__)) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)))
#define __must_check __attribute__((__warn_unused_result__))
#else
#define __must_check
#endif /* __GNUC__ */
#endif /* __must_check */

#endif