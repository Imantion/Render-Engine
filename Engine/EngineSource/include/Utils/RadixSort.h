#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>	// QueryPerformanceCounter
#include "Utils/Definitions.h"
// ------------------------------------------------------------------------------------------------
// ---- Basic types

typedef long int32;
typedef unsigned long uint32;
typedef float real32;
typedef double real64;
typedef unsigned char uint8;
typedef const char* cpointer;

// ------------------------------------------------------------------------------------------------
// Configuration/Testing

// ---- really, a correctness check, not correctness itself ;)
#define CORRECTNESS	1

// ---- use SSE prefetch (needs compiler support), not really a problem on non-SSE machines.
//		need http://msdn.microsoft.com/vstudio/downloads/ppack/default.asp
//		or recent VC to use this

#define PREFETCH 0

#if PREFETCH
#include <xmmintrin.h>	// for prefetch
#define pfval	64
#define pfval2	128
#define pf(x)	_mm_prefetch(cpointer(x + i + pfval), 0)
#define pf2(x)	_mm_prefetch(cpointer(x + i + pfval2), 0)
#else
#define pf(x)
#define pf2(x)
#endif

// ------------------------------------------------------------------------------------------------
// ---- Visual C++ eccentricities

#if _WINDOWS
#define finline __forceinline
#else
#define finline inline
#endif

// ================================================================================================
// flip a float for sorting
//  finds SIGN of fp number.
//  if it's 1 (negative float), it flips all bits
//  if it's 0 (positive float), it flips the sign only
// ================================================================================================
finline uint32 FloatFlip(uint32 f)
{
	uint32 mask = -int32(f >> 31) | 0x80000000;
	return f ^ mask;
}

finline void FloatFlipX(uint32& f)
{
	uint32 mask = -int32(f >> 31) | 0x80000000;
	f ^= mask;
}

// ================================================================================================
// flip a float back (invert FloatFlip)
//  signed was flipped from above, so:
//  if sign is 1 (negative), it flips the sign bit back
//  if sign is 0 (positive), it flips all bits back
// ================================================================================================
finline uint32 IFloatFlip(uint32 f)
{
	uint32 mask = ((f >> 31) - 1) | 0x80000000;
	return f ^ mask;
}

// ---- utils for accessing 11-bit quantities
#define _0(x)	(x & 0x7FF)
#define _1(x)	(x >> 11 & 0x7FF)
#define _2(x)	(x >> 22 )


static void RadixSort11(Engine::IndexedDistance* farray, Engine::IndexedDistance* sorted, uint32 elements)
{
	uint32 i;
	Engine::IndexedDistance* sort = sorted;
	Engine::IndexedDistance* array = farray;

	// 3 histograms on the stack:
	const uint32 kHist = 2048;
	uint32 b0[kHist * 3];

	uint32* b1 = b0 + kHist;
	uint32* b2 = b1 + kHist;

	for (i = 0; i < kHist * 3; i++) {
		b0[i] = 0;
	}
	//memset(b0, 0, kHist * 12);

	// 1.  parallel histogramming pass
	//
	for (i = 0; i < elements; i++) {

		pf(array);

		uint32 fi = FloatFlip((uint32&)array[i].distance);


		b0[_0(fi)]++;
		b1[_1(fi)]++;
		b2[_2(fi)]++;
	}

	// 2.  Sum the histograms -- each histogram entry records the number of values preceding itself.
	{
		uint32 sum0 = 0, sum1 = 0, sum2 = 0;
		uint32 tsum;
		for (i = 0; i < kHist; i++) {

			tsum = b0[i] + sum0;
			b0[i] = sum0 - 1;
			sum0 = tsum;

			tsum = b1[i] + sum1;
			b1[i] = sum1 - 1;
			sum1 = tsum;

			tsum = b2[i] + sum2;
			b2[i] = sum2 - 1;
			sum2 = tsum;
		}
	}

	// byte 0: floatflip entire value, read/write histogram, write out flipped
	for (i = 0; i < elements; i++) {

		uint32 fi = *(uint32*)&array[i].distance;
		FloatFlipX(fi);
		uint32 pos = _0(fi);

		pf2(array);
		sort[++b0[pos]].index = array[i].index;
		sort[b0[pos]].distance = *(float*)&fi;
	}

	// byte 1: read/write histogram, copy
	//   sorted -> array
	for (i = 0; i < elements; i++) {
		uint32 si = *(uint32*)&sort[i].distance;
		uint32 pos = _1(si);

		pf2(sort);
		array[++b1[pos]].index = sort[i].index;
		array[b1[pos]].distance = (float&)si;
	}

	// byte 2: read/write histogram, copy & flip out
	//   array -> sorted
	for (i = 0; i < elements; i++) {
		uint32 ai = (uint32&)array[i].distance;
		uint32 pos = _2(ai);

		pf2(array);
		uint32 value = IFloatFlip(ai);
		sort[++b2[pos]].distance = (float&)value;
		sort[b2[pos]].index = array[i].index;
	}

	// to write original:
	// memcpy(array, sorted, elements * 4);
}
