﻿#ifndef __DCTIITAB_H__
#define __DCTIITAB_H__

/* format: S1.30
 * MAX dctii_32_costab[0][0]=1.000000,  0x40000000
 * MIN dctii_32_costab[0][190]=-0.980785,   0xc13ad061
 * different entries: 15
 *
 * MAX dctii_32_costab[1][152]=0.998795,    0x3fec43c6
 * MIN dctii_32_costab[1][250]=-0.998795,   0xc013bc3a
 * different entries: 32
 *
 * MAX dctii_32_costab[2][247]=0.995185,    0x3fb11b47
 * MIN dctii_32_costab[2][210]=-0.995185,   0xc04ee4b9
 * different entries: 16
 *
 * MAX dctii_32_costab[3][239]=0.998795,    0x3fec43c6
 * MIN dctii_32_costab[3][179]=-0.998795,   0xc013bc3a
 * different entries: 32
 */

static const int dctii_32_costab[4][256]={
{
0x40000000, 0x40000000, 0x40000000, 0x40000000, 0x40000000, 0x40000000,
0x40000000, 0x40000000, 0x40000000, 0x40000000, 0x40000000, 0x40000000,
0x40000000, 0x40000000, 0x40000000, 0x40000000, 0x40000000, 0x40000000,
0x40000000, 0x40000000, 0x40000000, 0x40000000, 0x40000000, 0x40000000,
0x40000000, 0x40000000, 0x40000000, 0x40000000, 0x40000000, 0x40000000,
0x40000000, 0x40000000, 0x3ec52f9f, 0x3536cc52, 0x238e7673, 0x0c7c5c1e,
0xf383a3e2, 0xdc71898d, 0xcac933ae, 0xc13ad061, 0xc13ad061, 0xcac933ae,
0xdc71898d, 0xf383a3e2, 0x0c7c5c1e, 0x238e7673, 0x3536cc52, 0x3ec52f9f,
0x3ec52f9f, 0x3536cc52, 0x238e7673, 0x0c7c5c1e, 0xf383a3e2, 0xdc71898d,
0xcac933ae, 0xc13ad061, 0xc13ad061, 0xcac933ae, 0xdc71898d, 0xf383a3e2,
0x0c7c5c1e, 0x238e7673, 0x3536cc52, 0x3ec52f9f, 0x3b20d79e, 0x187de2a6,
0xe7821d5a, 0xc4df2862, 0xc4df2862, 0xe7821d5a, 0x187de2a6, 0x3b20d79e,
0x3b20d79e, 0x187de2a6, 0xe7821d5a, 0xc4df2862, 0xc4df2862, 0xe7821d5a,
0x187de2a6, 0x3b20d79e, 0x3b20d79e, 0x187de2a6, 0xe7821d5a, 0xc4df2862,
0xc4df2862, 0xe7821d5a, 0x187de2a6, 0x3b20d79e, 0x3b20d79e, 0x187de2a6,
0xe7821d5a, 0xc4df2862, 0xc4df2862, 0xe7821d5a, 0x187de2a6, 0x3b20d79e,
0x3536cc52, 0xf383a3e2, 0xc13ad061, 0xdc71898d, 0x238e7673, 0x3ec52f9f,
0x0c7c5c1e, 0xcac933ae, 0xcac933ae, 0x0c7c5c1e, 0x3ec52f9f, 0x238e7673,
0xdc71898d, 0xc13ad061, 0xf383a3e2, 0x3536cc52, 0x3536cc52, 0xf383a3e2,
0xc13ad061, 0xdc71898d, 0x238e7673, 0x3ec52f9f, 0x0c7c5c1e, 0xcac933ae,
0xcac933ae, 0x0c7c5c1e, 0x3ec52f9f, 0x238e7673, 0xdc71898d, 0xc13ad061,
0xf383a3e2, 0x3536cc52, 0x2d413ccc, 0xd2bec334, 0xd2bec334, 0x2d413ccc,
0x2d413ccc, 0xd2bec334, 0xd2bec334, 0x2d413ccc, 0x2d413ccc, 0xd2bec334,
0xd2bec334, 0x2d413ccc, 0x2d413ccc, 0xd2bec334, 0xd2bec334, 0x2d413ccc,
0x2d413ccc, 0xd2bec334, 0xd2bec334, 0x2d413ccc, 0x2d413ccc, 0xd2bec334,
0xd2bec334, 0x2d413ccc, 0x2d413ccc, 0xd2bec334, 0xd2bec334, 0x2d413ccc,
0x2d413ccc, 0xd2bec334, 0xd2bec334, 0x2d413ccc, 0x238e7673, 0xc13ad061,
0x0c7c5c1e, 0x3536cc52, 0xcac933ae, 0xf383a3e2, 0x3ec52f9f, 0xdc71898d,
0xdc71898d, 0x3ec52f9f, 0xf383a3e2, 0xcac933ae, 0x3536cc52, 0x0c7c5c1e,
0xc13ad061, 0x238e7673, 0x238e7673, 0xc13ad061, 0x0c7c5c1e, 0x3536cc52,
0xcac933ae, 0xf383a3e2, 0x3ec52f9f, 0xdc71898d, 0xdc71898d, 0x3ec52f9f,
0xf383a3e2, 0xcac933ae, 0x3536cc52, 0x0c7c5c1e, 0xc13ad061, 0x238e7673,
0x187de2a6, 0xc4df2862, 0x3b20d79e, 0xe7821d5a, 0xe7821d5a, 0x3b20d79e,
0xc4df2862, 0x187de2a6, 0x187de2a6, 0xc4df2862, 0x3b20d79e, 0xe7821d5a,
0xe7821d5a, 0x3b20d79e, 0xc4df2862, 0x187de2a6, 0x187de2a6, 0xc4df2862,
0x3b20d79e, 0xe7821d5a, 0xe7821d5a, 0x3b20d79e, 0xc4df2862, 0x187de2a6,
0x187de2a6, 0xc4df2862, 0x3b20d79e, 0xe7821d5a, 0xe7821d5a, 0x3b20d79e,
0xc4df2862, 0x187de2a6, 0x0c7c5c1e, 0xdc71898d, 0x3536cc52, 0xc13ad061,
0x3ec52f9f, 0xcac933ae, 0x238e7673, 0xf383a3e2, 0xf383a3e2, 0x238e7673,
0xcac933ae, 0x3ec52f9f, 0xc13ad061, 0x3536cc52, 0xdc71898d, 0x0c7c5c1e,
0x0c7c5c1e, 0xdc71898d, 0x3536cc52, 0xc13ad061, 0x3ec52f9f, 0xcac933ae,
0x238e7673, 0xf383a3e2, 0xf383a3e2, 0x238e7673, 0xcac933ae, 0x3ec52f9f,
0xc13ad061, 0x3536cc52, 0xdc71898d, 0x0c7c5c1e, },
{
0xc013bc3a, 0xc0b15502, 0xc1eb0209, 0xc3bdbdf7, 0xc6250a18, 0xc91af976,
0xcc983f71, 0xd09441bc, 0xd5052d97, 0xd9e01007, 0xdf18f0ce, 0xe4a2eff7,
0xea70658b, 0xf0730343, 0xf69bf7c9, 0xfcdc1342, 0x0323ecbe, 0x09640837,
0x0f8cfcbd, 0x158f9a75, 0x1b5d1009, 0x20e70f32, 0x261feff9, 0x2afad269,
0x2f6bbe44, 0x3367c08f, 0x36e5068a, 0x39daf5e8, 0x3c424209, 0x3e14fdf7,
0x3f4eaafe, 0x3fec43c6, 0xc1eb0209, 0xd09441bc, 0xea70658b, 0x09640837,
0x261feff9, 0x39daf5e8, 0x3fec43c6, 0x36e5068a, 0x20e70f32, 0x0323ecbe,
0xe4a2eff7, 0xcc983f71, 0xc0b15502, 0xc3bdbdf7, 0xd5052d97, 0xf0730343,
0x0f8cfcbd, 0x2afad269, 0x3c424209, 0x3f4eaafe, 0x3367c08f, 0x1b5d1009,
0xfcdc1342, 0xdf18f0ce, 0xc91af976, 0xc013bc3a, 0xc6250a18, 0xd9e01007,
0xf69bf7c9, 0x158f9a75, 0x2f6bbe44, 0x3e14fdf7, 0xc6250a18, 0xf0730343,
0x261feff9, 0x3fec43c6, 0x2afad269, 0xf69bf7c9, 0xc91af976, 0xc3bdbdf7,
0xea70658b, 0x20e70f32, 0x3f4eaafe, 0x2f6bbe44, 0xfcdc1342, 0xcc983f71,
0xc1eb0209, 0xe4a2eff7, 0x1b5d1009, 0x3e14fdf7, 0x3367c08f, 0x0323ecbe,
0xd09441bc, 0xc0b15502, 0xdf18f0ce, 0x158f9a75, 0x3c424209, 0x36e5068a,
0x09640837, 0xd5052d97, 0xc013bc3a, 0xd9e01007, 0x0f8cfcbd, 0x39daf5e8,
0xcc983f71, 0x158f9a75, 0x3fec43c6, 0x0f8cfcbd, 0xc91af976, 0xd09441bc,
0x1b5d1009, 0x3f4eaafe, 0x09640837, 0xc6250a18, 0xd5052d97, 0x20e70f32,
0x3e14fdf7, 0x0323ecbe, 0xc3bdbdf7, 0xd9e01007, 0x261feff9, 0x3c424209,
0xfcdc1342, 0xc1eb0209, 0xdf18f0ce, 0x2afad269, 0x39daf5e8, 0xf69bf7c9,
0xc0b15502, 0xe4a2eff7, 0x2f6bbe44, 0x36e5068a, 0xf0730343, 0xc013bc3a,
0xea70658b, 0x3367c08f, 0xd5052d97, 0x3367c08f, 0x20e70f32, 0xc6250a18,
0xea70658b, 0x3e14fdf7, 0x09640837, 0xc013bc3a, 0x0323ecbe, 0x3f4eaafe,
0xf0730343, 0xc3bdbdf7, 0x1b5d1009, 0x36e5068a, 0xd9e01007, 0xd09441bc,
0x2f6bbe44, 0x261feff9, 0xc91af976, 0xe4a2eff7, 0x3c424209, 0x0f8cfcbd,
0xc0b15502, 0xfcdc1342, 0x3fec43c6, 0xf69bf7c9, 0xc1eb0209, 0x158f9a75,
0x39daf5e8, 0xdf18f0ce, 0xcc983f71, 0x2afad269, 0xdf18f0ce, 0x3fec43c6,
0xe4a2eff7, 0xd9e01007, 0x3f4eaafe, 0xea70658b, 0xd5052d97, 0x3e14fdf7,
0xf0730343, 0xd09441bc, 0x3c424209, 0xf69bf7c9, 0xcc983f71, 0x39daf5e8,
0xfcdc1342, 0xc91af976, 0x36e5068a, 0x0323ecbe, 0xc6250a18, 0x3367c08f,
0x09640837, 0xc3bdbdf7, 0x2f6bbe44, 0x0f8cfcbd, 0xc1eb0209, 0x2afad269,
0x158f9a75, 0xc0b15502, 0x261feff9, 0x1b5d1009, 0xc013bc3a, 0x20e70f32,
0xea70658b, 0x36e5068a, 0xc0b15502, 0x2afad269, 0xfcdc1342, 0xd9e01007,
0x3e14fdf7, 0xc6250a18, 0x1b5d1009, 0x0f8cfcbd, 0xcc983f71, 0x3fec43c6,
0xd09441bc, 0x09640837, 0x20e70f32, 0xc3bdbdf7, 0x3c424209, 0xdf18f0ce,
0xf69bf7c9, 0x2f6bbe44, 0xc013bc3a, 0x3367c08f, 0xf0730343, 0xe4a2eff7,
0x39daf5e8, 0xc1eb0209, 0x261feff9, 0x0323ecbe, 0xd5052d97, 0x3f4eaafe,
0xc91af976, 0x158f9a75, 0xf69bf7c9, 0x1b5d1009, 0xd5052d97, 0x36e5068a,
0xc1eb0209, 0x3fec43c6, 0xc3bdbdf7, 0x3367c08f, 0xd9e01007, 0x158f9a75,
0xfcdc1342, 0xf0730343, 0x20e70f32, 0xd09441bc, 0x39daf5e8, 0xc0b15502,
0x3f4eaafe, 0xc6250a18, 0x2f6bbe44, 0xdf18f0ce, 0x0f8cfcbd, 0x0323ecbe,
0xea70658b, 0x261feff9, 0xcc983f71, 0x3c424209, 0xc013bc3a, 0x3e14fdf7,
0xc91af976, 0x2afad269, 0xe4a2eff7, 0x09640837, },
{
0x3fb11b47, 0x3d3e82ad, 0x387165e3, 0x317900d6, 0x2899e64a, 0x1e2b5d38,
0x1294062e, 0x0645e9af, 0xf9ba1651, 0xed6bf9d2, 0xe1d4a2c8, 0xd76619b6,
0xce86ff2a, 0xc78e9a1d, 0xc2c17d53, 0xc04ee4b9, 0xc04ee4b9, 0xc2c17d53,
0xc78e9a1d, 0xce86ff2a, 0xd76619b6, 0xe1d4a2c8, 0xed6bf9d2, 0xf9ba1651,
0x0645e9af, 0x1294062e, 0x1e2b5d38, 0x2899e64a, 0x317900d6, 0x387165e3,
0x3d3e82ad, 0x3fb11b47, 0x3d3e82ad, 0x2899e64a, 0x0645e9af, 0xe1d4a2c8,
0xc78e9a1d, 0xc04ee4b9, 0xce86ff2a, 0xed6bf9d2, 0x1294062e, 0x317900d6,
0x3fb11b47, 0x387165e3, 0x1e2b5d38, 0xf9ba1651, 0xd76619b6, 0xc2c17d53,
0xc2c17d53, 0xd76619b6, 0xf9ba1651, 0x1e2b5d38, 0x387165e3, 0x3fb11b47,
0x317900d6, 0x1294062e, 0xed6bf9d2, 0xce86ff2a, 0xc04ee4b9, 0xc78e9a1d,
0xe1d4a2c8, 0x0645e9af, 0x2899e64a, 0x3d3e82ad, 0x387165e3, 0x0645e9af,
0xce86ff2a, 0xc2c17d53, 0xed6bf9d2, 0x2899e64a, 0x3fb11b47, 0x1e2b5d38,
0xe1d4a2c8, 0xc04ee4b9, 0xd76619b6, 0x1294062e, 0x3d3e82ad, 0x317900d6,
0xf9ba1651, 0xc78e9a1d, 0xc78e9a1d, 0xf9ba1651, 0x317900d6, 0x3d3e82ad,
0x1294062e, 0xd76619b6, 0xc04ee4b9, 0xe1d4a2c8, 0x1e2b5d38, 0x3fb11b47,
0x2899e64a, 0xed6bf9d2, 0xc2c17d53, 0xce86ff2a, 0x0645e9af, 0x387165e3,
0x317900d6, 0xe1d4a2c8, 0xc2c17d53, 0x0645e9af, 0x3fb11b47, 0x1294062e,
0xc78e9a1d, 0xd76619b6, 0x2899e64a, 0x387165e3, 0xed6bf9d2, 0xc04ee4b9,
0xf9ba1651, 0x3d3e82ad, 0x1e2b5d38, 0xce86ff2a, 0xce86ff2a, 0x1e2b5d38,
0x3d3e82ad, 0xf9ba1651, 0xc04ee4b9, 0xed6bf9d2, 0x387165e3, 0x2899e64a,
0xd76619b6, 0xc78e9a1d, 0x1294062e, 0x3fb11b47, 0x0645e9af, 0xc2c17d53,
0xe1d4a2c8, 0x317900d6, 0x2899e64a, 0xc78e9a1d, 0xed6bf9d2, 0x3fb11b47,
0xf9ba1651, 0xc2c17d53, 0x1e2b5d38, 0x317900d6, 0xce86ff2a, 0xe1d4a2c8,
0x3d3e82ad, 0x0645e9af, 0xc04ee4b9, 0x1294062e, 0x387165e3, 0xd76619b6,
0xd76619b6, 0x387165e3, 0x1294062e, 0xc04ee4b9, 0x0645e9af, 0x3d3e82ad,
0xe1d4a2c8, 0xce86ff2a, 0x317900d6, 0x1e2b5d38, 0xc2c17d53, 0xf9ba1651,
0x3fb11b47, 0xed6bf9d2, 0xc78e9a1d, 0x2899e64a, 0x1e2b5d38, 0xc04ee4b9,
0x2899e64a, 0x1294062e, 0xc2c17d53, 0x317900d6, 0x0645e9af, 0xc78e9a1d,
0x387165e3, 0xf9ba1651, 0xce86ff2a, 0x3d3e82ad, 0xed6bf9d2, 0xd76619b6,
0x3fb11b47, 0xe1d4a2c8, 0xe1d4a2c8, 0x3fb11b47, 0xd76619b6, 0xed6bf9d2,
0x3d3e82ad, 0xce86ff2a, 0xf9ba1651, 0x387165e3, 0xc78e9a1d, 0x0645e9af,
0x317900d6, 0xc2c17d53, 0x1294062e, 0x2899e64a, 0xc04ee4b9, 0x1e2b5d38,
0x1294062e, 0xce86ff2a, 0x3fb11b47, 0xc78e9a1d, 0x1e2b5d38, 0x0645e9af,
0xd76619b6, 0x3d3e82ad, 0xc2c17d53, 0x2899e64a, 0xf9ba1651, 0xe1d4a2c8,
0x387165e3, 0xc04ee4b9, 0x317900d6, 0xed6bf9d2, 0xed6bf9d2, 0x317900d6,
0xc04ee4b9, 0x387165e3, 0xe1d4a2c8, 0xf9ba1651, 0x2899e64a, 0xc2c17d53,
0x3d3e82ad, 0xd76619b6, 0x0645e9af, 0x1e2b5d38, 0xc78e9a1d, 0x3fb11b47,
0xce86ff2a, 0x1294062e, 0x0645e9af, 0xed6bf9d2, 0x1e2b5d38, 0xd76619b6,
0x317900d6, 0xc78e9a1d, 0x3d3e82ad, 0xc04ee4b9, 0x3fb11b47, 0xc2c17d53,
0x387165e3, 0xce86ff2a, 0x2899e64a, 0xe1d4a2c8, 0x1294062e, 0xf9ba1651,
0xf9ba1651, 0x1294062e, 0xe1d4a2c8, 0x2899e64a, 0xce86ff2a, 0x387165e3,
0xc2c17d53, 0x3fb11b47, 0xc04ee4b9, 0x3d3e82ad, 0xc78e9a1d, 0x317900d6,
0xd76619b6, 0x1e2b5d38, 0xed6bf9d2, 0x0645e9af, },
{
0xc0b15502, 0xc6250a18, 0xd09441bc, 0xdf18f0ce, 0xf0730343, 0x0323ecbe,
0x158f9a75, 0x261feff9, 0x3367c08f, 0x3c424209, 0x3fec43c6, 0x3e14fdf7,
0x36e5068a, 0x2afad269, 0x1b5d1009, 0x09640837, 0xf69bf7c9, 0xe4a2eff7,
0xd5052d97, 0xc91af976, 0xc1eb0209, 0xc013bc3a, 0xc3bdbdf7, 0xcc983f71,
0xd9e01007, 0xea70658b, 0xfcdc1342, 0x0f8cfcbd, 0x20e70f32, 0x2f6bbe44,
0x39daf5e8, 0x3f4eaafe, 0xc3bdbdf7, 0xdf18f0ce, 0x09640837, 0x2f6bbe44,
0x3fec43c6, 0x3367c08f, 0x0f8cfcbd, 0xe4a2eff7, 0xc6250a18, 0xc1eb0209,
0xd9e01007, 0x0323ecbe, 0x2afad269, 0x3f4eaafe, 0x36e5068a, 0x158f9a75,
0xea70658b, 0xc91af976, 0xc0b15502, 0xd5052d97, 0xfcdc1342, 0x261feff9,
0x3e14fdf7, 0x39daf5e8, 0x1b5d1009, 0xf0730343, 0xcc983f71, 0xc013bc3a,
0xd09441bc, 0xf69bf7c9, 0x20e70f32, 0x3c424209, 0xc91af976, 0x0323ecbe,
0x39daf5e8, 0x3367c08f, 0xf69bf7c9, 0xc3bdbdf7, 0xd09441bc, 0x0f8cfcbd,
0x3e14fdf7, 0x2afad269, 0xea70658b, 0xc0b15502, 0xd9e01007, 0x1b5d1009,
0x3fec43c6, 0x20e70f32, 0xdf18f0ce, 0xc013bc3a, 0xe4a2eff7, 0x261feff9,
0x3f4eaafe, 0x158f9a75, 0xd5052d97, 0xc1eb0209, 0xf0730343, 0x2f6bbe44,
0x3c424209, 0x09640837, 0xcc983f71, 0xc6250a18, 0xfcdc1342, 0x36e5068a,
0xd09441bc, 0x261feff9, 0x36e5068a, 0xe4a2eff7, 0xc3bdbdf7, 0x0f8cfcbd,
0x3f4eaafe, 0xfcdc1342, 0xc013bc3a, 0xf69bf7c9, 0x3e14fdf7, 0x158f9a75,
0xc6250a18, 0xdf18f0ce, 0x3367c08f, 0x2afad269, 0xd5052d97, 0xcc983f71,
0x20e70f32, 0x39daf5e8, 0xea70658b, 0xc1eb0209, 0x09640837, 0x3fec43c6,
0x0323ecbe, 0xc0b15502, 0xf0730343, 0x3c424209, 0x1b5d1009, 0xc91af976,
0xd9e01007, 0x2f6bbe44, 0xd9e01007, 0x3c424209, 0x0323ecbe, 0xc1eb0209,
0x20e70f32, 0x2afad269, 0xc6250a18, 0xf69bf7c9, 0x3f4eaafe, 0xe4a2eff7,
0xd09441bc, 0x36e5068a, 0x0f8cfcbd, 0xc013bc3a, 0x158f9a75, 0x3367c08f,
0xcc983f71, 0xea70658b, 0x3fec43c6, 0xf0730343, 0xc91af976, 0x2f6bbe44,
0x1b5d1009, 0xc0b15502, 0x09640837, 0x39daf5e8, 0xd5052d97, 0xdf18f0ce,
0x3e14fdf7, 0xfcdc1342, 0xc3bdbdf7, 0x261feff9, 0xe4a2eff7, 0x3e14fdf7,
0xcc983f71, 0x0323ecbe, 0x2f6bbe44, 0xc0b15502, 0x20e70f32, 0x158f9a75,
0xc3bdbdf7, 0x36e5068a, 0xf69bf7c9, 0xd5052d97, 0x3fec43c6, 0xd9e01007,
0xf0730343, 0x39daf5e8, 0xc6250a18, 0x0f8cfcbd, 0x261feff9, 0xc013bc3a,
0x2afad269, 0x09640837, 0xc91af976, 0x3c424209, 0xea70658b, 0xdf18f0ce,
0x3f4eaafe, 0xd09441bc, 0xfcdc1342, 0x3367c08f, 0xc1eb0209, 0x1b5d1009,
0xf0730343, 0x2afad269, 0xc3bdbdf7, 0x3f4eaafe, 0xcc983f71, 0x1b5d1009,
0x0323ecbe, 0xdf18f0ce, 0x36e5068a, 0xc013bc3a, 0x39daf5e8, 0xd9e01007,
0x09640837, 0x158f9a75, 0xd09441bc, 0x3e14fdf7, 0xc1eb0209, 0x2f6bbe44,
0xea70658b, 0xf69bf7c9, 0x261feff9, 0xc6250a18, 0x3fec43c6, 0xc91af976,
0x20e70f32, 0xfcdc1342, 0xe4a2eff7, 0x3367c08f, 0xc0b15502, 0x3c424209,
0xd5052d97, 0x0f8cfcbd, 0xfcdc1342, 0x09640837, 0xf0730343, 0x158f9a75,
0xe4a2eff7, 0x20e70f32, 0xd9e01007, 0x2afad269, 0xd09441bc, 0x3367c08f,
0xc91af976, 0x39daf5e8, 0xc3bdbdf7, 0x3e14fdf7, 0xc0b15502, 0x3fec43c6,
0xc013bc3a, 0x3f4eaafe, 0xc1eb0209, 0x3c424209, 0xc6250a18, 0x36e5068a,
0xcc983f71, 0x2f6bbe44, 0xd5052d97, 0x261feff9, 0xdf18f0ce, 0x1b5d1009,
0xea70658b, 0x0f8cfcbd, 0xf69bf7c9, 0x0323ecbe, },
};

#endif // __DCTIITAB_H__

