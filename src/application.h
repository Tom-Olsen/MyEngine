#ifndef __INCLUDE_GUARD_application_h__
#define __INCLUDE_GUARD_application_h__
#include <memory>
#include <vector>
#include <chrono>
#include "sdlWindow.h"
#include "vulkanInstance.h"
#include "vulkanPhysicalDevice.h"
#include "vulkanSurface.h"
#include "vulkanLogicalDevice.h"
#include "vulkanMemoryAllocator.h"
#include "vulkanContext.h"
#include "vulkanDescriptorPool.h"
#include "vulkanSwapchain.h"
#include "vulkanRenderpass.h"
#include "vulkanMsaaImage.h"
#include "vulkanDepthImage.h"
#include "vulkanFrameBuffers.h"
#include "vulkanCommand.h"
#include "mesh.h"
#include "vulkanUniformBuffer.h"
#include "vulkanSampler.h"
#include "texture2d.h"
#include "material.h"



/// <summary>
/// Main application class.
/// Contains:
/// -Vulkan objects
/// -Render loop
/// -Swapchain management
/// -Command buffer management
/// Coming:
/// -Game loop
/// -Input handling
/// </summary>
class Application
{
private: // Members:
	// Vulkan context:
	std::unique_ptr<SdlWindow> window;
	std::unique_ptr<VulkanInstance> instance;
	std::unique_ptr<VulkanPhysicalDevice> physicalDevice;
	std::unique_ptr<VulkanSurface> surface;
	std::unique_ptr<VulkanLogicalDevice> logicalDevice;
	std::unique_ptr<VulkanMemoryAllocator> allocator;
	std::unique_ptr<VulkanContext> context;

	// Other:
	std::unique_ptr<VulkanDescriptorPool> descriptorPool;
	std::unique_ptr<VulkanSwapchain> swapchain;
	std::unique_ptr<VulkanRenderpass> renderpass;

	// Render resources:
	std::unique_ptr<VulkanMsaaImage> msaaImage;
	std::unique_ptr<VulkanDepthImage> depthImage;
	std::unique_ptr<VulkanFrameBuffers> frameBuffers;
	std::vector<VulkanCommand> commands;

	// Sync objects:
	std::vector<VkFence> fences;
	std::vector<VkSemaphore> acquireSemaphores;
	std::vector<VkSemaphore> releaseSemaphores;

	// Data:
	std::unique_ptr<Mesh> mesh;
	std::unique_ptr<Material> material;
	MaterialProperties materialProperties;
	std::unique_ptr<VulkanSampler> sampler;
	std::unique_ptr<Texture2d> texture2d;
	UniformObject uniformObject;

	// Render management:
	const uint32_t framesInFlight = 2;
	double time;
	uint32_t imageIndex;
	bool rebuildSwapchain;

public: // Methods:
	Application();
	~Application();
	void Run();

private: // Methods:
	void Render();
	bool AcquireImage();
	void RecordCommandBuffer();
	void SubmitCommandBuffer();
	bool PresentImage();
	void SetViewportAndScissor(VkCommandBuffer& commandBuffer);
	void ResizeSwapchain();
	void CreateFences();
	void CreateSemaphores();
	void DestroyFences();
	void DestroySemaphores();
};



#endif // __INCLUDE_GUARD_application_h__