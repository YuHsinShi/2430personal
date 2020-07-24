// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2009 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This is a pure C header, no C++ here.
// The functions declared here will be implemented in C++ but
// we don't have to know, because thanks to the extern "C" syntax,
// they will be compiled to C object code.
#include <iostream>

#if !defined(_MSC_VER)
#ifdef __cplusplus
extern "C"
{
#endif

	// just dummy empty structs to give different pointer types,
	// instead of using void* which would be type unsafe
	struct C_MatrixXd
	{
	};
	struct C_Map_MatrixXd
	{
	};
	struct C_MatrixXcf
	{
	};
	struct C_MatrixXi
	{
	};
	struct C_MatrixXi64_t
	{
	};
	struct C_MatrixXci64_t
	{
	};
	struct C_MatrixXf
	{
	};

	void MatrixXcf_Inverse(C_MatrixXcf *m1, C_MatrixXcf *m2, C_MatrixXcf *result);
	void MatrixXcf_KSICA(C_MatrixXcf *m1, C_MatrixXcf *m2, C_MatrixXcf *m3,
						 C_MatrixXcf *m4, C_MatrixXcf *m5, float *rY, C_MatrixXcf *m6,
						 C_MatrixXcf *result, int band);
	void matrix2ci_fastica(mcf64_t *m1, mcf64_t *result, int L);
	void LDA_Analysis(double *m1, double *m2, double *result, int L = 0);
	void ICA_Decomposition(double *m1, double *m2, double *result, int L = 0);
	void MatrixXcf_CMN(C_MatrixXcf *m1, C_MatrixXcf *result);
	void MatrixXi_PCA(struct C_MatrixXi64_t *m1, struct C_MatrixXi64_t *result,
					  struct C_MatrixXi64_t *m_eivalues);
	void MatrixXci_PCA(struct C_MatrixXci64_t *m1, struct C_MatrixXci64_t *result,
					   struct C_MatrixXci64_t *m_eivalues);
	void MatrixXcf_PCA(struct C_MatrixXcf *m1, struct C_MatrixXcf *result,
					   struct C_MatrixXcf *m_eivalues);
	void MatrixXcf_FastICA(struct C_MatrixXcf *m1, struct C_MatrixXcf *result,
						   int L);
	void MatrixXci_FastICA(struct C_MatrixXci64_t *m1,
						   struct C_MatrixXci64_t *result, int L);
	struct C_MatrixXi64_t *MatrixXi64_t_new(int rows, int cols);
	void MatrixXi64_t_delete(struct C_MatrixXi64_t *m);
	struct C_MatrixXcf *MatrixXcf_new(int rows, int cols);
	struct C_MatrixXci64_t *MatrixXci64_t_new(int rows, int cols);
	void MatrixXcf_delete(struct C_MatrixXcf *m);
	void MatrixXcf_resize(struct C_MatrixXcf *m, int rows, int cols);
	void MatrixXcf_copy(struct C_MatrixXcf *dst, const struct C_MatrixXcf *src);
	void MatrixXcf_set_Identity(struct C_MatrixXcf *m, int rows, int cols);
	void MatrixXcf_set_Ones(C_MatrixXcf *m, int rows, int cols);
	void MatrixXi64_t_set_coeff(struct C_MatrixXi64_t *m, int i, int j,
								int64_t coeff);
	void MatrixXcf_set_coeff(struct C_MatrixXcf *m, int i, int j,
							 float _Complex coeff);
	float _Complex MatrixXcf_get_coeff(const struct C_MatrixXcf *m, int i, int j);
	void MatrixXi64_t_print(struct C_MatrixXi64_t const *m);
	void MatrixXcf_print(const struct C_MatrixXcf *m);
	void MatrixXcf_add(const struct C_MatrixXcf *m1, const struct C_MatrixXcf *m2,
					   struct C_MatrixXcf *result);
	void MatrixXcf_multiply(const struct C_MatrixXcf *m1,
							const struct C_MatrixXcf *m2, struct C_MatrixXcf *result);
	void MatrixXcf_adjoint(const struct C_MatrixXcf *m1,
						   struct C_MatrixXcf *result);
	void MatrixXcf_scalar(const struct C_MatrixXcf *m1, struct C_MatrixXcf *result,
						  float _Complex coeff);
	void MatrixXcf_norm(struct C_MatrixXcf const *m, struct C_MatrixXcf *result);
	void MatrixXcf_pinv(C_MatrixXcf *m1, C_MatrixXcf *result);
	void MatrixXcf_inverse(const struct C_MatrixXcf *m1,
						   struct C_MatrixXcf *result);
	void MatrixXcf_eigensolver(const struct C_MatrixXcf *m1,
							   struct C_MatrixXcf *result, struct C_MatrixXcf *m_eivalues);
	void MatrixXcf_columnswap(struct C_MatrixXcf *m1, int i, int j);
	void MatrixXcf_rowswap(struct C_MatrixXcf *m1, int i, int j);
	void MatrixXcf_transpose(struct C_MatrixXcf *m1, struct C_MatrixXcf *result);
	void MatrixXcf_sub(const struct C_MatrixXcf *m1, const struct C_MatrixXcf *m2,
					   struct C_MatrixXcf *result);
	void MatrixXcf_diagonal(struct C_MatrixXcf *m1, struct C_MatrixXcf *result);
	void MatrixXcf_orthonormal(struct C_MatrixXcf *m1, struct C_MatrixXcf *result);
	float MatrixXcf_cols_norm(struct C_MatrixXcf *m1);

	// the C_MatrixXd class, wraps some of the functionality
	// of Eigen::MatrixXd.
	struct C_MatrixXd *MatrixXd_new(int rows, int cols);
	void MatrixXd_delete(struct C_MatrixXd *m);
	double *MatrixXd_data(struct C_MatrixXd *m);
	void MatrixXd_set_zero(struct C_MatrixXd *m);
	void MatrixXd_resize(struct C_MatrixXd *m, int rows, int cols);
	void MatrixXd_copy(struct C_MatrixXd *dst, const struct C_MatrixXd *src);
	void MatrixXd_copy_map(struct C_MatrixXd *dst,
						   const struct C_Map_MatrixXd *src);
	void MatrixXd_set_coeff(struct C_MatrixXd *m, int i, int j, double coeff);
	double MatrixXd_get_coeff(const struct C_MatrixXd *m, int i, int j);
	void MatrixXd_print(const struct C_MatrixXd *m);
	void MatrixXd_add(const struct C_MatrixXd *m1, const struct C_MatrixXd *m2,
					  struct C_MatrixXd *result);
	void MatrixXd_multiply(const struct C_MatrixXd *m1, const struct C_MatrixXd *m2,
						   struct C_MatrixXd *result);

	// the C_Map_MatrixXd class, wraps some of the functionality
	// of Eigen::Map<MatrixXd>
	struct C_Map_MatrixXd *Map_MatrixXd_new(double *array, int rows, int cols);
	void Map_MatrixXd_delete(struct C_Map_MatrixXd *m);
	void Map_MatrixXd_set_zero(struct C_Map_MatrixXd *m);
	void Map_MatrixXd_copy(struct C_Map_MatrixXd *dst,
						   const struct C_Map_MatrixXd *src);
	void Map_MatrixXd_copy_matrix(struct C_Map_MatrixXd *dst,
								  const struct C_MatrixXd *src);
	void Map_MatrixXd_set_coeff(struct C_Map_MatrixXd *m, int i, int j,
								double coeff);
	double Map_MatrixXd_get_coeff(const struct C_Map_MatrixXd *m, int i, int j);
	void Map_MatrixXd_print(const struct C_Map_MatrixXd *m);
	void Map_MatrixXd_add(const struct C_Map_MatrixXd *m1,
						  const struct C_Map_MatrixXd *m2, struct C_Map_MatrixXd *result);
	void Map_MatrixXd_multiply(const struct C_Map_MatrixXd *m1,
							   const struct C_Map_MatrixXd *m2, struct C_Map_MatrixXd *result);

#ifdef __cplusplus
} // end extern "C"
#endif
#endif
