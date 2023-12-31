﻿#include "Types.h"

INT2::INT2(int x, int y) :
	x{ x },
	y{ y }
{

}

INT2 operator+(const INT2& lhs, const INT2& rhs)
{
	INT2 value{ lhs };
	value.x += rhs.x;
	value.y += rhs.y;
	return value;
}

INT2 operator-(const INT2& lhs, const INT2& rhs)
{
	INT2 value{ lhs };
	value.x -= rhs.x;
	value.y -= rhs.y;
	return value;
}

void operator+=(INT2& lhs, const INT2& rhs)
{
	lhs = lhs + rhs;
}

void operator-=(INT2& lhs, const INT2& rhs)
{
	lhs = lhs - rhs;
}

FLOAT2::FLOAT2(FLOAT x, FLOAT y)
{
	this->x = x;
	this->y = y;
}

FLOAT2 operator+(const FLOAT2& lhs, const FLOAT2& rhs)
{
	FLOAT2 value{ lhs };
	value.x += rhs.x;
	value.y += rhs.y;
	return value;
}

FLOAT2 operator-(const FLOAT2& lhs, const FLOAT2& rhs)
{
	FLOAT2 value{ lhs };
	value.x -= rhs.x;
	value.y -= rhs.y;
	return value;
}

void operator+=(FLOAT2& lhs, const FLOAT2& rhs)
{
	lhs = lhs + rhs;
}

void operator-=(FLOAT2& lhs, const FLOAT2& rhs)
{
	lhs = lhs - rhs;
}

FLOAT2 operator*(const FLOAT2& lhs, FLOAT rhs)
{
	return { lhs.x * rhs, lhs.y * rhs };
}

FLOAT2 operator/(const FLOAT2& lhs, FLOAT rhs)
{
	return { lhs.x / rhs, lhs.y / rhs };
}

RECTI::RECTI() :
	left{ 0 }, top{ 0 }, right{ 0 }, bottom{ 0 }
{

}

RECTI::RECTI(int left, int top, int right, int bottom) :
	left{ left }, top{ top }, right{ right }, bottom{ bottom }
{

}

RECTI& RECTI::Offset(int x, int y)
{
	left += x;
	top += y;
	right += x;
	bottom += y;
	return *this;
}

bool RECTI::IsContain(const INT2& point)
{
	if (left <= point.x && point.x <= right &&
		top <= point.y && point.y <= bottom)
		return true;
	return false;
}


RECTF::RECTF()
{
	left = 0.0f;
	top = 0.0f;
	right = 0.0f;
	bottom = 0.0f;
}

RECTF::RECTF(FLOAT left, FLOAT top, FLOAT right, FLOAT bottom)
{
	this->left = left;
	this->top = top;
	this->right = right;
	this->bottom = bottom;
}

RECTF& RECTF::Offset(FLOAT x, FLOAT y)
{
	left += x;
	top += y;
	right += x;
	bottom += y;
	return *this;
}

bool RECTF::IsContain(FLOAT2 point)
{
	if (left <= point.x && point.x <= right &&
		top <= point.y && point.y <= bottom)
		return true;
	return false;
}