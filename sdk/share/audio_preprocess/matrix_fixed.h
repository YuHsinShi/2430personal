/*
 * matrix_fixed.h
 *
 *  Created on: 2018¦~7¤ë19¤é
 *      Author: USER
 */

#ifndef MATRIX_FIXED_H_
#define MATRIX_FIXED_H_

extern "C" {
#include "type_def.h"
#include "basic_op.h"
}
#define FIXMATRIX_MAX_SIZE (64)

typedef struct {
	int64_t real;
	int64_t imag;
} ComplexInt64;

typedef struct {
	int rows;
	int cols;
	ComplexInt16 data[FIXMATRIX_MAX_SIZE][FIXMATRIX_MAX_SIZE];
} mcf16_t;

typedef struct {
	int rows;
	int cols;
	ComplexInt64 data[FIXMATRIX_MAX_SIZE][FIXMATRIX_MAX_SIZE];
} mcf64_t;

extern "C" {
ComplexInt16 cfa16_dot(const ComplexInt16 *a, int a_stride,
		const ComplexInt16 *b, int b_stride, int n);
void mcf16_mul(mcf16_t *dest, const mcf16_t *a, const mcf16_t *b);
void mcf16_mul_bt(mcf16_t *dest, const mcf16_t *a, const mcf16_t *bt);
ComplexInt64 cfa64_dot(ComplexInt64 const *a, int a_stride,
		ComplexInt64 const *b, int b_stride, int n);
void mcf64_mul(mcf64_t *dest, mcf64_t const *a, mcf64_t const *b);
void mcf64_mul_ah(mcf64_t __restrict *dest, mcf64_t const *ah,
		mcf64_t const *b);
void mcf64_mul_at(mcf64_t *dest, mcf64_t const *at, mcf64_t const *b);
void mcf64_mul_bt(mcf64_t *dest, mcf64_t const *a, mcf64_t const *bt);
}

#endif /* MATRIX_FIXED_H_ */
