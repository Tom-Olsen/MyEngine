#ifndef __INCLUDE_GUARD_vulkanPipeline_h__
#define __INCLUDE_GUARD_vulkanPipeline_h__
#include <string>
#include "vulkanContext.h"
#include "forwardRenderPass.h"
#include "glmTypes.h"
#include "pushConstantObject.h"



/// <summary>
/// Pipeline is immutable and must be recreated from scratch if any of its state changes.
/// It is also expensive to create, so it is recommended to create them at initialization and not during rendering.
/// The only exception are the dynamic states, which can be changed without recreating the pipeline.
/// These include viewport, scissor, line width, depth bias, blend constants, and stencil compare mask.
/// </summary>
class VulkanPipeline
{
public: // Members:
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

private: // Members:
	VulkanContext* context;

public: // Methods:
	VulkanPipeline(VulkanContext* context, VkRenderPass* renderPass,
		const std::vector<char>& vertexCode,
		const std::vector<char>& fragmentCode,
		const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	~VulkanPipeline();

private: // Methods:
	void CreatePipelineLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	void CreatePipeline(VkRenderPass* renderPass, const VkShaderModule& vertexShaderModule, const VkShaderModule& fragmentShaderModule);
};



#endif // __INCLUDE_GUARD_vulkanPipeline_h__