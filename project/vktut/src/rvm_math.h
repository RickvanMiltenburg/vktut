// Copyright (c) 2016 Rick van Miltenburg
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial
// portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __RVM_MATH_H
#define __RVM_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Options

#define RVM_MATH_VECTOR_INSTR_SET_NONE 0
#define RVM_MATH_VECTOR_INSTR_SET_SSE  1
#define RVM_MATH_VECTOR_INSTR_SET_AVX  2

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// User config

#ifdef _WIN32
#define RVM_MATH_VECTOR_INSTR_SET RVM_MATH_VECTOR_INSTR_SET_SSE
#else
#define RVM_MATH_VECTOR_INSTR_SET RVM_MATH_VECTOR_INSTR_SET_NONE
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>

typedef struct { union { struct { float *x; }; float* cells[1]; }; uint32_t vectorCount; } rvm_soa_vec1;
typedef struct { union { struct { float *x, *y; }; float* cells[2]; }; uint32_t vectorCount; } rvm_soa_vec2;
typedef struct { union { struct { float *x, *y, *z; }; float* cells[3]; }; uint32_t vectorCount; } rvm_soa_vec3;
typedef struct { union { struct { float *x, *y, *z, *w; }; float* cells[4]; }; uint32_t vectorCount; } rvm_soa_vec4;
typedef struct { union { float* rows[4][4]; float* cells[16]; }; uint32_t matrixCount; } rvm_soa_mat4;

typedef struct { union { struct { float x, y; }; float cells[2]; }; } rvm_aos_vec2;
typedef struct { union { struct { float x, y, z; }; float cells[3]; }; } rvm_aos_vec3;
typedef struct { union { struct { float x, y, z, w; }; float cells[4]; }; } rvm_aos_vec4;
typedef struct { union { float rows[3][3]; float cells[9]; }; } rvm_aos_mat3;
typedef struct { union { float rows[4][4]; float cells[16]; }; } rvm_aos_mat4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define RVM_PI  3.141592654f
#define RVM_TAU 6.283185307f
#define RVM_DEG2RAD(d) ((d)/180.0f*(RVM_PI))
#define RVM_RAD2DEG(r) ((r)/(RVM_PI)*180.0f)
#define RVM_BIT(n) (1<<(n))
#define RVM_ALIGN_UP_POW2(x,n) (((x)+((n)-1))&(~((n)-1)))
#define RVM_MIN(x,y) (((x)<(y))?(x):(y))
#define RVM_MAX(x,y) (((x)>(y))?(x):(y))
#define RVM_CLAMP(x,xMin,xMax) (RVM_MAX((xMin),RVM_MIN((xMax),(x))))
#define RVM_DIV_CEIL(x,y) (((x)+((y)-1))/(y))

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	
uint32_t rvm_is_pow2 ( uint32_t v );
uint32_t rvm_floor_pow2 ( uint32_t v );
uint32_t rvm_ceil_pow2 ( uint32_t v );
uint32_t rvm_index_pow2 ( uint32_t v );

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t rvm_swizzle_mask ( uint32_t x, uint32_t swizzleMask );

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void rvm_soa_vec2_init ( rvm_soa_vec2* v, float* x, float* y, uint32_t vecCount );
void rvm_soa_vec3_init ( rvm_soa_vec3* v, float* x, float* y, float* z, uint32_t vecCount );
void rvm_soa_vec4_init ( rvm_soa_vec4* v, float* x, float* y, float* z, float* w, uint32_t vecCount );
void rvm_soa_mat4_init_auto ( rvm_soa_mat4* m, float* data, uint32_t matCount );

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void rvm_soa_vec3_extract ( rvm_aos_vec3* o, const rvm_soa_vec3* v, uint32_t i );
void rvm_soa_vec4_extract ( rvm_aos_vec4* o, const rvm_soa_vec4* v, uint32_t i );
void rvm_soa_mat4_extract ( rvm_aos_mat4* out, const rvm_soa_mat4* mats, uint32_t matIndex );

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void rvm_soa_vec2_overwrite_add_xy_float ( rvm_soa_vec2* v, const float add );
void rvm_soa_vec2_overwrite_mul_soa_vec4_w ( rvm_soa_vec2* o, const rvm_soa_vec4* v );

void rvm_soa_vec4_mul_x_float  ( rvm_soa_vec1* out, const rvm_soa_vec4* v, float f );
void rvm_soa_vec4_mul_y_float  ( rvm_soa_vec1* out, const rvm_soa_vec4* v, float f );
void rvm_soa_vec4_mul_xy_float ( rvm_soa_vec2* out, const rvm_soa_vec4* v, float f );
void rvm_soa_vec4_mad_x  ( rvm_soa_vec1* out, const rvm_soa_vec4* v, float multiply, float add );
void rvm_soa_vec4_mad_y  ( rvm_soa_vec1* out, const rvm_soa_vec4* v, float multiply, float add );
void rvm_soa_vec4_mad_xy ( rvm_soa_vec2* out, const rvm_soa_vec4* v, float multiply, float add );

void rvm_soa_vec4_overwrite_mul_x_float  ( rvm_soa_vec4* v, float f );
void rvm_soa_vec4_overwrite_mul_y_float  ( rvm_soa_vec4* v, float f );
void rvm_soa_vec4_overwrite_mul_xy_float ( rvm_soa_vec4* v, float f );
void rvm_soa_vec4_overwrite_mad_x  ( rvm_soa_vec4* v, float multiply, float add );
void rvm_soa_vec4_overwrite_mad_y  ( rvm_soa_vec4* v, float multiply, float add );
void rvm_soa_vec4_overwrite_mad_xy ( rvm_soa_vec4* v, float multiply, float add );

void rvm_soa_vec4_div_xy_w  ( rvm_soa_vec2* out, const rvm_soa_vec4* v );
void rvm_soa_vec4_div_xyz_w ( rvm_soa_vec3* out, const rvm_soa_vec4* v );

void rvm_soa_vec4_overwrite_mul_xy_w  ( rvm_soa_vec4* v );
void rvm_soa_vec4_overwrite_mul_xyz_w ( rvm_soa_vec4* v );
void rvm_soa_vec4_overwrite_div_xy_w  ( rvm_soa_vec4* v );
void rvm_soa_vec4_overwrite_div_xyz_w ( rvm_soa_vec4* v );

void rvm_soa_vec4_overwrite_rcp_w ( rvm_soa_vec4* v );

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void rvm_aos_vec3_normalize ( rvm_aos_vec3* out, const rvm_aos_vec3* v1 );
float rvm_aos_vec3_length ( const rvm_aos_vec3* v );
float rvm_aos_vec3_sqlength ( const rvm_aos_vec3* v );
float rvm_aos_vec3_dot_aos_vec3 ( const rvm_aos_vec3* v1, const rvm_aos_vec3* v2 );
void rvm_aos_vec3_cross_aos_vec3 ( rvm_aos_vec3* out, const rvm_aos_vec3* v1, const rvm_aos_vec3* v2 );
void rvm_aos_vec3_add_aos_vec3 ( rvm_aos_vec3* out, const rvm_aos_vec3* v1, const rvm_aos_vec3* v2 );
void rvm_aos_vec3_sub_aos_vec3 ( rvm_aos_vec3* out, const rvm_aos_vec3* v1, const rvm_aos_vec3* v2 );

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

float rvm_aos_mat4_max ( const rvm_aos_mat4* m );
float rvm_aos_mat4_min ( const rvm_aos_mat4* m );
rvm_aos_vec3 rvm_soa_vec3_min_xyz ( const rvm_soa_vec3* v );
rvm_aos_vec3 rvm_soa_vec3_max_xyz ( const rvm_soa_vec3* v );

rvm_aos_mat3 rvm_aos_mat3_mul_aos_mat3 ( const rvm_aos_mat3* m1, const rvm_aos_mat3* m2 );
rvm_aos_vec2 rvm_aos_mat3_mul_aos_vec2z0 ( const rvm_aos_mat3* m, const rvm_aos_vec2* v );
rvm_aos_vec2 rvm_aos_mat3_mul_aos_vec2z1 ( const rvm_aos_mat3* m, const rvm_aos_vec2* v );
rvm_aos_vec3 rvm_aos_mat3_mul_aos_vec2z1_out_vec3 ( const rvm_aos_mat3* m, const rvm_aos_vec2* v );
rvm_aos_vec3 rvm_aos_mat3_mul_aos_vec3 ( const rvm_aos_mat3* m, const rvm_aos_vec3* v );

rvm_aos_mat4 rvm_aos_mat4_mul_aos_mat4 ( const rvm_aos_mat4* m1, const rvm_aos_mat4* m2 );
rvm_aos_vec3 rvm_aos_mat4_mul_aos_vec3w0 ( const rvm_aos_mat4* m, const rvm_aos_vec3* v );
rvm_aos_vec3 rvm_aos_mat4_mul_aos_vec3w1 ( const rvm_aos_mat4* m, const rvm_aos_vec3* v );
rvm_aos_vec4 rvm_aos_mat4_mul_aos_vec3w1_out_vec4 ( const rvm_aos_mat4* m, const rvm_aos_vec3* v );
rvm_aos_vec4 rvm_aos_mat4_mul_aos_vec4 ( const rvm_aos_mat4* m, const rvm_aos_vec4* v );

void rvm_aos_mat4_mul_soa_mat4 ( rvm_soa_mat4* out, const rvm_aos_mat4* m1, const rvm_soa_mat4* m2 );
void rvm_aos_mat4_mul_soa_vec3w0 ( rvm_soa_vec3* out, const rvm_aos_mat4* m, const rvm_soa_vec3* v );
void rvm_aos_mat4_mul_soa_vec3w1 ( rvm_soa_vec3* out, const rvm_aos_mat4* m, const rvm_soa_vec3* v );
void rvm_aos_mat4_mul_soa_vec3w1_out_vec4 ( rvm_soa_vec4* out, const rvm_aos_mat4* m, const rvm_soa_vec3* v );
void rvm_aos_mat4_mul_soa_vec4 ( rvm_soa_vec4* out, const rvm_aos_mat4* m, const rvm_soa_vec4* v );

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

rvm_aos_mat3 rvm_aos_mat3_translate ( float x, float y );//NOTE(Rick): Generates 2D translation matrix
rvm_aos_mat3 rvm_aos_mat3_rotate ( float angleRad );		//NOTE(Rick): Generates 2D rotation matrix
rvm_aos_mat3 rvm_aos_mat3_scale ( float x, float y);		//NOTE(Rick): Generates 2D scale matrix

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

