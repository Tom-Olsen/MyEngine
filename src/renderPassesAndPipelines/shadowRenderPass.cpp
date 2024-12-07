#include "shadowRenderPass.h"
#include "vulkanMacros.h"
#include "macros.h"
#include <fstream>



// static members:
uint32_t ShadowRenderPass::shadowMapWidth = 1024;
uint32_t ShadowRenderPass::shadowMapHeight = 1024;
uint32_t ShadowRenderPass::layerCount = MAX_D_LIGHTS + MAX_S_LIGHTS + 6 * MAX_P_LIGHTS;



// Constructor:
ShadowRenderPass::ShadowRenderPass(VulkanContext* context)
{
	this->context = context;

	CreateShadowMapTexture();
	CreateRenderpass();
	CreateFramebuffers();
}



// Destructor:
ShadowRenderPass::~ShadowRenderPass()
{

}



// Private:
void ShadowRenderPass::CreateShadowMapTexture()
{
	// Subresource range:
	VkImageSubresourceRange subresourceRange;
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = layerCount;

	// Image info:
	VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = shadowMapWidth;
	imageInfo.extent.height = shadowMapHeight;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = layerCount;
	imageInfo.format = shadowMapFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	// Allocation info:
	VmaAllocationCreateInfo allocationInfo = {};
	allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocationInfo.flags = 0;
	allocationInfo.requiredFlags = 0;
	allocationInfo.preferredFlags = 0;

	VmaImage* image = new VmaImage(context, imageInfo, allocationInfo, subresourceRange);
	shadowMaps = std::make_unique<Texture2d>(context, image, "shadowMaps");
}
void ShadowRenderPass::CreateRenderpass()
{
	// Attachment description:
	VkAttachmentDescription attachment = {};
	attachment.format = shadowMapFormat;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// clear framebuffer to black before rendering
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;					// store for later render passes
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;			// do not use stencils
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		// do not use stencils
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;				// we don't care about initial layout of the image
	attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// will be read in a shader

	// Attachment reference:
	VkAttachmentReference shadowMapAttachmentReference = {};
	shadowMapAttachmentReference.attachment = 0;
	shadowMapAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Subpass description:
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pDepthStencilAttachment = &shadowMapAttachmentReference;

	// Create render pass:
	VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &attachment;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	VKA(vkCreateRenderPass(context->GetVkDevice(), &createInfo, nullptr, &renderPass));
}
void ShadowRenderPass::CreateFramebuffers()
{
	// One framebuffer per swapchain image:
	framebuffers.resize(context->framesInFlight);
	for (uint32_t i = 0; i < context->framesInFlight; i++)
	{
		VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &shadowMaps->image->GetVkImageView();
		framebufferInfo.width = shadowMapWidth;
		framebufferInfo.height = shadowMapHeight;
		framebufferInfo.layers = layerCount;
		vkCreateFramebuffer(context->GetVkDevice(), &framebufferInfo, nullptr, &framebuffers[i]);
	}
}