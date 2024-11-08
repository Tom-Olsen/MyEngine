#ifndef __INCLUDE_GUARD_vulkanCommands_h__
#define __INCLUDE_GUARD_vulkanCommands_h__
#include <vulkan/vulkan.h>
#include <vector>
#include "vulkanContext.h"



/// <summary>
/// VulkanCommand class owns command pools and command buffers.
/// </summary>
class VulkanCommand
{
public: // Members:
	VkCommandPool pool;
	VkCommandBuffer buffer;

private: // Members:
	VulkanContext* context;

public: // Methods:
	VulkanCommand(VulkanContext* context, VulkanQueue queue);
	~VulkanCommand();

	static VulkanCommand BeginSingleTimeCommand(VulkanContext* context, const VulkanQueue& queue);
	static void EndSingleTimeCommand(VulkanContext* context, const VulkanCommand& command, const VulkanQueue& queue);
};



#endif // __INCLUDE_GUARD_vulkanCommands_h__