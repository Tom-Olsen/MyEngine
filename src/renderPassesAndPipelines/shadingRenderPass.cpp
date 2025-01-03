#include "shadingRenderPass.h"
#include "vmaImage.h"
#include "vulkanCommand.h"
#include "vulkanContext.h"
#include "vulkanMacros.h"



// Constructor/Destructor:
ShadingRenderPass::ShadingRenderPass(VulkanContext* pContext)
{
	m_pContext = pContext;

	CreateRenderPass();
	CreateMsaaImage();
	CreateDepthImage();
	CreateFrameBuffers();
}
ShadingRenderPass::~ShadingRenderPass()
{

}



// Public methods:
const VmaImage* const ShadingRenderPass::GetMsaaVmaImage() const
{
	return m_msaaImage.get();
}
const VmaImage* const ShadingRenderPass::GetDepthVmaImage() const
{
	return m_depthImage.get();
}



// Private methods:
void ShadingRenderPass::CreateRenderPass()
{
	// Attachments:
	std::array<VkAttachmentDescription, 3> attachments{};
	{
		// Multisampled color attachment description:
		attachments[0].format = m_pContext->pSurface->GetVkSurfaceFormatKHR().format;
		attachments[0].samples = m_pContext->msaaSamples;							// multisampling count
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// clear framebuffer to black before rendering
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;				// no need to store multisampls after render
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;			// do not use stencils
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		// do not use stencils
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;				// we don't care about initial layout of the image
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// multisampled images are stored in color layout and not rdy for presenting yet

		// Depth attachment description:
		attachments[1].format = m_depthFormat;									// must be same as depth image format
		attachments[1].samples = m_pContext->msaaSamples;							// msaaSamples
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// clear depth buffer before rendering
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;				// depth content is discarded after rendering
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;			// stencil part not used yet
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		// stencil part not used yet
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Color resolve attachment description: (resolve multisampled fragments)
		attachments[2].format = m_pContext->pSurface->GetVkSurfaceFormatKHR().format;
		attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	// rdy for presenting
	}

	// Attachment references:
	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorResolveAttachmentReference = {};
	colorResolveAttachmentReference.attachment = 2;
	colorResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Subpass:
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // vulkan may also support compute subpasses in the future
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;
	subpass.pResolveAttachments = &colorResolveAttachmentReference;

	// Synchronization dependencies of individual subpasses:
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;	// index of source subpass, where dependency originates. VK_SUBPASS_EXTERNAL = before renderpass
	dependency.dstSubpass = 0;	// index of destination subpass, where dependency ends.  VK_SUBPASS_EXTERNAL = after renderpass
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	// pipeline stages in source subpass which must complete before the dependency is resolved
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	// pipeline stages in destination subpass which must wait for the source stages to complete
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;			// types of memory accesses in the source subpass that must be completed
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;			// types of memory accesses in the destination subpass that must wait on the source subpass to complete
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;					// specify special behaviors

	VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VKA(vkCreateRenderPass(m_pContext->GetVkDevice(), &renderPassInfo, nullptr, &m_renderPass));
}
void ShadingRenderPass::CreateMsaaImage()
{
	VkImageSubresourceRange* pSubresourceRange = new VkImageSubresourceRange();
	pSubresourceRange->aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	pSubresourceRange->baseMipLevel = 0;
	pSubresourceRange->levelCount = 1;
	pSubresourceRange->baseArrayLayer = 0;
	pSubresourceRange->layerCount = 1;

	VkImageCreateInfo* pImageInfo = new VkImageCreateInfo();
	pImageInfo->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	pImageInfo->imageType = VK_IMAGE_TYPE_2D;
	pImageInfo->extent.width = m_pContext->pSurface->GetCurrentExtent().width;
	pImageInfo->extent.height = m_pContext->pSurface->GetCurrentExtent().height;
	pImageInfo->extent.depth = 1;
	pImageInfo->mipLevels = 1;
	pImageInfo->arrayLayers = 1;
	pImageInfo->format = m_pContext->pSurface->GetVkSurfaceFormatKHR().format;
	pImageInfo->tiling = VK_IMAGE_TILING_OPTIMAL;
	pImageInfo->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	pImageInfo->usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	pImageInfo->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	pImageInfo->samples = m_pContext->msaaSamples;
	pImageInfo->flags = 0;

	VmaAllocationCreateInfo* pAllocationInfo = new VmaAllocationCreateInfo();
	pAllocationInfo->usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	pAllocationInfo->flags = 0;
	pAllocationInfo->requiredFlags = 0;
	pAllocationInfo->preferredFlags = 0;

	m_msaaImage = std::make_unique<VmaImage>(m_pContext, pImageInfo, pAllocationInfo, pSubresourceRange);
}
void ShadingRenderPass::CreateDepthImage()
{
	VkImageSubresourceRange* pSubresourceRange = new VkImageSubresourceRange();
	pSubresourceRange->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	pSubresourceRange->baseMipLevel = 0;
	pSubresourceRange->levelCount = 1;
	pSubresourceRange->baseArrayLayer = 0;
	pSubresourceRange->layerCount = 1;

	VkImageCreateInfo* pImageInfo = new VkImageCreateInfo();
	pImageInfo->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	pImageInfo->imageType = VK_IMAGE_TYPE_2D;
	pImageInfo->extent.width = m_pContext->pSurface->GetCurrentExtent().width;
	pImageInfo->extent.height = m_pContext->pSurface->GetCurrentExtent().height;
	pImageInfo->extent.depth = 1;
	pImageInfo->mipLevels = 1;
	pImageInfo->arrayLayers = 1;
	pImageInfo->format = m_depthFormat;
	pImageInfo->tiling = VK_IMAGE_TILING_OPTIMAL;
	pImageInfo->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	pImageInfo->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	pImageInfo->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	pImageInfo->samples = m_pContext->msaaSamples;
	pImageInfo->flags = 0;

	VmaAllocationCreateInfo* pAllocationInfo = new VmaAllocationCreateInfo();
	pAllocationInfo->usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	pAllocationInfo->flags = 0;
	pAllocationInfo->requiredFlags = 0;
	pAllocationInfo->preferredFlags = 0;

	m_depthImage = std::make_unique<VmaImage>(m_pContext, pImageInfo, pAllocationInfo, pSubresourceRange);

	// Transition depth image layout:
	VulkanCommand command = VulkanCommand::BeginSingleTimeCommand(m_pContext, m_pContext->pLogicalDevice->GetGraphicsQueue());
	{
		VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.srcAccessMask = VK_ACCESS_NONE;					// types of memory access allowed before the barrier
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;	// types of memory access allowed after the barrier
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_depthImage->GetVkImage();
		barrier.subresourceRange = *pSubresourceRange;

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;	// Immediatly
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;		// early fragment test stage
		vkCmdPipelineBarrier(
			command.GetVkCommandBuffer(),
			srcStage, dstStage,
			0,	// dependency flags, typically 0
			0, nullptr,				// memory barriers
			0, nullptr,	// buffer memory barriers
			1, &barrier);	// image memory barriers
	}
	VulkanCommand::EndSingleTimeCommand(m_pContext, command, m_pContext->pLogicalDevice->GetGraphicsQueue());
}
void ShadingRenderPass::CreateFrameBuffers()
{
	size_t size = m_pContext->pSwapchain->GetImages().size();
	VkExtent2D extent = m_pContext->pSurface->GetCurrentExtent();
	m_framebuffers.resize(size);
	std::array<VkImageView, 3> attachments;

	for (size_t i = 0; i < size; i++)
	{
		// order of attachments is important!
		attachments[0] = m_msaaImage->GetVkImageView();
		attachments[1] = m_depthImage->GetVkImageView();
		attachments[2] = m_pContext->pSwapchain->GetImageViews()[i];

		VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;
		vkCreateFramebuffer(m_pContext->GetVkDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]);
	}
}