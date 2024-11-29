#pragma once
#ifndef __INCLUDE_GUARD_vulkanUniformBuffer_h__
#define __INCLUDE_GUARD_vulkanUniformBuffer_h__
#include <vulkan/vulkan.h>
#include <memory>
#include "vulkanContext.h"
#include "vmaBuffer.h"
#include "spirvReflect.h"



class VulkanUniformBuffer
{
public: // Members:
	std::shared_ptr<VmaBuffer> buffer;

private: // Members:
	VulkanContext* context;
	void* deviceData;
	std::vector<char> hostData;
	UniformBufferBlock* uniformBufferBlock;

public: // Methods:
	VulkanUniformBuffer(VulkanContext* context, UniformBufferBlock* uniformBufferBlock);
	~VulkanUniformBuffer();

	void UpdateBuffer();

	// Getters:
	uint32_t GetSize();

	// Setters:
	template<typename T>
	bool SetValue(const std::string& memberName, const T& value);
	template<typename T>
	bool SetValue(const std::string& arrayName, uint32_t arrayIndex, const T& value);
	template<typename T>
	bool SetValue(const std::string& arrayName, uint32_t arrayIndex, const std::string& memberName, const T& value);
	template<typename T>
	bool SetValue(const std::string& arrayName, uint32_t arrayIndex, const std::string& subArrayName, uint32_t subArrayIndex, const T& value);

private: // Methods:
	template<typename T>
	bool CheckAndUpdateData(const T& value, uint32_t offset, uint32_t size);
};



#endif // __INCLUDE_GUARD_vulkanUniformBuffer_h__