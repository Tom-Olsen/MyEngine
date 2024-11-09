#include "uint3.h"
#include <stdexcept>
#include <sstream>



// Constructors:
Uint3::Uint3() : x(0), y(0), z(0) {}
Uint3::Uint3(uint32_t xyz) : x(xyz), y(xyz), z(xyz) {}
Uint3::Uint3(uint32_t x, uint32_t y, uint32_t z) : x(x), y(y), z(z) {}
Uint3::Uint3(const Uint3& xyz) : x(xyz.x), y(xyz.y), z(xyz.z) {}



// Math operations:



// Static math operations:



// Access:
uint32_t& Uint3::operator[](int index)
{
	if (index == 0) return x;
	if (index == 1) return y;
	if (index == 2) return z;
	throw std::out_of_range("Uint3 index out of range.");
}
uint32_t Uint3::operator[](int index) const
{
	if (index == 0) return x;
	if (index == 1) return y;
	if (index == 2) return z;
	throw std::out_of_range("Uint3 index out of range.");
}



// Assignment:
Uint3& Uint3::operator=(const Uint3& other)
{
	if (this != &other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}
	return *this;
}
Uint3& Uint3::operator=(Uint3&& other) noexcept
{
	if (this != &other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}
	return *this;
}



// Addition:
Uint3 Uint3::operator+(const Uint3& other) const
{
	return Uint3(x + other.x, y + other.y, z + other.z);
}
Uint3& Uint3::operator+=(const Uint3& other)
{
	this->x += other.x;
	this->y += other.y;
	this->z += other.z;
	return *this;
}



// Substraction:
Uint3 Uint3::operator-(const Uint3& other) const
{
	return Uint3(x - other.x, y - other.y, z - other.z);
}
Uint3& Uint3::operator-=(const Uint3& other)
{
	this->x -= other.x;
	this->y -= other.y;
	this->z -= other.z;
	return *this;
}



// Comparison:
bool Uint3::operator==(const Uint3& other) const
{
	return x == other.x && y == other.y && z == other.z;
}
bool Uint3::operator!=(const Uint3& other) const
{
	return !(*this == other);
}



// Logging:
std::string Uint3::ToString() const
{
	std::ostringstream oss;
	oss << "(" << x << ", " << y << ", " << z << ")";
	return oss.str();
}
std::ostream& operator<<(std::ostream& os, const Uint3& value)
{
	os << value.ToString();
	return os;
}



// Static members:
Uint3 Uint3::zero = Uint3(0);
Uint3 Uint3::one = Uint3(1);
Uint3 Uint3::right = Uint3(1, 0, 0);
Uint3 Uint3::up = Uint3(0, 1, 0);
Uint3 Uint3::forward = Uint3(0, 0, 1);