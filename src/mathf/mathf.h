#ifndef __INCLUDE_GUARD_mathf_h__
#define __INCLUDE_GUARD_mathf_h__
#include <stdint.h>



// Enums:
enum CoordinateSystem
{
	local,
	world
};



// Multi indexing:
struct Index2
{
	uint32_t i, j;
};



// Class/Struct independent functions:
namespace mathf
{
	// Constants:
	constexpr float EPSILON = 1e-4f;
	constexpr float PI = 3.14159265358979323846f;
	constexpr float PI_2 = 0.50f * PI;
	constexpr float PI_4 = 0.25f * PI;
	constexpr float PI_INV = 1.0f / PI;
	constexpr float RAD2DEG = 180.0f / PI;
	constexpr float DEG2RAD = PI / 180.0f;
	constexpr float E = 2.71828182845904523536f;
	constexpr float SQRT2 = 1.41421356237309504880f;
	constexpr float SQRT2_INV = 0.70710678118654752440f;
	constexpr float SQRT3 = 1.73205080756887729353f;
	constexpr float SQRT3_INV = 0.57735026918962576451f;

	// Basic math:
	float Abs(float value);
	float Clamp(float value, float min, float max);
	float Max(float a, float b);
	float Min(float a, float b);
	float Sign(float value);
	float Sqrt(float value);
	float Factorial(int n);

	// Trigonometry:
	float Sin(float radiant);
	float Cos(float radiant);
	float Tan(float radiant);
	float Asin(float value);
	float Acos(float value);
	float Atan(float z);
	float Atan2(float y, float x);
}



// Include all mathf headers:
// Basic types:
#include "int2.h"
#include "int3.h"
#include "uint3.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "float2x2.h"
#include "float2x3.h"
#include "float3x2.h"
#include "float3x3.h"
#include "float4x4.h"

// Geometry:
#include "bounds.h"
#include "geometry3d.h"

// Random:
#include "emberRandom.h"
#endif // __INCLUDE_GUARD_mathf_h__