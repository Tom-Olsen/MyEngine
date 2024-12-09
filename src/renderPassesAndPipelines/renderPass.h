#ifndef __INCLUDE_GUARD_renderPass_h__
#define __INCLUDE_GUARD_renderPass_h__
#include <vector>
#include <vulkan/vulkan.h>



struct VulkanContext;



/// <summary>
/// Polymorphic parent class for all renderPasses.
/// </summary>
class RenderPass
{
protected: // Members:
	VkRenderPass m_renderPass;
	std::vector<VkFramebuffer> m_framebuffers;
	VulkanContext* m_pContext;

public: // Methods:
	virtual ~RenderPass();
	const VkRenderPass& GetVkRenderPass() const;
	const std::vector<VkFramebuffer>& GetFramebuffers() const;
};



#endif // __INCLUDE_GUARD_renderPass_h__