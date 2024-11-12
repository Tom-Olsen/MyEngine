#pragma once
#ifndef __INCLUDE_GUARD_float3_h__
#define __INCLUDE_GUARD_float3_h__
#include <string>



// Forward declarations:
struct Float2;
struct Float4;



struct Float3
{
private:
	constexpr static float epsilon = 1e-8f;
public:
	// Members:
	float x, y, z;

	// Constructors:
	Float3();
	Float3(float xyz);
	Float3(float x, float y);
	Float3(float x, float y, float z);
	Float3(const Float2& xy);
	Float3(const Float2& xy, float z);
	Float3(const Float3& xyz);
	Float3(const Float4& xyz);
	static Float3 Direction(float theta, float phi);

	// Math operations:
	float Length2() const;
	float Length() const;
	float Theta() const;
	float Phi() const;
	Float2 AnglesDegrees() const;
	Float2 AnglesRadians() const;
	Float3 Normalize() const;
	Float3 Rotate(float theta, float phi) const;
	bool IsEpsilonZero() const;

	// Static math operations:
	static float Dot(const Float3& a, const Float3& b);
	static Float3 Cross(const Float3& a, const Float3& b);
	static float VectorToPlaneDistance(const Float3& vec, const Float3& planeNormal);
	static Float3 VectorToPlaneProjection(const Float3& vec, const Float3& planeNormal);
	static Float3 Reflect(const Float3& vec, const Float3& planeNormal);
	static float Distance2(const Float3& a, const Float3& b);
	static float Distance(const Float3& a, const Float3& b);
	static float AngleDegrees(const Float3& a, const Float3& b);
	static float AngleRadians(const Float3& a, const Float3& b);

	// Access:
	float& operator[](int index);
	float operator[](int index) const;

	// Assignment:
	Float3& operator=(const Float3& other);
	Float3& operator=(Float3&& other) noexcept;

	// Addition:
	Float3 operator+(const Float3& other) const;
	Float3& operator+=(const Float3& other);

	// Substraction:
	Float3 operator-(const Float3& other) const;
	Float3& operator-=(const Float3& other);
	Float3 operator-() const;

	// Multiplication:
	Float3 operator*(const Float3& other) const;
	Float3& operator*=(const Float3& other);
	Float3 operator*(float scalar) const;
	Float3& operator*=(float scalar);

	// Division:
	Float3 operator/(const Float3& other) const;
	Float3& operator/=(const Float3& other);
	Float3 operator/(float scalar) const;
	Float3& operator/=(float scalar);

	// Comparison:
	bool IsEpsilonEqual(const Float3& other) const;
	bool operator==(const Float3& other) const;
	bool operator!=(const Float3& other) const;

	// Friend functions:
	friend Float3 operator*(float a, const Float3& b);

	// Conversion:
	Float3 ToDegrees() const;
	Float3 ToRadians() const;

	// Logging:
	std::string ToString() const;
	friend std::ostream& operator<<(std::ostream& os, const Float3& value);

	// Static members:
	static Float3 zero;
	static Float3 one;
	static Float3 right;	// +x = ( 1, 0, 0).
	static Float3 left;		// -x = (-1, 0, 0).
	static Float3 up;		// +y = ( 0, 1, 0).
	static Float3 down;		// -y = ( 0,-1, 0).
	static Float3 forward;	// +z = ( 0, 0, 1).
	static Float3 backward;	// -z = ( 0, 0,-1).
};



#endif // __INCLUDE_GUARD_float3_h__