rvm_aos_mat4 rvm_aos_mat4_perspective ( float fovY, float aspect, float near, float far );//NOTE(Rick): Generates 3D projection matrix
rvm_aos_mat4 rvm_aos_mat4_orthographic ( float width, float height, float near, float far );//NOTE(Rick): Generates 3D projection matrix
rvm_aos_mat4 rvm_aos_mat4_translate ( float x, float y, float z );//NOTE(Rick): Generates 3D translation matrix
rvm_aos_mat4 rvm_aos_mat4_rotate_x ( float angleRad );	//NOTE(Rick): Generates 3D rotation matrix
rvm_aos_mat4 rvm_aos_mat4_rotate_y ( float angleRad );	//NOTE(Rick): Generates 3D rotation matrix
rvm_aos_mat4 rvm_aos_mat4_rotate_z ( float angleRad );	//NOTE(Rick): Generates 3D rotation matrix
rvm_aos_mat4 rvm_aos_mat4_scale ( float x, float y, float z );		//NOTE(Rick): Generates 3D scale matrix

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

float rvm_aos_mat4_determinant ( const rvm_aos_mat4* m );
rvm_aos_mat4 rvm_aos_mat4_transpose ( const rvm_aos_mat4* m );
rvm_aos_mat4 rvm_aos_mat4_inverse ( const rvm_aos_mat4* m );

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef RVM_MATH_IMPLEMENTATION

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	#define RVM_MATH_VECTOR_WIDTH 4
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	#define RVM_MATH_VECTOR_WIDTH 8
#else
	#define RVM_MATH_VECTOR_WIDTH 1
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <float.h>
#include <math.h>

#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE || RVM_MATH_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
#include <immintrin.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t rvm_is_pow2 ( uint32_t x )
{
	return (x & (x-1)) == 0;
}

uint32_t rvm_floor_pow2 ( uint32_t x )
{
#if LZCNT
	return x & (1 << _lzcnt_u32 ( x ));
#else
	uint32_t x2  = x & (x-1);
	while ( x2 )
	{
		x  = x2;
		x2 = x & (x-1);
	}
	return x;
#endif
}

uint32_t rvm_ceil_pow2 ( uint32_t x )
{
#if LZCNT
	return (x & (1 << _lzcnt_u32 ( x ))) << (_mm_popcnt_u32 ( x ) > 1);
#else
	uint32_t x2  = x & (x-1);
	uint32_t shl = 0;
	while ( x2 )
	{
		shl |= 0x1;

		x  = x2;
		x2 = x & (x-1);
	}
	return x << shl;
#endif
}

uint32_t rvm_index_pow2 ( uint32_t x )
{
#if LZCNT
	return _lzcnt_u32 ( x );
#else
	for ( int32_t i = 31; i >= 0; i-- )
	{
		if ( x & (1<<i) )
			return i;
	}
	return 32;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !PDEP
static inline uint32_t pdep_em ( uint32_t x, uint32_t mask )
{
	uint32_t dst = 0;
	uint32_t m = 0;
	uint32_t k = 0;

	while ( m < 32 )
	{
		if ( mask & (1<<m) )
		{
			dst |= ((x >> k) & 0x1) << m;
			k++;
		}
		m++;
	}
	return dst;
}
#endif

uint32_t rvm_swizzle_mask ( uint32_t x, uint32_t swizzleMask )
{
#if PDEP
	return _pdep_u32(x, swizzleMask);
#else
	return pdep_em(x, swizzleMask);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void rvm_soa_vec2_init ( rvm_soa_vec2* v, float* x, float* y, uint32_t vecCount )
{
	assert ( (((uintptr_t)x) & (RVM_MATH_VECTOR_WIDTH*4-1)) == 0 );	// Invalid alignment
	assert ( (((uintptr_t)y) & (RVM_MATH_VECTOR_WIDTH*4-1)) == 0 );	// Invalid alignment

	v->x = x;
	v->y = y;
	v->vectorCount = vecCount;
}

void rvm_soa_vec3_init ( rvm_soa_vec3* v, float* x, float* y, float* z, uint32_t vecCount )
{
	assert ( (((uintptr_t)x) & (RVM_MATH_VECTOR_WIDTH*4-1)) == 0 );	// Invalid alignment
	assert ( (((uintptr_t)y) & (RVM_MATH_VECTOR_WIDTH*4-1)) == 0 );	// Invalid alignment
	assert ( (((uintptr_t)z) & (RVM_MATH_VECTOR_WIDTH*4-1)) == 0 );	// Invalid alignment

	v->x = x;
	v->y = y;
	v->z = z;
	v->vectorCount = vecCount;
}

void rvm_soa_vec4_init ( rvm_soa_vec4* v, float* x, float* y, float* z, float* w, uint32_t vecCount )
{
	assert ( (((uintptr_t)x) & (RVM_MATH_VECTOR_WIDTH*4-1)) == 0 );	// Invalid alignment
	assert ( (((uintptr_t)y) & (RVM_MATH_VECTOR_WIDTH*4-1)) == 0 );	// Invalid alignment
	assert ( (((uintptr_t)z) & (RVM_MATH_VECTOR_WIDTH*4-1)) == 0 );	// Invalid alignment
	assert ( (((uintptr_t)w) & (RVM_MATH_VECTOR_WIDTH*4-1)) == 0 );	// Invalid alignment

	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
	v->vectorCount = vecCount;
}

void rvm_soa_mat4_init_auto ( rvm_soa_mat4* m, float* data, uint32_t matCount )
{
	assert ( (((uintptr_t)data) & (RVM_MATH_VECTOR_WIDTH*4-1)) == 0 );	// Invalid alignment
	assert ( (matCount & 3) == 0 );				// Must be a multiple of 4 for this version of the constructor!

	for ( uint32_t i = 0; i < (4*4); i++, data += matCount )
		m->cells[i] = data;
	m->matrixCount = matCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void rvm_soa_vec3_extract ( rvm_aos_vec3* o, const rvm_soa_vec3* v, uint32_t i )
{
	assert ( i < v->vectorCount );
	o->x = v->x[i], o->y = v->y[i], o->z = v->z[i];
}

void rvm_soa_vec4_extract ( rvm_aos_vec4* o, const rvm_soa_vec4* v, uint32_t i )
{
	assert ( i < v->vectorCount );
	o->x = v->x[i], o->y = v->y[i], o->z = v->z[i], o->w = v->w[i];
}

void rvm_soa_mat4_extract ( rvm_aos_mat4* out, const rvm_soa_mat4* mats, uint32_t matIndex )
{
	assert ( matIndex < mats->matrixCount );
	float* outptr = out->cells;
	const float* const* inptr = mats->cells;
	for ( uint32_t i = 0; i < 16; i++ )
		*(outptr++) = (*inptr++)[matIndex];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void rvm_soa_vec2_overwrite_add_xy_float ( rvm_soa_vec2* v, const float add )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x, *y = v->y;
	for ( uint32_t i = 0; i < v->vectorCount; i++, x++, y++ )
	{
		*(x) = *(x) + add;
		*(y) = *(y) + add;
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 a = _mm_set1_ps ( add );
	__m128 *x = (__m128*)v->x, *y = (__m128*)v->y;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, x++ )
		*(x) = _mm_add_ps ( *x, a );
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, y++ )
		*(y) = _mm_add_ps ( *y, a );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 a = _mm256_set1_ps ( add );
	__m256 *x = (__m256*)v->x, *y = (__m256*)v->y;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, x++ )
		*(x) = _mm256_add_ps ( *x, a );
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, y++ )
		*(y) = _mm256_add_ps ( *y, a );
#endif
}

void rvm_soa_vec2_overwrite_mul_soa_vec4_w ( rvm_soa_vec2* o, const rvm_soa_vec4* v )
{
	assert ( o->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *w = v->w;
	float *ox = o->x, *oy = o->y;
	for ( uint32_t i = 0; i < v->vectorCount; i++, ox++, oy++, w++ )
	{
		*(ox) = *(ox) * *(w);
		*(oy) = *(oy) * *(w);
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 *w  = (__m128*)v->w;
	__m128 *ox = (__m128*)o->x, *oy = (__m128*)o->y;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, ox++, w++ )
		*(ox) = _mm_mul_ps ( *ox, *w );
	w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, oy++, w++ )
		*(oy) = _mm_mul_ps ( *oy, *w );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 *w  = (__m256*)v->w;
	__m256 *ox = (__m256*)o->x, *oy = (__m256*)o->y;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, ox++, w++ )
		*(ox) = _mm256_mul_ps ( *ox, *w );
	w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, oy++, w++ )
		*(oy) = _mm256_mul_ps ( *oy, *w );
#endif
}

void rvm_soa_vec4_mul_x_float ( rvm_soa_vec1* out, const rvm_soa_vec4* v, float f )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x;
	float *ox = out->x;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(ox++) = *(x++) * f;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 fv  = _mm_set1_ps ( f );
	__m128 *x  = (__m128*)v->x;
	__m128 *ox = (__m128*)out->x;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(ox++) = _mm_mul_ps ( *(x++), fv );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 fv  = _mm256_set1_ps ( f );
	__m256 *x  = (__m256*)v->x;
	__m256 *ox = (__m256*)out->x;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++ )
		*(ox++) = _mm256_mul_ps ( *(x++), fv );
#endif
}

void rvm_soa_vec4_mul_y_float ( rvm_soa_vec1* out, const rvm_soa_vec4* v, float f )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *y = v->y;
	float *oy = out->x;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(oy++) = *(y++) * f;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 fv  = _mm_set1_ps ( f );
	__m128 *y  = (__m128*)v->y;
	__m128 *oy = (__m128*)out->x;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(oy++) = _mm_mul_ps ( *(y++), fv );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 fv  = _mm256_set1_ps ( f );
	__m256 *y  = (__m256*)v->y;
	__m256 *oy = (__m256*)out->x;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++ )
		*(oy++) = _mm256_mul_ps ( *(y++), fv );
#endif
}

void rvm_soa_vec4_mul_xy_float ( rvm_soa_vec2* out, const rvm_soa_vec4* v, float f )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x, *y = v->y;
	float *ox = out->x, *oy = out->y;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(ox++) = *(x++) * f;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(oy++) = *(y++) * f;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 fv  = _mm_set1_ps ( f );
	__m128 *x  = (__m128*)v->x,   *y  = (__m128*)v->y;
	__m128 *ox = (__m128*)out->x, *oy = (__m128*)out->y;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(ox++) = _mm_mul_ps ( *(x++), fv );
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(oy++) = _mm_mul_ps ( *(y++), fv );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 fv  = _mm256_set1_ps ( f );
	__m256 *x  = (__m256*)v->x,   *y  = (__m256*)v->y;
	__m256 *ox = (__m256*)out->x, *oy = (__m256*)out->y;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++ )
		*(ox++) = _mm256_mul_ps ( *(x++), fv );
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++ )
		*(oy++) = _mm256_mul_ps ( *(y++), fv );
#endif
}

