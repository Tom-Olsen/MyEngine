#ifndef __INCLUDE_GUARD_application_h__
#define __INCLUDE_GUARD_application_h__
#include <memory>
#include <vector>
#include <chrono>
#include "sdlWindow.h"
#include "vulkanSurface.h"
#include "vulkanSwapchain.h"
#include "vulkanRenderpass.h"
#include "vulkanPipeline.h"
#include "vulkanFrameBuffers.h"
#include "vulkanCommands.h"
#include "vulkanVertexBuffer.h"
#include "vulkanIndexBuffer.h"
#include "vulkanUniformBuffer.h"
#include "vulkanDescriptorPool.h"
#include "vulkanSampler.h"
#include "texture2d.h"
#include "vulkanDepthImage.h"
#include "vulkanMsaaImage.h"
#include "mesh.h"



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
	// Vulkan objects:
	std::unique_ptr<SdlWindow> window;
	std::unique_ptr<VulkanInstance> instance;
	std::unique_ptr<VulkanPhysicalDevice> physicalDevice;
	std::unique_ptr<VulkanLogicalDevice> logicalDevice;
	std::unique_ptr<VulkanSurface> surface;
	std::unique_ptr<VulkanSwapchain> swapchain;
	std::unique_ptr<VulkanSwapchain> oldSwapchain;
	std::unique_ptr<VulkanDepthImage> depthImage;
	std::unique_ptr<VulkanMsaaImage> msaaImage;
	std::unique_ptr<VulkanRenderpass> renderpass;
	std::unique_ptr<VulkanPipelineLayout> pipelineLayout;
	std::unique_ptr<VulkanPipeline> pipeline;
	std::unique_ptr<VulkanFrameBuffers> frameBuffers;
	std::unique_ptr<VulkanCommands> commands;
	std::unique_ptr<VulkanDescriptorPool> descriptorPool;
	std::vector<VkFence> fences;
	std::vector<VkSemaphore> acquireSemaphores;
	std::vector<VkSemaphore> releaseSemaphores;

	// Data:
	std::unique_ptr<Mesh> mesh;
	std::unique_ptr<VulkanVertexBuffer> vertexBuffer;
	std::unique_ptr<VulkanIndexBuffer> indexBuffer;
	std::vector<VulkanUniformBuffer> uniformBuffers;
	std::unique_ptr<VulkanSampler> sampler;
	std::unique_ptr<Texture2d> texture2d;

	GlobalUniformObject gloabalUbo;

	// Render management:
	const size_t framesInFlight = 2;
	double time;
	uint32_t frameIndex;
	uint32_t imageIndex;
	bool rebuildSwapchain;

public: // Methods:
	Application();
	~Application();
	void Run();

private: // Methods:
	void PrintApplicationStatus();
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