#pragma once
#ifndef __INCLUDE_GUARD_materialProperties_h__
#define __INCLUDE_GUARD_materialProperties_h__
#include <string>
#include "vulkanContext.h"
#include "vulkanPipeline.h"
#include "vulkanUniformBuffer.h"
#include "vulkanSampler.h"
#include "texture2d.h"



template<typename T>
struct ResourceBinding
{
	uint32_t binding;
	T resource;
	std::vector<std::string> frameNames;
};



class MaterialProperties
{
public: // Members:
	std::unordered_map<std::string, ResourceBinding<VulkanUniformBuffer>> uniformBufferMap;
	std::unordered_map<std::string, ResourceBinding<VulkanSampler*>> samplerMap;
	std::unordered_map<std::string, ResourceBinding<Texture2d*>> texture2dMap;
	std::vector<VkDescriptorSet> descriptorSets;
	VulkanPipeline* pipeline;

private: // Members:
	VulkanContext* context;
	std::vector<VkWriteDescriptorSet> descriptorWrites;

public: // Methods:
	MaterialProperties();
	MaterialProperties(VulkanContext* context, VulkanPipeline* pipeline);
	~MaterialProperties();

	// Initializers:
	void InitUniformObjectResourceBinding(std::string name, uint32_t binding, const UniformObject& uniformObject);
	void InitSamplerResourceBinding(std::string name, uint32_t binding, VulkanSampler* sampler);
	void InitTexture2dResourceBinding(std::string name, uint32_t binding, Texture2d* texture2d);
	void InitDescriptorSets();

	// Setters:
	void SetContext(VulkanContext* context);
	void SetPipeline(VulkanPipeline* pipeline);
	void SetUniformBuffer(const std::string& name, const UniformObject& data);
	void SetSampler(const std::string& name, VulkanSampler* sampler);
	void SetTexture2d(const std::string& name, Texture2d* texture2d);

	// Getters:
	MaterialProperties GetCopy();

private: // Methods:
	void CreateDescriptorSets();
	void UpdateAllDescriptorSets(uint32_t frameIndex);
	void UpdateUniformBufferDescriptorSets(uint32_t frameIndex);
	void UpdateSamplerDescriptorSets(uint32_t frameIndex);
	void UpdateTexture2dDescriptorSets(uint32_t frameIndex);
};



#endif // __INCLUDE_GUARD_materialProperties_h__