void rvm_soa_vec4_mad_x ( rvm_soa_vec1* out, const rvm_soa_vec4* v, float multiply, float add )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x;
	float *ox = out->x;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(ox++) = *(x++) * multiply + add;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 mv  = _mm_set1_ps ( multiply ), av = _mm_set1_ps ( add );
	__m128 *x  = (__m128*)v->x;
	__m128 *ox = (__m128*)out->x;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(ox++) = _mm_add_ps ( _mm_mul_ps ( *(x++), mv ), av );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 mv  = _mm256_set1_ps ( multiply ), av = _mm256_set1_ps ( add );
	__m256 *x  = (__m256*)v->x;
	__m256 *ox = (__m256*)out->x;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++ )
		*(ox++) = _mm256_add_ps ( _mm256_mul_ps ( *(x++), mv ), av );
#endif
}

void rvm_soa_vec4_mad_y ( rvm_soa_vec1* out, const rvm_soa_vec4* v, float multiply, float add )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *y = v->y;
	float *oy = out->x;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(oy++) = *(y++) * multiply + add;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 mv  = _mm_set1_ps ( multiply ), av = _mm_set1_ps ( add );
	__m128 *y  = (__m128*)v->y;
	__m128 *oy = (__m128*)out->x;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(oy++) = _mm_add_ps ( _mm_mul_ps ( *(y++), mv ), av );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 mv  = _mm256_set1_ps ( multiply ), av = _mm256_set1_ps ( add );
	__m256 *y  = (__m256*)v->y;
	__m256 *oy = (__m256*)out->x;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++ )
		*(oy++) = _mm256_add_ps ( _mm256_mul_ps ( *(y++), mv ), av );
#endif
}

void rvm_soa_vec4_mad_xy ( rvm_soa_vec2* out, const rvm_soa_vec4* v, float multiply, float add )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x, *y = v->y;
	float *ox = out->x, *oy = out->y;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(ox++) = *(x++) * multiply + add;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(oy++) = *(y++) * multiply + add;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 mv  = _mm_set1_ps ( multiply ), av = _mm_set1_ps ( add );
	__m128 *x  = (__m128*)v->x,   *y  = (__m128*)v->y;
	__m128 *ox = (__m128*)out->x, *oy = (__m128*)out->y;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(ox++) = _mm_add_ps ( _mm_mul_ps ( *(x++), mv ), av );
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(oy++) = _mm_add_ps ( _mm_mul_ps ( *(y++), mv ), av );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 mv  = _mm256_set1_ps ( multiply ), av = _mm256_set1_ps ( add );
	__m256 *x  = (__m256*)v->x,   *y  = (__m256*)v->y;
	__m256 *ox = (__m256*)out->x, *oy = (__m256*)out->y;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(ox++) = _mm256_add_ps ( _mm256_mul_ps ( *(x++), mv ), av );
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(oy++) = _mm256_add_ps ( _mm256_mul_ps ( *(y++), mv ), av );
#endif
}

void rvm_soa_vec4_overwrite_mul_x_float ( rvm_soa_vec4* v, float f )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x;
	for ( uint32_t i = 0; i < v->vectorCount; i++, x++ )
		*(x) = *(x) * f;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 fv  = _mm_set1_ps ( f );
	__m128 *x  = (__m128*)v->x;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, x++ )
		*(x) = _mm_mul_ps ( *(x), fv );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 fv  = _mm256_set1_ps ( f );
	__m256 *x  = (__m256*)v->x;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, x++ )
		*(x) = _mm256_mul_ps ( *(x), fv );
#endif
}

void rvm_soa_vec4_overwrite_mul_y_float ( rvm_soa_vec4* v, float f )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *y = v->y;
	for ( uint32_t i = 0; i < v->vectorCount; i++, y++ )
		*(y) = *(y) * f;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 fv  = _mm_set1_ps ( f );
	__m128 *y  = (__m128*)v->y;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, y++ )
		*(y) = _mm_mul_ps ( *(y), fv );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 fv  = _mm256_set1_ps ( f );
	__m256 *y  = (__m256*)v->y;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, y++ )
		*(y) = _mm256_mul_ps ( *(y), fv );
#endif
}

void rvm_soa_vec4_overwrite_mul_xy_float ( rvm_soa_vec4* v, float f )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x, *y = v->y;
	for ( uint32_t i = 0; i < v->vectorCount; i++, x++ )
		*(x) = *(x) * f;
	for ( uint32_t i = 0; i < v->vectorCount; i++, y++ )
		*(y) = *(y) * f;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 fv  = _mm_set1_ps ( f );
	__m128 *x  = (__m128*)v->x, *y  = (__m128*)v->y;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, x++ )
		*(x) = _mm_mul_ps ( *(x), fv );
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, y++ )
		*(y) = _mm_mul_ps ( *(y), fv );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 fv  = _mm256_set1_ps ( f );
	__m256 *x  = (__m256*)v->x, *y  = (__m256*)v->y;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, x++ )
		*(x) = _mm256_mul_ps ( *(x), fv );
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, y++ )
		*(y) = _mm256_mul_ps ( *(y), fv );
#endif
}

void rvm_soa_vec4_overwrite_mad_x ( rvm_soa_vec4* v, float multiply, float add )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x;
	for ( uint32_t i = 0; i < v->vectorCount; i++, x++ )
		*(x) = *(x) * multiply + add;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 mv  = _mm_set1_ps ( multiply ), av = _mm_set1_ps ( add );
	__m128 *x  = (__m128*)v->x;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, x++ )
		*(x) = _mm_add_ps ( _mm_mul_ps ( *(x), mv ), av );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 mv  = _mm256_set1_ps ( multiply ), av = _mm256_set1_ps ( add );
	__m256 *x  = (__m256*)v->x;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, x++ )
		*(x) = _mm256_add_ps ( _mm256_mul_ps ( *(x), mv ), av );
#endif
}

void rvm_soa_vec4_overwrite_mad_y ( rvm_soa_vec4* v, float multiply, float add )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *y = v->y;
	for ( uint32_t i = 0; i < v->vectorCount; i++, y++ )
		*(y) = *(y) * multiply + add;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 mv  = _mm_set1_ps ( multiply ), av = _mm_set1_ps ( add );
	__m128 *y  = (__m128*)v->y;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, y++ )
		*(y) = _mm_add_ps ( _mm_mul_ps ( *(y), mv ), av );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 mv  = _mm256_set1_ps ( multiply ), av = _mm256_set1_ps ( add );
	__m256 *y  = (__m256*)v->y;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, y++ )
		*(y) = _mm256_add_ps ( _mm256_mul_ps ( *(y), mv ), av );
#endif
}

void rvm_soa_vec4_overwrite_mad_xy ( rvm_soa_vec4* v, float multiply, float add )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x, *y = v->y;
	for ( uint32_t i = 0; i < v->vectorCount; i++, x++ )
		*(x) = *(x) * multiply + add;
	for ( uint32_t i = 0; i < v->vectorCount; i++, y++ )
		*(y) = *(y) * multiply + add;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 mv  = _mm_set1_ps ( multiply ), av = _mm_set1_ps ( add );
	__m128 *x  = (__m128*)v->x, *y  = (__m128*)v->y;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, x++ )
		*(x) = _mm_add_ps ( _mm_mul_ps ( *(x), mv ), av );
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, y++ )
		*(y) = _mm_add_ps ( _mm_mul_ps ( *(y), mv ), av );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 mv  = _mm256_set1_ps ( multiply ), av = _mm256_set1_ps ( add );
	__m256 *x  = (__m256*)v->x, *y  = (__m256*)v->y;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, x++ )
		*(x) = _mm256_add_ps ( _mm256_mul_ps ( *(x), mv ), av );
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, y++ )
		*(y) = _mm256_add_ps ( _mm256_mul_ps ( *(y), mv ), av );
#endif
}

void rvm_soa_vec4_div_xy_w ( rvm_soa_vec2* out, const rvm_soa_vec4* v )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x, *y = v->y, *w = v->w;
	float *ox = out->x, *oy = out->y;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(ox++) = *(x++) / *(w++);
	w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(oy++) = *(y++) / *(w++);
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 *x  = (__m128*)v->x,   *y  = (__m128*)v->y, *w = (__m128*)v->w;
	__m128 *ox = (__m128*)out->x, *oy = (__m128*)out->y;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(ox++) = _mm_div_ps ( *(x++), *(w++) );
	w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++ )
		*(oy++) = _mm_div_ps ( *(y++), *(w++) );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 *x  = (__m256*)v->x,   *y  = (__m256*)v->y, *w = (__m256*)v->w;
	__m256 *ox = (__m256*)out->x, *oy = (__m256*)out->y;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++ )
		*(ox++) = _mm256_div_ps ( *(x++), *(w++) );
	w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++ )
		*(oy++) = _mm256_div_ps ( *(y++), *(w++) );
#endif
}

void rvm_soa_vec4_div_xyz_w ( rvm_soa_vec3* out, const rvm_soa_vec4* v )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x, *y = v->y, *z = v->z, *w = v->w;
	float *ox = out->x, *oy = out->y, *oz = out->z;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(ox++) = *(x++) / *(w++);
	w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(oy++) = *(y++) / *(w++);
	w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(oz++) = *(z++) / *(w++);
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 *x  = (__m128*)v->x,   *y  = (__m128*)v->y,   *z  = (__m128*)v->z, *w = (__m128*)v->w;
	__m128 *ox = (__m128*)out->x, *oy = (__m128*)out->y, *oz = (__m128*)out->z;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(ox++) = _mm_div_ps ( *(x++), *(w++) );
	w = (__m128*)v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(oy++) = _mm_div_ps ( *(y++), *(w++) );
	w = (__m128*)v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++ )
		*(oz++) = _mm_div_ps ( *(z++), *(w++) );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 *x  = (__m256*)v->x,   *y  = (__m256*)v->y,   *z  = (__m256*)v->z, *w = (__m256*)v->w;
	__m256 *ox = (__m256*)out->x, *oy = (__m256*)out->y, *oz = (__m256*)out->z;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++ )
		*(ox++) = _mm256_div_ps ( *(x++), *(w++) );
	w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++ )
		*(oy++) = _mm256_div_ps ( *(y++), *(w++) );
	w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++ )
		*(oz++) = _mm256_div_ps ( *(z++), *(w++) );
#endif
}

void rvm_soa_vec4_overwrite_mul_xy_w ( rvm_soa_vec4* v )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x, *y = v->y, *w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, x++ )
		*(x) = *(x) * *(w++);
	w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, y++ )
		*(y) = *(y) * *(w++);
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 *x = (__m128*)v->x, *y = (__m128*)v->y, *w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, x++ )
		*(x) = _mm_mul_ps ( *(x), *(w++) );
	w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, y++ )
		*(y) = _mm_mul_ps ( *(y), *(w++) );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 *x = (__m256*)v->x, *y = (__m256*)v->y, *w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, x++ )
		*(x) = _mm256_mul_ps ( *(x), *(w++) );
	w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, y++ )
		*(y) = _mm256_mul_ps ( *(y), *(w++) );
#endif
}

