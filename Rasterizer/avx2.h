#pragma once
#include <immintrin.h>

static void add_avx2(float* a, float* b, float* res) {
	// Load 8 floats into 256-bit registers
	__m256 va = _mm256_load_ps(a); // aligned memory
	__m256 vb = _mm256_load_ps(b);

	// Perform element-wise addition
	__m256 vc = _mm256_add_ps(va, vb);

	// Store the result back to memory
	_mm256_store_ps(res, vc);
}

static void mul_avx2(float* a, float* b, float* res)
{
	// Load 8 floats into 256-bit registers
	__m256 va = _mm256_load_ps(a); // aligned memory
	__m256 vb = _mm256_load_ps(b);

	// Perform element-wise addition
	__m256 vc = _mm256_mul_ps(va, vb);

	// Store the result back to memory
	_mm256_store_ps(res, vc);
}

static float dot_avx2(unsigned int size, float* vec1, float* vec2)
{
	__m256 vc = _mm256_setzero_ps();

	for (int i = 0; i < size; i += 8)
	{
		// Load 8 floats into 256-bit registers
		__m256 va = _mm256_load_ps(&vec1[i]); // aligned memory
		__m256 vb = _mm256_load_ps(&vec2[i]);

		// Perform element-wise addition
		vc = _mm256_fmadd_ps(va, vb, vc);
	}

	alignas(64) float sum[8];
	// Store the result back to memory
	_mm256_store_ps(sum, vc);

	float res = 0;
	for (int i = 0; i < 8; i++)
		res += sum[i];

	return res;
}