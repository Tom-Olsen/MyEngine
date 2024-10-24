#pragma once
#ifndef __INCLUDE_GUARD_pushConstant_h__
#define __INCLUDE_GUARD_pushConstant_h__
#include "glmTypes.h"



/// <summary>
/// Size limit for push constants is 128 bytes.
/// Only used for small data that is updated every frame.
/// </summary>
struct PushConstantObject
{
public:
	alignas(16) Float4 time;	// 16 bytes
	alignas(16) Float4 deltaTime;	// 16 bytes

private:
	char padding[128 - 2 * sizeof(Float4)];

public:
	PushConstantObject()
	{
		time = Float4(0.0f);
		deltaTime = Float4(0.0f);
		for (int i = 0; i < sizeof(padding); i++)
			padding[i] = 0;
	}

	PushConstantObject(Float4 time, Float4 deltaTime)
	{
		this->time = time;
		this->deltaTime = deltaTime;
		for (int i = 0; i < sizeof(padding); i++)
			padding[i] = 0;
	}
};



#endif // __INCLUDE_GUARD_pushConstant_h__