void rvm_soa_vec4_overwrite_mul_xyz_w ( rvm_soa_vec4* v )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x, *y = v->y, *z = v->z, *w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, x++ )
		*(x) = *(x) * *(w++);
	w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, y++ )
		*(y) = *(y) * *(w++);
	w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, z++ )
		*(z) = *(z) * *(w++);
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 *x = (__m128*)v->x, *y = (__m128*)v->y, *z = (__m128*)v->z, *w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, x++ )
		*(x) = _mm_mul_ps ( *(x), *(w++) );
	w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, y++ )
		*(y) = _mm_mul_ps ( *(y), *(w++) );
	w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, z++ )
		*(z) = _mm_mul_ps ( *(z), *(w++) );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 *x = (__m256*)v->x, *y = (__m256*)v->y, *z = (__m256*)v->z, *w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, x++ )
		*(x) = _mm256_mul_ps ( *(x), *(w++) );
	w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, y++ )
		*(y) = _mm256_mul_ps ( *(y), *(w++) );
	w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, z++ )
		*(z) = _mm256_mul_ps ( *(z), *(w++) );
#endif
}

void rvm_soa_vec4_overwrite_div_xy_w ( rvm_soa_vec4* v )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x, *y = v->y, *w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, x++ )
		*(x) = *(x) / *(w++);
	w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, y++ )
		*(y) = *(y) / *(w++);
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 *x = (__m128*)v->x, *y = (__m128*)v->y, *w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, x++ )
		*(x) = _mm_div_ps ( *(x), *(w++) );
	w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, y++ )
		*(y) = _mm_div_ps ( *(y), *(w++) );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 *x = (__m256*)v->x, *y = (__m256*)v->y, *w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, x++ )
		*(x) = _mm256_div_ps ( *(x), *(w++) );
	w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, y++ )
		*(y) = _mm256_div_ps ( *(y), *(w++) );
#endif
}

void rvm_soa_vec4_overwrite_div_xyz_w ( rvm_soa_vec4* v )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *x = v->x, *y = v->y, *z = v->z, *w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, x++ )
		*(x) = *(x) / *(w++);
	w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, y++ )
		*(y) = *(y) / *(w++);
	w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, z++ )
		*(z) = *(z) / *(w++);
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 *x = (__m128*)v->x, *y = (__m128*)v->y, *z = (__m128*)v->z, *w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, x++ )
		*(x) = _mm_div_ps ( *(x), *(w++) );
	w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, y++ )
		*(y) = _mm_div_ps ( *(y), *(w++) );
	w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, z++ )
		*(z) = _mm_div_ps ( *(z), *(w++) );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 *x = (__m256*)v->x, *y = (__m256*)v->y, *z = (__m256*)v->z, *w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, x++ )
		*(x) = _mm256_div_ps ( *(x), *(w++) );
	w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, y++ )
		*(y) = _mm256_div_ps ( *(y), *(w++) );
	w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, z++ )
		*(z) = _mm256_div_ps ( *(z), *(w++) );
#endif
}

void rvm_soa_vec4_overwrite_rcp_w ( rvm_soa_vec4* v )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float *w = v->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, w++ )
		*(w) = 1.0f / *(w);
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	__m128 one = _mm_set1_ps ( 1.0f );
	__m128 *w = (__m128*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+3)/4; i++, w++ )
		// _mm_rcp_ps has problems with precision!
		*(w) = _mm_div_ps ( one, *w ); ;//_mm_rcp_ps ( *(w) );
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	__m256 one = _mm256_set1_ps ( 1.0f );
	__m256 *w = (__m256*)v->w;
	for ( uint32_t i = 0; i < (v->vectorCount+7)/8; i++, w++ )
		// _mm256_rcp_ps has problems with precision!
		*(w) = _mm256_div_ps ( one, *w ); ;//_mm256_rcp_ps ( *(w) );
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void rvm_aos_vec3_normalize ( rvm_aos_vec3* out, const rvm_aos_vec3* v )
{
	float oneOverLength = 1.0f / rvm_aos_vec3_length ( v );
	*out = (rvm_aos_vec3){ .x = oneOverLength * v->x, .y = oneOverLength * v->y, .z = oneOverLength * v->z };
}

float rvm_aos_vec3_length ( const rvm_aos_vec3* v )
{
	return sqrtf ( (v->x * v->x) + (v->y * v->y) + (v->z * v->z) );
}

float rvm_aos_vec3_sqlength ( const rvm_aos_vec3* v )
{
	return (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
}

float rvm_aos_vec3_dot_aos_vec3 ( const rvm_aos_vec3* v1, const rvm_aos_vec3* v2 )
{
	return (v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z);
}

void rvm_aos_vec3_cross_aos_vec3 ( rvm_aos_vec3* out, const rvm_aos_vec3* v1, const rvm_aos_vec3* v2 )
{
	*out = (rvm_aos_vec3){
		v1->y * v2->z - v1->z * v2->y,
		v1->z * v2->x - v1->x * v2->z,
		v1->x * v2->y - v1->y * v2->x
	};
}

void rvm_aos_vec3_add_aos_vec3 ( rvm_aos_vec3* out, const rvm_aos_vec3* v1, const rvm_aos_vec3* v2 )
{
	*out = (rvm_aos_vec3){ .x = v1->x + v2->x, .y = v1->y + v2->y, .z = v1->z + v2->z };
}

void rvm_aos_vec3_sub_aos_vec3 ( rvm_aos_vec3* out, const rvm_aos_vec3* v1, const rvm_aos_vec3* v2 )
{
	*out = (rvm_aos_vec3){ .x = v1->x - v2->x, .y = v1->y - v2->y, .z = v1->z - v2->z };
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

float rvm_aos_mat4_max ( const rvm_aos_mat4* m )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float max = -FLT_MAX;
	const float* c = m->cells;
	for ( uint32_t i = 0; i < 16; i++, c++ )
	{
		if ( *c > max )
			max = *c;
	}
	return max;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	//1 6 5 2 | min: 1
	//8 2 5 1 | max: 8
	//7 4 3 8 |
	//1 5 6 3 |
	//
	//1 6 5 2     7 4 3 8
	//8 2 5 1     1 5 6 3 
	//------- m   ------- m
	//8 6 5 2     7 5 6 8
	//7 5 6 8 <-
	//------- m
	//8 6 6 8
	//8 6 6 8 s(wzyx)
	//------- m
	//8 6 6 8 // w == x, z == y
	//6 8 8 6 s(yxwz)
	//------- m
	//8 8 8 8 // x == y, x == z, x == w
	//
	//5 min/max, 2 shuffle
	float* c = (float*)m->cells;

	__m128 r0 = _mm_loadu_ps ( m->cells );
	__m128 r1 = _mm_loadu_ps ( m->cells + 4 );
	__m128 r2 = _mm_loadu_ps ( m->cells + 8 );
	__m128 r3 = _mm_loadu_ps ( m->cells + 12 );

	r0 = _mm_max_ps ( r0, r1 );
	r1 = _mm_max_ps ( r2, r3 );

	r0 = _mm_max_ps ( r0, r1 );
	r1 = _mm_shuffle_ps ( r0, r0, _MM_SHUFFLE(0,1,2,3) );

	r0 = _mm_max_ps ( r0, r1 );
	r1 = _mm_shuffle_ps ( r0, r0, _MM_SHUFFLE(2,3,0,1) );

	r0 = _mm_max_ps ( r0, r1 );
	return *(float*)&r0;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	//float* c = (float*)m->cells;

	// x1 y1 z1 w1 x2 y2 z2 w2
	//  0  1  2  3  4  5  6  7 //  x1 y1 z1 w1 x2 y2 z2 w2
	//  4  5  6  7  0  1  2  3 //  x2 y2 z2 w2 x1 y1 z1 w1

	
	// 4 5 6 7 // x y z w
	// 6 7 4 5 // z w x y

	// 6 7 // x y
	// 7 6 

	union
	{
		__m256 r0;
		struct { __m128 r00, r01; };
	};
	__m256 r1;

	r0 = _mm256_loadu_ps ( m->cells );
	r1 = _mm256_loadu_ps ( m->cells + 8 );

	r0 = _mm256_max_ps ( r0, r1 );

	r1 = _mm256_permute2f128_ps ( r0, r0, 0x01 );
	r0 = _mm256_max_ps ( r0, r1 );

	r01 = _mm_shuffle_ps ( r00, r00, _MM_SHUFFLE(0,1,2,3) );

	r00 = _mm_max_ps ( r00, r01 );
	r01 = _mm_shuffle_ps ( r00, r00, _MM_SHUFFLE(2,3,0,1) );

	r00 = _mm_max_ps ( r00, r01 );
	return *(float*)&r00;
#endif
}

float rvm_aos_mat4_min ( const rvm_aos_mat4* m )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float min = FLT_MAX;
	const float* c = m->cells;
	for ( uint32_t i = 0; i < 16; i++, c++ )
	{
		if ( *c < min )
			min = *c;
	}
	return min;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	//1 6 5 2 | min: 1
	//8 2 5 1 | max: 8
	//7 4 3 8 |
	//1 5 6 3 |
	//
	//1 6 5 2     7 4 3 8
	//8 2 5 1     1 5 6 3 
	//------- m   ------- m
	//8 6 5 2     7 5 6 8
	//7 5 6 8 <-
	//------- m
	//8 6 6 8
	//8 6 6 8 s(wzyx)
	//------- m
	//8 6 6 8 // w == x, z == y
	//6 8 8 6 s(yxwz)
	//------- m
	//8 8 8 8 // x == y, x == z, x == w
	//
	//5 min/max, 2 shuffle
	float* c = (float*)m->cells;

	__m128 r0 = _mm_loadu_ps ( m->cells );
	__m128 r1 = _mm_loadu_ps ( m->cells + 4 );
	__m128 r2 = _mm_loadu_ps ( m->cells + 8 );
	__m128 r3 = _mm_loadu_ps ( m->cells + 12 );

	r0 = _mm_min_ps ( r0, r1 );
	r1 = _mm_min_ps ( r2, r3 );

	r0 = _mm_min_ps ( r0, r1 );
	r1 = _mm_shuffle_ps ( r0, r0, _MM_SHUFFLE(0,1,2,3) );

	r0 = _mm_min_ps ( r0, r1 );
	r1 = _mm_shuffle_ps ( r0, r0, _MM_SHUFFLE(2,3,0,1) );

	r0 = _mm_min_ps ( r0, r1 );
	return *(float*)&r0;
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	union
	{
		__m256 r0;
		struct { __m128 r00, r01; };
	};
	__m256 r1;

	r0 = _mm256_loadu_ps ( m->cells );
	r1 = _mm256_loadu_ps ( m->cells + 8 );

	r0 = _mm256_min_ps ( r0, r1 );

	r1 = _mm256_permute2f128_ps ( r0, r0, 0x01 );
	r0 = _mm256_min_ps ( r0, r1 );

	r01 = _mm_shuffle_ps ( r00, r00, _MM_SHUFFLE(0,1,2,3) );

	r00 = _mm_min_ps ( r00, r01 );
	r01 = _mm_shuffle_ps ( r00, r00, _MM_SHUFFLE(2,3,0,1) );

	r00 = _mm_min_ps ( r00, r01 );
	return *(float*)&r00;
#endif
}

