#include "Vec2.h"
#include <math.h>
#include <iostream>

Vec2::Vec2()
{

}

Vec2::Vec2(float xin, float yin) 
	: x(xin), y(yin) { }

bool Vec2::operator == (const Vec2& rhs) const
{
	return (x == rhs.x && y == rhs.y);
}

bool Vec2::operator != (const Vec2& rhs) const
{
	return !(x == rhs.x && y == rhs.y);
}

Vec2 Vec2::operator + (const Vec2& rhs) const
{
	return Vec2(x + rhs.x, y + rhs.y);
}

Vec2 Vec2::operator - (const Vec2& rhs) const
{
	return Vec2(x - rhs.x, y - rhs.y);
}

Vec2 Vec2::operator * (const float val) const
{
	return Vec2(x * val, y * val);
}

Vec2 Vec2::operator / (const float val) const
{
	return Vec2(x / val, y / val);
}

void Vec2::operator += (const Vec2& rhs)
{
	x += rhs.x;
	y += rhs.y;
}
void Vec2::operator -= (const Vec2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
}

void Vec2::operator *= (const float val)
{
	x *= val;
	y *= val;
}

void Vec2::operator /= (const float val)
{
	x /= val;
	y /= val;
}


float Vec2::dist(const Vec2& rhs) const
{
    Vec2 dVec = *this - rhs;
    return std::sqrtf((dVec.x * dVec.x) + (dVec.y * dVec.y));
}

float Vec2::length() const
{
	return std::sqrtf((x * x) + (y * y));
}

void Vec2::normalize()
{
    float length = this->length();
    *this /= length;
}

// Will have rounding issues, keep an eye on it.
Vec2 Vec2::fromAngleAndSpeed(float angle, float speed)
{
    return Vec2(speed * std::cosf(angle), speed * std::sinf(angle));
}

void Vec2::test()
{
    Vec2 v1(100, 100);
    Vec2 v2(300, 500);

    // Test addition
    Vec2 v3 = v1 + v2;
    std::cout << v3.x << " == " << 400 << std::endl;
    std::cout << v3.y << " == " << 600 << std::endl;
    // Test subtraction
    Vec2 v4 = v1 - v2;
    std::cout << v4.x << " == " << -200 << std::endl;
    std::cout << v4.y << " == " << -400 << std::endl;
    // Test multiplication
    Vec2 v5 = v1 * 2;
    std::cout << v5.x << " == " << 200 << std::endl;
    std::cout << v5.y << " == " << 200 << std::endl;
    // Test division
    Vec2 v6 = v1 / 2;
    std::cout << v6.x << " == " << 50 << std::endl;
    std::cout << v6.y << " == " << 50 << std::endl;
    // Test +=
    v1 += v2;
    std::cout << v1.x << " == " << 400 << std::endl;
    std::cout << v1.y << " == " << 600 << std::endl;
    // Test -=
    v1 -= v2;
    std::cout << v1.x << " == " << 100 << std::endl;
    std::cout << v1.y << " == " << 100 << std::endl;
    // Test *=
    v1 *= 2;
    std::cout << v1.x << " == " << 200 << std::endl;
    std::cout << v1.y << " == " << 200 << std::endl;
    // Test /=
    v1 /= 2;
    std::cout << v1.x << " == " << 100 << std::endl;
    std::cout << v1.y << " == " << 100 << std::endl;
    // Test dist
    float dist = Vec2(0, 0).dist(Vec2(3, 4));
    std::cout << dist << " == " << 5 << std::endl;
    // Test length
    std::cout << v1.length() << " ~= " << 141.42 << std::endl;
    // Test normalize
    v1.normalize();
    std::cout << v1.length() << " == " << 1 << std::endl;
    // Test fromAngleAndSpeed
    Vec2 v7 = Vec2::fromAngleAndSpeed(0.9272, 5);
    std::cout << v7.x << " ~= " << 3 << std::endl;
    std::cout << v7.y << " ~= " << 4 << std::endl;
    Vec2 v8 = Vec2::fromAngleAndSpeed(0, 5);
    std::cout << v8.x << " ~= " << 5 << std::endl;
    std::cout << v8.y << " ~= " << 0 << std::endl;
}