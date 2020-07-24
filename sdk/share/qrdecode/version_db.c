/* quirc -- QR-code recognition library
 * Copyright (C) 2010-2012 Daniel Beer <dlbeer@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "quirc_internal.h"

const struct quirc_version_info quirc_version_db[QUIRC_MAX_VERSION + 1] = {
	    {0},
	    { /* Version 1 */
		    26,
		    {0},
		    {
			    {26, 16, 1},
			    {26, 19, 1},
			    {26, 9, 1},
			    {26, 13, 1}
		    }
	    },
	    { /* Version 2 */
		    44,
		    {6, 18, 0},
		    {
			    {44, 28, 1},
			    {44, 34, 1},
			    {44, 16, 1},
			    {44, 22, 1}
		    }
	    },
	    { /* Version 3 */
		    70,
		    {6, 22, 0},
		    {
			    {70, 44, 1},
			    {70, 55, 1},
			    {35, 13, 2},
			    {35, 17, 2}
		    }
	    },
	    { /* Version 4 */
		    100,
		    {6, 26, 0},
		    {
			    {50, 32, 2},
			    {100, 80, 1},
			    {25, 9, 4},
			    {50, 24, 2}
		    }
	    },
	    { /* Version 5 */
		    134,
		    {6, 30, 0},
		    {
			    {67, 43, 2},
			    {134, 108, 1},
			    {33, 11, 2},
			    {33, 15, 2}
		    }
	    },
	    { /* Version 6 */
		    172,
		    {6, 34, 0},
		    {
			    {43, 27, 4},
			    {86, 68, 2},
			    {43, 15, 4},
			    {43, 19, 4}
		    }
	    },
	    { /* Version 7 */
		    196,
		    {6, 22, 38, 0},
		    {
			    {49, 31, 4},
			    {98, 78, 2},
			    {39, 13, 4},
			    {32, 14, 2}
		    }
	    },
	    { /* Version 8 */
		    242,
		    {6, 24, 42, 0},
		    {
			    {60, 38, 2},
			    {121, 97, 2},
			    {40, 14, 4},
			    {40, 18, 4}
		    }
	    },
	    { /* Version 9 */
		    292,
		    {6, 26, 46, 0},
		    {
			    {58, 36, 3},
			    {146, 116, 2},
			    {36, 12, 4},
			    {36, 16, 4}
		    }
	    },
	    { /* Version 10 */
		    346,
		    {6, 28, 50, 0},
		    {
			    {69, 43, 4},
			    {86, 68, 2},
			    {43, 15, 6},
			    {43, 19, 6}
		    }
	    },
	    { /* Version 11 */
		    404,
		    {6, 30, 54, 0},
		    {
			    {80, 50, 1},
			    {101, 81, 4},
			    {36, 12, 3},
			    {50, 22, 4}
		    }
	    },
	    { /* Version 12 */
		    466,
		    {6, 32, 58, 0},
		    {
			    {58, 36, 6},
			    {116, 92, 2},
			    {42, 14, 7},
			    {46, 20, 4}
		    }
	    },
	    { /* Version 13 */
		    532,
		    {6, 34, 62, 0},
		    {
			    {59, 37, 8},
			    {133, 107, 4},
			    {33, 11, 12},
			    {44, 20, 8}
		    }
	    },
	    { /* Version 14 */
		    581,
		    {6, 26, 46, 66, 0},
		    {
			    {64, 40, 4},
			    {145, 115, 3},
			    {36, 12, 11},
			    {36, 16, 11}
		    }
	    },
	    { /* Version 15 */
		    655,
		    {6, 26, 48, 70, 0},
		    {
			    {65, 41, 5},
			    {109, 87, 5},
			    {36, 12, 11},
			    {54, 24, 5}
		    }
	    },
	    { /* Version 16 */
		    733,
		    {6, 26, 50, 74, 0},
		    {
			    {73, 45, 7},
			    {122, 98, 5},
			    {45, 15, 3},
			    {43, 19, 15}
		    }
	    },
	    { /* Version 17 */
		    815,
		    {6, 30, 54, 78, 0},
		    {
			    {74, 46, 10},
			    {135, 107, 1},
			    {42, 14, 2},
			    {50, 22, 1}
		    }
	    },
	    { /* Version 18 */
		    901,
		    {6, 30, 56, 82, 0},
		    {
			    {69, 43, 9},
			    {150, 120, 5},
			    {42, 14, 2},
			    {50, 22, 17}
		    }
	    },
	    { /* Version 19 */
		    991,
		    {6, 30, 58, 86, 0},
		    {
			    {70, 44, 3},
			    {141, 113, 3},
			    {39, 13, 9},
			    {47, 21, 17}
		    }
	    },
	    { /* Version 20 */
		    1085,
		    {6, 34, 62, 90, 0},
		    {
			    {67, 41, 3},
			    {135, 107, 3},
			    {43, 15, 15},
			    {54, 24, 15}
		    }
	    },
	    { /* Version 21 */
		    1156,
		    {6, 28, 50, 72, 92, 0},
		    {
			    {68, 42, 17},
			    {144, 116, 4},
			    {46, 16, 19},
			    {50, 22, 17}
		    }
	    },
	    { /* Version 22 */
		    1258,
		    {6, 26, 50, 74, 98, 0},
		    {
			    {74, 46, 17},
			    {139, 111, 2},
			    {37, 13, 34},
			    {54, 24, 7}
		    }
	    },
	    { /* Version 23 */
		    1364,
		    {6, 30, 54, 78, 102, 0},
		    {
			    {75, 47, 4},
			    {151, 121, 4},
			    {45, 15, 16},
			    {54, 24, 11}
		    }
	    },
	    { /* Version 24 */
		    1474,
		    {6, 28, 54, 80, 106, 0},
		    {
			    {73, 45, 6},
			    {147, 117, 6},
			    {46, 16, 30},
			    {54, 24, 11}
		    }
	    },
	    { /* Version 25 */
		    1588,
		    {6, 32, 58, 84, 110, 0},
		    {
			    {75, 47, 8},
			    {132, 106, 8},
			    {45, 15, 22},
			    {54, 24, 7}
		    }
	    },
	    { /* Version 26 */
		    1706,
		    {6, 30, 58, 86, 114, 0},
		    {
			    {74, 46, 19},
			    {142, 114, 10},
			    {46, 16, 33},
			    {50, 22, 28}
		    }
	    },
	    { /* Version 27 */
		    1828,
		    {6, 34, 62, 90, 118, 0},
		    {
			    {73, 45, 22},
			    {152, 122, 8},
			    {45, 15, 12},
			    {53, 23, 8}
		    }
	    },
	    { /* Version 28 */
		    1921,
		    {6, 26, 50, 74, 98, 122, 0},
		    {
			    {73, 45, 3},
			    {147, 117, 3},
			    {45, 15, 11},
			    {54, 24, 4}
		    }
	    },
	    { /* Version 29 */
		    2051,
		    {6, 30, 54, 78, 102, 126, 0},
		    {
			    {73, 45, 21},
			    {146, 116, 7},
			    {45, 15, 19},
			    {53, 23, 1}
		    }
	    },
	    { /* Version 30 */
		    2185,
		    {6, 26, 52, 78, 104, 130, 0},
		    {
			    {75, 47, 19},
			    {145, 115, 5},
			    {45, 15, 23},
			    {54, 24, 15}
		    }
	    },
	    { /* Version 31 */
		    2323,
		    {6, 30, 56, 82, 108, 134, 0},
		    {
			    {74, 46, 2},
			    {145, 115, 13},
			    {45, 15, 23},
			    {54, 24, 42}
		    }
	    },
	    { /* Version 32 */
		    2465,
		    {6, 34, 60, 86, 112, 138, 0},
		    {
			    {74, 46, 10},
			    {145, 115, 17},
			    {45, 15, 19},
			    {54, 24, 10}
		    }
	    },
	    { /* Version 33 */
		    2611,
		    {6, 30, 58, 86, 114, 142, 0},
		    {
			    {74, 46, 14},
			    {145, 115, 17},
			    {45, 15, 11},
			    {54, 24, 29}
		    }
	    },
	    { /* Version 34 */
		    2761,
		    {6, 34, 62, 90, 118, 146, 0},
		    {
			    {74, 46, 14},
			    {145, 115, 13},
			    {46, 16, 59},
			    {54, 24, 44}
		    }
	    },
	    { /* Version 35 */
		    2876,
		    {6, 30, 54, 78, 102, 126, 150},
		    {
			    {75, 47, 12},
			    {151, 121, 12},
			    {45, 15, 22},
			    {54, 24, 39}
		    }
	    },
	    { /* Version 36 */
		    3034,
		    {6, 24, 50, 76, 102, 128, 154},
		    {
			    {75, 47, 6},
			    {151, 121, 6},
			    {45, 15, 2},
			    {54, 24, 46}
		    }
	    },
	    { /* Version 37 */
		    3196,
		    {6, 28, 54, 80, 106, 132, 158},
		    {
			    {74, 46, 29},
			    {152, 122, 17},
			    {45, 15, 24},
			    {54, 24, 49}
		    }
	    },
	    { /* Version 38 */
		    3362,
		    {6, 32, 58, 84, 110, 136, 162},
		    {
			    {74, 46, 13},
			    {152, 122, 4},
			    {45, 15, 42},
			    {54, 24, 48}
		    }
	    },
	    { /* Version 39 */
		    3532,
		    {6, 26, 54, 82, 110, 138, 166},
		    {
			    {75, 47, 40},
			    {147, 117, 20},
			    {45, 15, 10},
			    {54, 24, 43}
		    }
	    },
	    { /* Version 40 */
		    3706,
		    {6, 30, 58, 86, 114, 142, 170},
		    {
			    {75, 47, 18},
			    {148, 118, 19},
			    {45, 15, 20},
			    {54, 24, 34}
		    }
	    }
};