rvm_aos_vec3 rvm_soa_vec3_min_xyz ( const rvm_soa_vec3* v )
{
	rvm_aos_vec3 out;
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	for ( uint32_t cell = 0; cell < 3; cell++ )
	{
		float* co = &(out.cells[cell]);
		float* ci = v->cells[cell];
		*co = FLT_MAX;
		for ( uint32_t i = 0; i < v->vectorCount; i++, ci++ )
		{
			if ( *ci < *co )
				*co = *ci;
		}
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	const uint32_t passCount  = (v->vectorCount)/4;
	const uint32_t undefCount = v->vectorCount - 4 * passCount;
	const __m128 undefMask = _mm_cmplt_ps ( _mm_set_ps ( 3, 2, 1, 0 ), _mm_set1_ps ( undefCount - 0.1f ) );
	float* o = &(out.x);
	for ( uint32_t cell = 0; cell < 3; cell++, o++ )
	{
		__m128 min = _mm_set1_ps ( FLT_MAX );
		__m128* ci = (__m128*)v->cells[cell];
		
		for ( uint32_t i = 0; i < passCount; i++, ci++ )
		{
			min = _mm_min_ps ( min, *ci );
		}

		// If N is not aligned by 4, the (up to) 3 last floats for each cell can be undefined
		__m128 undef = _mm_or_ps ( _mm_and_ps ( undefMask, *ci ), _mm_andnot_ps ( undefMask, _mm_set1_ps ( FLT_MAX ) ) );
		min = _mm_min_ps ( min, undef );

		min = _mm_min_ps ( min, _mm_shuffle_ps ( min, min, _MM_SHUFFLE(0,1,2,3) ) );
		min = _mm_min_ps ( min, _mm_shuffle_ps ( min, min, _MM_SHUFFLE(2,3,0,1) ) );

		*o = *(float*)&min;
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	const uint32_t passCount  = (v->vectorCount)/8;
	const uint32_t undefCount = v->vectorCount - 8 * passCount;
	const __m256 undefMask = _mm256_cmp_ps ( _mm256_set_ps ( 7, 6, 5, 4, 3, 2, 1, 0 ), _mm256_set1_ps ( undefCount - 0.1f ), _CMP_LT_OQ ); 
	float* o = &(out.x);
	for ( uint32_t cell = 0; cell < 3; cell++, o++ )
	{
		union
		{
			__m256 min;
			struct { __m128 min0, min1; };
		};
		       min = _mm256_set1_ps ( FLT_MAX );
		__m256* ci = (__m256*)v->cells[cell];
		
		for ( uint32_t i = 0; i < passCount; i++, ci++ )
		{
			min = _mm256_min_ps ( min, *ci );
		}

		// If N is not aligned by 4, the (up to) 3 last floats for each cell can be undefined
		__m256 undef = _mm256_or_ps ( _mm256_and_ps ( undefMask, *ci ), _mm256_andnot_ps ( undefMask, _mm256_set1_ps ( FLT_MAX ) ) );

		min = _mm256_min_ps ( min, undef );
		min = _mm256_min_ps ( min, _mm256_permute2f128_ps ( min, min, 0x01 ) );

		min0 = _mm_min_ps ( min0, _mm_shuffle_ps ( min0, min0, _MM_SHUFFLE(0,1,2,3) ) );
		min0 = _mm_min_ps ( min0, _mm_shuffle_ps ( min0, min0, _MM_SHUFFLE(2,3,0,1) ) );

		*o = *(float*)&min0;
	}
#endif
	return out;
}

rvm_aos_vec3 rvm_soa_vec3_max_xyz ( const rvm_soa_vec3* v )
{
	rvm_aos_vec3 out;
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	for ( uint32_t cell = 0; cell < 3; cell++ )
	{
		float* co = &(out.cells[cell]);
		float* ci = v->cells[cell];
		*co = -FLT_MAX;
		for ( uint32_t i = 0; i < v->vectorCount; i++, ci++ )
		{
			if ( *ci > *co )
				*co = *ci;
		}
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	const uint32_t passCount  = (v->vectorCount)/4;
	const uint32_t undefCount = v->vectorCount - 4 * passCount;
	const __m128 undefMask = _mm_cmplt_ps ( _mm_set_ps ( 3, 2, 1, 0 ), _mm_set1_ps ( undefCount - 0.1f ) );
	float* o = &(out.x);
	for ( uint32_t cell = 0; cell < 3; cell++, o++ )
	{
		__m128 max = _mm_set1_ps ( -FLT_MAX );
		__m128* ci = (__m128*)v->cells[cell];
		
		for ( uint32_t i = 0; i < passCount; i++, ci++ )
		{
			max = _mm_max_ps ( max, *ci );
		}

		// If N is not aligned by 4, the (up to) 3 last floats for each cell can be undefined
		__m128 undef = _mm_or_ps ( _mm_and_ps ( undefMask, *ci ), _mm_andnot_ps ( undefMask, _mm_set1_ps ( -FLT_MAX ) ) );
		max = _mm_max_ps ( max, undef );

		max = _mm_max_ps ( max, _mm_shuffle_ps ( max, max, _MM_SHUFFLE(0,1,2,3) ) );
		max = _mm_max_ps ( max, _mm_shuffle_ps ( max, max, _MM_SHUFFLE(2,3,0,1) ) );

		*o = *(float*)&max;
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	const uint32_t passCount  = (v->vectorCount)/8;
	const uint32_t undefCount = v->vectorCount - 8 * passCount;
	const __m256 undefMask = _mm256_cmp_ps ( _mm256_set_ps ( 7, 6, 5, 4, 3, 2, 1, 0 ), _mm256_set1_ps ( undefCount - 0.1f ), _CMP_LT_OQ ); 
	float* o = &(out.x);
	for ( uint32_t cell = 0; cell < 3; cell++, o++ )
	{
		union
		{
			__m256 max;
			struct { __m128 max0, max1; };
		};
		       max = _mm256_set1_ps ( -FLT_MAX );
		__m256* ci = (__m256*)v->cells[cell];
		
		for ( uint32_t i = 0; i < passCount; i++, ci++ )
		{
			max = _mm256_max_ps ( max, *ci );
		}

		// If N is not aligned by 4, the (up to) 3 last floats for each cell can be undefined
		__m256 undef = _mm256_or_ps ( _mm256_and_ps ( undefMask, *ci ), _mm256_andnot_ps ( undefMask, _mm256_set1_ps ( -FLT_MAX ) ) );

		max = _mm256_max_ps ( max, undef );
		max = _mm256_max_ps ( max, _mm256_permute2f128_ps ( max, max, 0x01 ) );

		max0 = _mm_max_ps ( max0, _mm_shuffle_ps ( max0, max0, _MM_SHUFFLE(0,1,2,3) ) );
		max0 = _mm_max_ps ( max0, _mm_shuffle_ps ( max0, max0, _MM_SHUFFLE(2,3,0,1) ) );

		*o = *(float*)&max0;
	}
#endif
	return out;
}


rvm_aos_mat3 rvm_aos_mat3_mul_aos_mat3 ( const rvm_aos_mat3* m1, const rvm_aos_mat3* m2 )
{
	rvm_aos_mat3 out;
#define MUL_OPS(r,c) out.rows[r][c] = (m1->rows[0][c] * m2->rows[r][0] + m1->rows[1][c] * m2->rows[r][1] + m1->rows[2][c] * m2->rows[r][2])
	MUL_OPS ( 0, 0 ), MUL_OPS ( 0, 1 ), MUL_OPS ( 0, 2 );
	MUL_OPS ( 1, 0 ), MUL_OPS ( 1, 1 ), MUL_OPS ( 1, 2 );
	MUL_OPS ( 2, 0 ), MUL_OPS ( 2, 1 ), MUL_OPS ( 2, 2 );
#undef MUL_OPS
	return out;
}

rvm_aos_vec2 rvm_aos_mat3_mul_aos_vec2z0 ( const rvm_aos_mat3* m, const rvm_aos_vec2* v )
{
	rvm_aos_vec2 out;
	float x = v->x, y = v->y;

	out.x = m->rows[0][0] * (x) + m->rows[1][0] * (y);
	out.y = m->rows[0][1] * (x) + m->rows[1][1] * (y);
	return out;
}

rvm_aos_vec2 rvm_aos_mat3_mul_aos_vec2z1 ( const rvm_aos_mat3* m, const rvm_aos_vec2* v )
{
	float x = v->x, y = v->y;
	
	return (rvm_aos_vec2){
		.x = m->rows[0][0] * (x) + m->rows[1][0] * (y) + m->rows[2][0],
		.y = m->rows[0][1] * (x) + m->rows[1][1] * (y) + m->rows[2][1],
	};
}

rvm_aos_vec3 rvm_aos_mat3_mul_aos_vec2z1_out_vec3 ( const rvm_aos_mat3* m, const rvm_aos_vec2* v )
{
	float x = v->x, y = v->y;
	
	return (rvm_aos_vec3){
		.x = m->rows[0][0] * (x) + m->rows[1][0] * (y) + m->rows[2][0],
		.y = m->rows[0][1] * (x) + m->rows[1][1] * (y) + m->rows[2][1],
		.z = m->rows[0][2] * (x) + m->rows[1][2] * (y) + m->rows[2][2],
	};
}

rvm_aos_vec3 rvm_aos_mat3_mul_aos_vec3 ( const rvm_aos_mat3* m, const rvm_aos_vec3* v )
{
	float x = v->x, y = v->y, z = v->z;
	
	return (rvm_aos_vec3){
		.x = m->rows[0][0] * (x) + m->rows[1][0] * (y) + m->rows[2][0] * (z),
		.y = m->rows[0][1] * (x) + m->rows[1][1] * (y) + m->rows[2][1] * (z),
		.z = m->rows[0][2] * (x) + m->rows[1][2] * (y) + m->rows[2][2] * (z),
	};
}


rvm_aos_mat4 rvm_aos_mat4_mul_aos_mat4 ( const rvm_aos_mat4* m1, const rvm_aos_mat4* m2 )
{
	rvm_aos_mat4 out;
#define MUL_OPS(r,c) out.rows[r][c] = (m1->rows[0][c] * m2->rows[r][0] + m1->rows[1][c] * m2->rows[r][1] + m1->rows[2][c] * m2->rows[r][2] + m1->rows[3][c] * m2->rows[r][3])
	MUL_OPS ( 0, 0 ), MUL_OPS ( 0, 1 ), MUL_OPS ( 0, 2 ), MUL_OPS ( 0, 3 );
	MUL_OPS ( 1, 0 ), MUL_OPS ( 1, 1 ), MUL_OPS ( 1, 2 ), MUL_OPS ( 1, 3 );
	MUL_OPS ( 2, 0 ), MUL_OPS ( 2, 1 ), MUL_OPS ( 2, 2 ), MUL_OPS ( 2, 3 );
	MUL_OPS ( 3, 0 ), MUL_OPS ( 3, 1 ), MUL_OPS ( 3, 2 ), MUL_OPS ( 3, 3 );
#undef MUL_OPS
	return out;
}

rvm_aos_vec3 rvm_aos_mat4_mul_aos_vec3w0 ( const rvm_aos_mat4* m, const rvm_aos_vec3* v )
{
	float x = v->x, y = v->y, z = v->z;

	return (rvm_aos_vec3){
		.x = m->rows[0][0] * (x) + m->rows[1][0] * (y) + m->rows[2][0] * (z),
		.y = m->rows[0][1] * (x) + m->rows[1][1] * (y) + m->rows[2][1] * (z),
		.z = m->rows[0][2] * (x) + m->rows[1][2] * (y) + m->rows[2][2] * (z),
	};
}

rvm_aos_vec3 rvm_aos_mat4_mul_aos_vec3w1 ( const rvm_aos_mat4* m, const rvm_aos_vec3* v )
{
	float x = v->x, y = v->y, z = v->z;
	
	return (rvm_aos_vec3){
		.x = m->rows[0][0] * (x) + m->rows[1][0] * (y) + m->rows[2][0] * (z) + m->rows[3][0],
		.y = m->rows[0][1] * (x) + m->rows[1][1] * (y) + m->rows[2][1] * (z) + m->rows[3][1],
		.z = m->rows[0][2] * (x) + m->rows[1][2] * (y) + m->rows[2][2] * (z) + m->rows[3][2],
	};
}

rvm_aos_vec4 rvm_aos_mat4_mul_aos_vec3w1_out_vec4 ( const rvm_aos_mat4* m, const rvm_aos_vec3* v )
{
	float x = v->x, y = v->y, z = v->z;
	
	return (rvm_aos_vec4){
		.x = m->rows[0][0] * (x) + m->rows[1][0] * (y) + m->rows[2][0] * (z) + m->rows[3][0],
		.y = m->rows[0][1] * (x) + m->rows[1][1] * (y) + m->rows[2][1] * (z) + m->rows[3][1],
		.z = m->rows[0][2] * (x) + m->rows[1][2] * (y) + m->rows[2][2] * (z) + m->rows[3][2],
		.w = m->rows[0][3] * (x) + m->rows[1][3] * (y) + m->rows[2][3] * (z) + m->rows[3][3],
	};
}

rvm_aos_vec4 rvm_aos_mat4_mul_aos_vec4 ( const rvm_aos_mat4* m, const rvm_aos_vec4* v )
{
	float x = v->x, y = v->y, z = v->z, w = v->w;
	
	return (rvm_aos_vec4){
		.x = m->rows[0][0] * (x) + m->rows[1][0] * (y) + m->rows[2][0] * (z) + m->rows[3][0] * (w),
		.y = m->rows[0][1] * (x) + m->rows[1][1] * (y) + m->rows[2][1] * (z) + m->rows[3][1] * (w),
		.z = m->rows[0][2] * (x) + m->rows[1][2] * (y) + m->rows[2][2] * (z) + m->rows[3][2] * (w),
		.w = m->rows[0][3] * (x) + m->rows[1][3] * (y) + m->rows[2][3] * (z) + m->rows[3][3] * (w),
	};
}


void rvm_aos_mat4_mul_soa_mat4 ( rvm_soa_mat4* out, const rvm_aos_mat4* m1, const rvm_soa_mat4* m2 )
{
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	assert ( out->matrixCount == m2->matrixCount );
	float celldata1[4], *cellptr2[4], *outptr;

	for ( uint32_t r = 0; r < 4; r++ )
	{
		for ( uint32_t c = 0; c < 4; c++ )
		{
			outptr = out->rows[r][c];
			for ( uint32_t i = 0; i < 4; i++ )
				celldata1[i] = m1->rows[i][c];
			for ( uint32_t i = 0; i < 4; i++ )
				cellptr2[i] = m2->rows[r][i];
			for ( uint32_t i = 0; i < m2->matrixCount; i++ )
				*(outptr++) = (celldata1[0]) * *(cellptr2[0]++) + (celldata1[1]) * *(cellptr2[1]++) + (celldata1[2]) * *(cellptr2[2]++) + (celldata1[3]) * *(cellptr2[3]++);
		}
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	assert ( out->matrixCount == m2->matrixCount );

	__m128 celldata1[4], *cellptr2[4], *outptr;

	for ( uint32_t r = 0; r < 4; r++ )
	{
		for ( uint32_t c = 0; c < 4; c++ )
		{
			outptr = (__m128*)( out->rows[r][c] );
			for ( uint32_t i = 0; i < 4; i++ )
				celldata1[i] = _mm_set1_ps ( m1->rows[i][c] );
			for ( uint32_t i = 0; i < 4; i++ )
				cellptr2[i] = (__m128*) ( m2->rows[r][i] );
			for ( uint32_t i = 0; i < (m2->matrixCount + 3) / 4; i++ )
				*(outptr++) = _mm_add_ps ( _mm_add_ps ( _mm_mul_ps ( (celldata1[0]), *(cellptr2[0]++) ), _mm_mul_ps ( (celldata1[1]), *(cellptr2[1]++) ) ), _mm_add_ps ( _mm_mul_ps ( (celldata1[2]), *(cellptr2[2]++) ), _mm_mul_ps ( (celldata1[3]), *(cellptr2[3]++) ) ) );
		}
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	assert ( out->matrixCount == m2->matrixCount );

	__m256 celldata1[4], *cellptr2[4], *outptr;

	for ( uint32_t r = 0; r < 4; r++ )
	{
		for ( uint32_t c = 0; c < 4; c++ )
		{
			outptr = (__m256*)( out->rows[r][c] );
			for ( uint32_t i = 0; i < 4; i++ )
				celldata1[i] = _mm256_set1_ps ( m1->rows[i][c] );
			for ( uint32_t i = 0; i < 4; i++ )
				cellptr2[i] = (__m256*) ( m2->rows[r][i] );
			for ( uint32_t i = 0; i < (m2->matrixCount + 7) / 8; i++ )
				*(outptr++) = _mm256_add_ps ( _mm256_add_ps ( _mm256_mul_ps ( (celldata1[0]), *(cellptr2[0]++) ), _mm256_mul_ps ( (celldata1[1]), *(cellptr2[1]++) ) ), _mm256_add_ps ( _mm256_mul_ps ( (celldata1[2]), *(cellptr2[2]++) ), _mm256_mul_ps ( (celldata1[3]), *(cellptr2[3]++) ) ) );
		}
	}
#endif
}

void rvm_aos_mat4_mul_soa_vec3w0 ( rvm_soa_vec3* out, const rvm_aos_mat4* m, const rvm_soa_vec3* v )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	for ( uint32_t cell = 0; cell < 3; cell++ )
	{
		float *ix = v->cells[0], *iy = v->cells[1], *iz = v->cells[2];
		float* o = out->cells[cell];
		float md[3] = { m->rows[0][cell], m->rows[1][cell], m->rows[2][cell] };
		for ( uint32_t vec = 0; vec < v->vectorCount; vec++, ix++, iy++, iz++, o++ )
			*o = ( md[0] * *ix ) + ( ( md[1] * *iy ) + ( md[2] * *iz ) );
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	for ( uint32_t cell = 0; cell < 3; cell++ )
	{
		__m128 *ix = (__m128*)v->cells[0], *iy = (__m128*)v->cells[1], *iz = (__m128*)v->cells[2];
		__m128* o = (__m128*)out->cells[cell];
		__m128 md[3] = { _mm_set1_ps ( m->rows[0][cell] ), _mm_set1_ps ( m->rows[1][cell] ), _mm_set1_ps ( m->rows[2][cell] ) };
		for ( uint32_t vec = 0; vec < (v->vectorCount+3)/4; vec++, ix++, iy++, iz++, o++ )
			*o = _mm_add_ps ( _mm_mul_ps ( md[0], *ix ), _mm_add_ps ( _mm_mul_ps ( md[1], *iy ), _mm_mul_ps ( md[2], *iz ) ) );
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	for ( uint32_t cell = 0; cell < 3; cell++ )
	{
		__m256 *ix = (__m256*)v->cells[0], *iy = (__m256*)v->cells[1], *iz = (__m256*)v->cells[2];
		__m256* o = (__m256*)out->cells[cell];
		__m256 md[3] = { _mm256_set1_ps ( m->rows[0][cell] ), _mm256_set1_ps ( m->rows[1][cell] ), _mm256_set1_ps ( m->rows[2][cell] ) };
		for ( uint32_t vec = 0; vec < (v->vectorCount+7)/8; vec++, ix++, iy++, iz++, o++ )
			*o = _mm256_add_ps ( _mm256_mul_ps ( md[0], *ix ), _mm256_add_ps ( _mm256_mul_ps ( md[1], *iy ), _mm256_mul_ps ( md[2], *iz ) ) );
	}
#endif
}

void rvm_aos_mat4_mul_soa_vec3w1 ( rvm_soa_vec3* out, const rvm_aos_mat4* m, const rvm_soa_vec3* v )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	for ( uint32_t cell = 0; cell < 3; cell++ )
	{
		float *ix = v->cells[0], *iy = v->cells[1], *iz = v->cells[2];
		float* o = out->cells[cell];
		float md[4] = { m->rows[0][cell], m->rows[1][cell], m->rows[2][cell], m->rows[3][cell] };
		for ( uint32_t vec = 0; vec < v->vectorCount; vec++, ix++, iy++, iz++, o++ )
			*o = ( ( md[0] * *ix ) + ( md[1] * *iy ) ) + ( ( md[2] * *iz ) + md[3] );
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	for ( uint32_t cell = 0; cell < 3; cell++ )
	{
		__m128 *ix = (__m128*)v->cells[0], *iy = (__m128*)v->cells[1], *iz = (__m128*)v->cells[2];
		__m128* o = (__m128*)out->cells[cell];
		__m128 md[4] = { _mm_set1_ps ( m->rows[0][cell] ), _mm_set1_ps ( m->rows[1][cell] ), _mm_set1_ps ( m->rows[2][cell] ), _mm_set1_ps ( m->rows[3][cell] ) };
		for ( uint32_t vec = 0; vec < (v->vectorCount+3)/4; vec++, ix++, iy++, iz++, o++ )
			*o = _mm_add_ps ( _mm_add_ps ( _mm_mul_ps ( md[0], *ix ), _mm_mul_ps ( md[1], *iy ) ), _mm_add_ps ( _mm_mul_ps ( md[2], *iz ), md[3] ) );
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	for ( uint32_t cell = 0; cell < 3; cell++ )
	{
		__m256 *ix = (__m256*)v->cells[0], *iy = (__m256*)v->cells[1], *iz = (__m256*)v->cells[2];
		__m256* o = (__m256*)out->cells[cell];
		__m256 md[4] = { _mm256_set1_ps ( m->rows[0][cell] ), _mm256_set1_ps ( m->rows[1][cell] ), _mm256_set1_ps ( m->rows[2][cell] ), _mm256_set1_ps ( m->rows[3][cell] ) };
		for ( uint32_t vec = 0; vec < (v->vectorCount+7)/8; vec++, ix++, iy++, iz++, o++ )
			*o = _mm256_add_ps ( _mm256_add_ps ( _mm256_mul_ps ( md[0], *ix ), _mm256_mul_ps ( md[1], *iy ) ), _mm256_add_ps ( _mm256_mul_ps ( md[2], *iz ), md[3] ) );
	}
#endif
}

void rvm_aos_mat4_mul_soa_vec3w1_out_vec4 ( rvm_soa_vec4* out, const rvm_aos_mat4* m, const rvm_soa_vec3* v )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	for ( uint32_t cell = 0; cell < 4; cell++ )
	{
		float *ix = v->cells[0], *iy = v->cells[1], *iz = v->cells[2];
		float* o = out->cells[cell];
		float md[4] = { m->rows[0][cell], m->rows[1][cell], m->rows[2][cell], m->rows[3][cell] };
		for ( uint32_t vec = 0; vec < v->vectorCount; vec++, ix++, iy++, iz++, o++ )
			*o = ( ( md[0] * *ix ) + ( md[1] * *iy ) ) + ( ( md[2] * *iz ) + md[3] );
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	for ( uint32_t cell = 0; cell < 4; cell++ )
	{
		__m128 *ix = (__m128*)v->cells[0], *iy = (__m128*)v->cells[1], *iz = (__m128*)v->cells[2];
		__m128* o = (__m128*)out->cells[cell];
		__m128 md[4] = { _mm_set1_ps ( m->rows[0][cell] ), _mm_set1_ps ( m->rows[1][cell] ), _mm_set1_ps ( m->rows[2][cell] ), _mm_set1_ps ( m->rows[3][cell] ) };
		for ( uint32_t vec = 0; vec < (v->vectorCount+3)/4; vec++, ix++, iy++, iz++, o++ )
			*o = _mm_add_ps ( _mm_add_ps ( _mm_mul_ps ( md[0], *ix ), _mm_mul_ps ( md[1], *iy ) ), _mm_add_ps ( _mm_mul_ps ( md[2], *iz ), md[3] ) );
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	for ( uint32_t cell = 0; cell < 4; cell++ )
	{
		__m256 *ix = (__m256*)v->cells[0], *iy = (__m256*)v->cells[1], *iz = (__m256*)v->cells[2];
		__m256* o = (__m256*)out->cells[cell];
		__m256 md[4] = { _mm256_set1_ps ( m->rows[0][cell] ), _mm256_set1_ps ( m->rows[1][cell] ), _mm256_set1_ps ( m->rows[2][cell] ), _mm256_set1_ps ( m->rows[3][cell] ) };
		for ( uint32_t vec = 0; vec < (v->vectorCount+7)/8; vec++, ix++, iy++, iz++, o++ )
			*o = _mm256_add_ps ( _mm256_add_ps ( _mm256_mul_ps ( md[0], *ix ), _mm256_mul_ps ( md[1], *iy ) ), _mm256_add_ps ( _mm256_mul_ps ( md[2], *iz ), md[3] ) );
	}
#endif
}

void rvm_aos_mat4_mul_soa_vec4 ( rvm_soa_vec4* out, const rvm_aos_mat4* m, const rvm_soa_vec4* v )
{
	assert ( out->vectorCount >= v->vectorCount );
#if RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_NONE
	float* xptr = v->x, *yptr = v->y, *zptr = v->z, *wptr = v->w;
	float* xptro = out->x, *yptro = out->y, *zptro = out->z, *wptro = out->w;
	for ( uint32_t i = 0; i < v->vectorCount; i++, xptr++, yptr++, zptr++, wptr++, xptro++, yptro++, zptro++, wptro++ )
	{
		*xptro = m->rows[0][0] * (*xptr) + m->rows[1][0] * (*yptr) + m->rows[2][0] * (*zptr) + m->rows[3][0] * (*wptr);
		*yptro = m->rows[0][1] * (*xptr) + m->rows[1][1] * (*yptr) + m->rows[2][1] * (*zptr) + m->rows[3][1] * (*wptr);
		*zptro = m->rows[0][2] * (*xptr) + m->rows[1][2] * (*yptr) + m->rows[2][2] * (*zptr) + m->rows[3][2] * (*wptr);
		*wptro = m->rows[0][3] * (*xptr) + m->rows[1][3] * (*yptr) + m->rows[2][3] * (*zptr) + m->rows[3][3] * (*wptr);
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_SSE
	for ( uint32_t cell = 0; cell < 4; cell++ )
	{
		__m128 *ix = (__m128*)v->cells[0], *iy = (__m128*)v->cells[1], *iz = (__m128*)v->cells[2], *iw = (__m128*)v->cells[3];
		__m128* o = (__m128*)out->cells[cell];
		__m128 md[4] = { _mm_set1_ps ( m->rows[0][cell] ), _mm_set1_ps ( m->rows[1][cell] ), _mm_set1_ps ( m->rows[2][cell] ), _mm_set1_ps ( m->rows[3][cell] ) };
		for ( uint32_t vec = 0; vec < (v->vectorCount+3)/4; vec++, ix++, iy++, iz++, iw++, o++ )
			*o = _mm_add_ps ( _mm_add_ps ( _mm_mul_ps ( md[0], *ix ), _mm_mul_ps ( md[1], *iy ) ), _mm_add_ps ( _mm_mul_ps ( md[2], *iz ), _mm_mul_ps ( md[3], *iw ) ) );
	}
#elif RVM_MATH_VECTOR_INSTR_SET == RVM_MATH_VECTOR_INSTR_SET_AVX
	for ( uint32_t cell = 0; cell < 4; cell++ )
	{
		__m256 *ix = (__m256*)v->cells[0], *iy = (__m256*)v->cells[1], *iz = (__m256*)v->cells[2], *iw = (__m256*)v->cells[3];
		__m256* o = (__m256*)out->cells[cell];
		__m256 md[4] = { _mm256_set1_ps ( m->rows[0][cell] ), _mm256_set1_ps ( m->rows[1][cell] ), _mm256_set1_ps ( m->rows[2][cell] ), _mm256_set1_ps ( m->rows[3][cell] ) };
		for ( uint32_t vec = 0; vec < (v->vectorCount+7)/8; vec++, ix++, iy++, iz++, iw++, o++ )
			*o = _mm256_add_ps ( _mm256_add_ps ( _mm256_mul_ps ( md[0], *ix ), _mm256_mul_ps ( md[1], *iy ) ), _mm256_add_ps ( _mm256_mul_ps ( md[2], *iz ), _mm256_mul_ps ( md[3], *iw ) ) );
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

rvm_aos_mat3 rvm_aos_mat3_translate ( float x, float y )
{
	return (rvm_aos_mat3){
		.rows[0][0] = 1.0f, .rows[0][1] = 0.0f, .rows[0][2] = 0.0f,
		.rows[1][0] = 0.0f, .rows[1][1] = 1.0f, .rows[1][2] = 0.0f,
		.rows[2][0] = x,    .rows[2][1] = y,    .rows[2][2] = 1.0f,
	};
}

rvm_aos_mat3 rvm_aos_mat3_rotate ( float angleRad )
{
	float s = sinf ( angleRad ), c = cosf ( angleRad );
	return (rvm_aos_mat3){
		.rows[0][0] = c,    .rows[0][1] = -s,   .rows[0][2] = 0.0f,
		.rows[1][0] = s,    .rows[1][1] =  c,   .rows[1][2] = 0.0f,
		.rows[2][0] = 0.0f, .rows[2][1] = 0.0f, .rows[2][2] = 1.0f,
	};
}

rvm_aos_mat3 rvm_aos_mat3_scale ( float x, float y )
{
	return (rvm_aos_mat3){
		.rows[0][0] = x,    .rows[0][1] = 0.0f, .rows[0][2] = 0.0f,
		.rows[1][0] = 0.0f, .rows[1][1] = y,    .rows[1][2] = 0.0f,
		.rows[2][0] = 0.0f, .rows[2][1] = 0.0f, .rows[2][2] = 1.0f,
	};
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Originally based upon Real Time Rendering perspective transform chapter
// Modifications based upon observations, followed by comparison with GLM's perspective function
rvm_aos_mat4 rvm_aos_mat4_perspective ( float fovY, float aspect, float n, float f )
{
	float t = tanf ( fovY/2 );
	float r = aspect * t;

	return (rvm_aos_mat4){
		.rows = {
			{ 1.0f/r, 0.0f,    0.0f,             0.0f },
			{ 0.0f,   1.0f/t,  0.0f,             0.0f },
			{ 0.0f,   0.0f,   -((f+n)/(f-n)),   -1.0f },
			{ 0.0f,   0.0f,   -((2*f*n)/(f-n)),  0.0f },
		},
	};
}

rvm_aos_mat4 rvm_aos_mat4_orthographic ( float w, float h, float n, float f )
{
	float r = w/2, l = -w/2;
	float t = h/2, b = -h/2;
	return (rvm_aos_mat4){
		.rows = {
			{      2/(r-l),         0.0f,      0.0f, 0.0f },
			{         0.0f,      2/(t-b),      0.0f, 0.0f },
			{         0.0f,         0.0f,  -1/(f-n), 0.0f },
			{ -(r+l)/(r-l), -(t+b)/(t-b), (n)/(f-n), 1.0f },
		},
	};
}

rvm_aos_mat4 rvm_aos_mat4_translate ( float x, float y, float z )
{
	return (rvm_aos_mat4){
		.rows[0][0] = 1.0f, .rows[0][1] = 0.0f, .rows[0][2] = 0.0f, .rows[0][3] = 0.0f,
		.rows[1][0] = 0.0f, .rows[1][1] = 1.0f, .rows[1][2] = 0.0f, .rows[1][3] = 0.0f,
		.rows[2][0] = 0.0f, .rows[2][1] = 0.0f, .rows[2][2] = 1.0f, .rows[2][3] = 0.0f,
		.rows[3][0] = x,    .rows[3][1] = y,    .rows[3][2] = z,    .rows[3][3] = 1.0f,
	};
}

rvm_aos_mat4 rvm_aos_mat4_rotate_x ( float angleRad )
{
	float s = sinf ( angleRad ), c = cosf ( angleRad );
	return (rvm_aos_mat4){
		.rows = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f,    c,   -s, 0.0f,
			0.0f,    s,    c, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		},
	};
}

rvm_aos_mat4 rvm_aos_mat4_rotate_y ( float angleRad )
{
	float s = sinf ( angleRad ), c = cosf ( angleRad );
	return (rvm_aos_mat4){
		.rows = {
			   c, 0.0f,   -s, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			   s, 0.0f,    c, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		},
	};
}

rvm_aos_mat4 rvm_aos_mat4_rotate_z ( float angleRad )
{
	float s = sinf ( angleRad ), c = cosf ( angleRad );
	return (rvm_aos_mat4){
		.rows = {
			   c,   -s, 0.0f, 0.0f,
			   s,    c, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		},
	};
}

rvm_aos_mat4 rvm_aos_mat4_scale ( float scaleX, float scaleY, float scaleZ )
{
	return (rvm_aos_mat4){
		.rows = {
			scaleX, 0.0f,   0.0f,   0.0f,
			0.0f,   scaleY, 0.0f,   0.0f,
			0.0f,   0.0f,   scaleZ, 0.0f,
			0.0f,   0.0f,   0.0f,   1.0f,
		},
	};
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// http://en.wikipedia.org/wiki/Laplace_expansion
float rvm_aos_mat4_determinant ( const rvm_aos_mat4* m )
{
	return    (m->cells[ 0] * m->cells[ 5] - m->cells[ 1] * m->cells[ 4]) * (m->cells[10] * m->cells[15] - m->cells[11] * m->cells[14])
			- (m->cells[ 0] * m->cells[ 6] - m->cells[ 2] * m->cells[ 4]) * (m->cells[ 9] * m->cells[15] - m->cells[11] * m->cells[13])
			+ (m->cells[ 0] * m->cells[ 7] - m->cells[ 3] * m->cells[ 4]) * (m->cells[ 9] * m->cells[14] - m->cells[10] * m->cells[13])
			+ (m->cells[ 1] * m->cells[ 6] - m->cells[ 2] * m->cells[ 5]) * (m->cells[ 8] * m->cells[15] - m->cells[11] * m->cells[12])
			- (m->cells[ 1] * m->cells[ 7] - m->cells[ 3] * m->cells[ 5]) * (m->cells[ 8] * m->cells[14] - m->cells[10] * m->cells[12])
			+ (m->cells[ 2] * m->cells[ 7] - m->cells[ 3] * m->cells[ 6]) * (m->cells[ 8] * m->cells[13] - m->cells[ 9] * m->cells[12]);
}

rvm_aos_mat4 rvm_aos_mat4_transpose ( const rvm_aos_mat4* m )
{
	return (rvm_aos_mat4){
		.rows = {
			{ m->rows[0][0], m->rows[1][0], m->rows[2][0], m->rows[3][0] },
			{ m->rows[0][1], m->rows[1][1], m->rows[2][1], m->rows[3][1] },
			{ m->rows[0][2], m->rows[1][2], m->rows[2][2], m->rows[3][2] },
			{ m->rows[0][3], m->rows[1][3], m->rows[2][3], m->rows[3][3] },
		}
	};
}

// https://www.youtube.com/watch?v=nNOz6Ez8Fn4
rvm_aos_mat4 rvm_aos_mat4_inverse ( const rvm_aos_mat4* m )
{
	rvm_aos_mat4 out;
	float detFactor = 1.0f / rvm_aos_mat4_determinant ( m );

#define DET3(m,c00,c01,c02,c10,c11,c12,c20,c21,c22)			\
		( (m->cells[c00] * m->cells[c11] * m->cells[c22])	\
		+ (m->cells[c01] * m->cells[c12] * m->cells[c20])	\
		+ (m->cells[c02] * m->cells[c10] * m->cells[c21])	\
		- (m->cells[c00] * m->cells[c12] * m->cells[c21])	\
		- (m->cells[c01] * m->cells[c10] * m->cells[c22])	\
		- (m->cells[c02] * m->cells[c11] * m->cells[c20]))

	//  0,  1,  2,  3
	//  4,  5,  6,  7
	//  8,  9, 10, 11
	// 12, 13, 14, 15

	// Cofactors matrix
	// -----------------------------------------------------
	// |  5,  6,  7 |  4,  6,  7 |  4,  5,  7 |  4,  5,  6 |
	// |  9, 10, 11 |  8, 10, 11 |  8,  9, 11 |  8,  9, 10 |
	// | 13, 14, 15 | 12, 14, 15 | 12, 13, 15 | 12, 13, 14 |
	// -----------------------------------------------------
	// |  1,  2,  3 |  0,  2,  3 |  0,  1,  3 |  0,  1,  2 |
	// |  9, 10, 11 |  8, 10, 11 |  8,  9, 11 |  8,  9, 10 |
	// | 13, 14, 15 | 12, 14, 15 | 12, 13, 15 | 12, 13, 14 |
	// -----------------------------------------------------
	// |  1,  2,  3 |  0,  2,  3 |  0,  1,  3 |  0,  1,  2 |
	// |  5,  6,  7 |  4,  6,  7 |  4,  5,  7 |  4,  5,  6 |
	// | 13, 14, 15 | 12, 14, 15 | 12, 13, 15 | 12, 13, 14 |
	// -----------------------------------------------------
	// |  1,  2,  3 |  0,  2,  3 |  0,  1,  3 |  0,  1,  2 |
	// |  5,  6,  7 |  4,  6,  7 |  4,  5,  7 |  4,  5,  6 |
	// |  9, 10, 11 |  8, 10, 11 |  8,  9, 11 |  8,  9, 10 |
	// -----------------------------------------------------

	// Cofactors transposed
	// -----------------------------------------------------
	// |  5,  6,  7 |  1,  2,  3 |  1,  2,  3 |  1,  2,  3 |
	// |  9, 10, 11 |  9, 10, 11 |  5,  6,  7 |  5,  6,  7 |
	// | 13, 14, 15 | 13, 14, 15 | 13, 14, 15 |  9, 10, 11 |
	// -----------------------------------------------------
	// |  4,  6,  7 |  0,  2,  3 |  0,  2,  3 |  0,  2,  3 |
	// |  8, 10, 11 |  8, 10, 11 |  4,  6,  7 |  4,  6,  7 |
	// | 12, 14, 15 | 12, 14, 15 | 12, 14, 15 |  8, 10, 11 |
	// -----------------------------------------------------
	// |  4,  5,  7 |  0,  1,  3 |  0,  1,  3 |  0,  1,  3 |
	// |  8,  9, 11 |  8,  9, 11 |  4,  5,  7 |  4,  5,  7 |
	// | 12, 13, 15 | 12, 13, 15 | 12, 13, 15 |  8,  9, 11 |
	// -----------------------------------------------------
	// |  4,  5,  6 |  0,  1,  2 |  0,  1,  2 |  0,  1,  2 |
	// |  8,  9, 10 |  8,  9, 10 |  4,  5,  6 |  4,  5,  6 |
	// | 12, 13, 14 | 12, 13, 14 | 12, 13, 14 |  8,  9, 10 |
	// -----------------------------------------------------
			
			
	// -----------------------------------------------------
	// |  5,  6,  7 |  1,  2,  3 |  1,  2,  3 |  1,  2,  3 |
	// |  9, 10, 11 |  9, 10, 11 |  5,  6,  7 |  5,  6,  7 |
	// | 13, 14, 15 | 13, 14, 15 | 13, 14, 15 |  9, 10, 11 |
	// -----------------------------------------------------
	out.rows[0][0] =  detFactor * DET3(m,  5,  6,  7,
								            9, 10, 11,
								            13, 14, 15 );
	out.rows[0][1] = -detFactor * DET3(m,  1,  2,  3,
								            9, 10, 11,
								            13, 14, 15 );
	out.rows[0][2] =  detFactor * DET3(m,  1,  2,  3,
								            5,  6,  7,
								            13, 14, 15 );
	out.rows[0][3] = -detFactor * DET3(m,  1,  2,  3,
								            5,  6,  7,
								            9, 10, 11 );

	// -----------------------------------------------------
	// |  4,  6,  7 |  0,  2,  3 |  0,  2,  3 |  0,  2,  3 |
	// |  8, 10, 11 |  8, 10, 11 |  4,  6,  7 |  4,  6,  7 |
	// | 12, 14, 15 | 12, 14, 15 | 12, 14, 15 |  8, 10, 11 |
	// -----------------------------------------------------
	out.rows[1][0] = -detFactor * DET3(m,  4,  6,  7,
								            8, 10, 11,
								            12, 14, 15 );
	out.rows[1][1] =  detFactor * DET3(m,  0,  2,  3,
								            8, 10, 11,
								            12, 14, 15 );
	out.rows[1][2] = -detFactor * DET3(m,  0,  2,  3,
								            4,  6,  7,
								            12, 14, 15 );
	out.rows[1][3] =  detFactor * DET3(m,  0,  2,  3,
								            4,  6,  7,
								            8, 10, 11 );

	// -----------------------------------------------------
	// |  4,  5,  7 |  0,  1,  3 |  0,  1,  3 |  0,  1,  3 |
	// |  8,  9, 11 |  8,  9, 11 |  4,  5,  7 |  4,  5,  7 |
	// | 12, 13, 15 | 12, 13, 15 | 12, 13, 15 |  8,  9, 11 |
	// -----------------------------------------------------
	out.rows[2][0] =  detFactor * DET3(m,  4,  5,  7,
								            8,  9, 11,
								            12, 13, 15 );
	out.rows[2][1] = -detFactor * DET3(m,  0,  1,  3,
								            8,  9, 11,
								            12, 13, 15 );
	out.rows[2][2] =  detFactor * DET3(m,  0,  1,  3,
								            4,  5,  7,
								            12, 13, 15 );
	out.rows[2][3] = -detFactor * DET3(m,  0,  1,  3,
								            4,  5,  7,
								            8,  9, 11 );

	// -----------------------------------------------------
	// |  4,  5,  6 |  0,  1,  2 |  0,  1,  2 |  0,  1,  2 |
	// |  8,  9, 10 |  8,  9, 10 |  4,  5,  6 |  4,  5,  6 |
	// | 12, 13, 14 | 12, 13, 14 | 12, 13, 14 |  8,  9, 10 |
	// -----------------------------------------------------
	out.rows[3][0] = -detFactor * DET3(m,  4,  5,  6,
								            8,  9, 10,
								            12, 13, 14 );
	out.rows[3][1] =  detFactor * DET3(m,  0,  1,  2,
								            8,  9, 10,
								            12, 13, 14 );
	out.rows[3][2] = -detFactor * DET3(m,  0,  1,  2,
								            4,  5,  6,
								            12, 13, 14 );
	out.rows[3][3] =  detFactor * DET3(m,  0,  1,  2,
								            4,  5,  6,
								            8,  9, 10 );
#undef DET3
	return out;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RVM_MATH_IMPLEMENTATION

#ifdef __cplusplus
};
#endif

#endif // __RVM_MATH_H