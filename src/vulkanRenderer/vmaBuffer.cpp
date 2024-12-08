#include "vmaBuffer.h"
#include "vmaImage.h"
#include "vulkanCommand.h"
#include "vulkanContext.h"
#include "vulkanMacros.h"



// Constructors/Destructor:
VmaBuffer::VmaBuffer()
{
	m_pContext = nullptr;
	m_buffer = VK_NULL_HANDLE;
	m_allocation = VK_NULL_HANDLE;
}
VmaBuffer::VmaBuffer(VulkanContext* pContext, const VkBufferCreateInfo& bufferInfo, const VmaAllocationCreateInfo& allocInfo)
{
	m_pContext = pContext;
	m_bufferInfo = bufferInfo;
	m_allocInfo = allocInfo;
	VKA(vmaCreateBuffer(m_pContext->GetVmaAllocator(), &m_bufferInfo, &m_allocInfo, &m_buffer, &m_allocation, nullptr));
}
VmaBuffer::~VmaBuffer()
{
	vmaDestroyBuffer(m_pContext->GetVmaAllocator(), m_buffer, m_allocation);
}



// Public methods:
// Getters:
const VkBuffer& VmaBuffer::GetVkBuffer() const
{
	return m_buffer;
}
const VmaAllocation& VmaBuffer::GetVmaAllocation() const
{
	return m_allocation;
}
const VkBufferCreateInfo& VmaBuffer::GetVkBufferCreateInfo() const
{
	return m_bufferInfo;
}
const VmaAllocationCreateInfo& VmaBuffer::GetVmaAllocationCreateInfo() const
{
	return m_allocInfo;
}
uint64_t VmaBuffer::GetSize()
{
	return m_bufferInfo.size;
}



// Static methods:
void VmaBuffer::CopyBufferToBuffer(VulkanContext* m_pContext, VmaBuffer* srcBuffer, VmaBuffer* dstBuffer, VkDeviceSize bufferSize, const VulkanQueue& queue)
{
	VulkanCommand command = VulkanCommand::BeginSingleTimeCommand(m_pContext, queue);

	// Queue copy command:
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = bufferSize;
	vkCmdCopyBuffer(command.GetVkCommandBuffer(), srcBuffer->GetVkBuffer(), dstBuffer->GetVkBuffer(), 1, &copyRegion);

	VulkanCommand::EndSingleTimeCommand(m_pContext, command, queue);
}
void VmaBuffer::CopyBufferToImage(VulkanContext* m_pContext, VmaBuffer* srcBuffer, VmaImage* dstImage, const VulkanQueue& queue, uint32_t layerCount)
{
	VulkanCommand command = VulkanCommand::BeginSingleTimeCommand(m_pContext, queue);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = layerCount;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = dstImage->GetExtent();

	vkCmdCopyBufferToImage(
		command.GetVkCommandBuffer(),
		srcBuffer->GetVkBuffer(),
		dstImage->GetVkImage(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region);

	VulkanCommand::EndSingleTimeCommand(m_pContext, command, queue);
}
VmaBuffer VmaBuffer::StagingBuffer(VulkanContext* m_pContext, uint64_t size, void* inputData)
{
	// Create buffer:
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY; // no need for CPU-GPU synchronization
	allocInfo.flags = 0;
	allocInfo.requiredFlags = 0;
	allocInfo.preferredFlags = 0;

	VmaBuffer stagingBuffer = VmaBuffer(m_pContext, bufferInfo, allocInfo);

	// Load data into buffer:
	void* data;
	VKA(vmaMapMemory(m_pContext->GetVmaAllocator(), stagingBuffer.GetVmaAllocation(), &data));
	memcpy(data, inputData, static_cast<size_t>(size));
	vmaUnmapMemory(m_pContext->GetVmaAllocator(), stagingBuffer.GetVmaAllocation());

	return stagingBuffer;
}
VmaBuffer VmaBuffer::StagingBuffer(VulkanContext* m_pContext, const std::vector<uint64_t>& sizes, const std::vector<void*>& inputDatas)
{
	// Check if sizes and inputDatas have the same size:
	if (sizes.size() != inputDatas.size())
		throw std::runtime_error("sizes and inputDatas must have the same size!");

	// Calculate total buffer size = sum of all buffer sizes:
	uint64_t totalSize = 0;
	for (uint64_t size : sizes)
		totalSize += size;

	// Create buffer:
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = totalSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY; // no need for CPU-GPU synchronization
	allocInfo.flags = 0;
	allocInfo.requiredFlags = 0;
	allocInfo.preferredFlags = 0;

	VmaBuffer stagingBuffer = VmaBuffer(m_pContext, bufferInfo, allocInfo);

	// Load data into buffer:
	void* data;
	VKA(vmaMapMemory(m_pContext->GetVmaAllocator(), stagingBuffer.GetVmaAllocation(), &data));
	uint64_t offset = 0;
	for (uint64_t i = 0; i < inputDatas.size(); i++)
	{
		if (inputDatas[i] != nullptr && sizes[i] > 0)
		{
			memcpy(static_cast<char*>(data) + offset, inputDatas[i], static_cast<size_t>(sizes[i]));
			offset += sizes[i];
		}
	}
	vmaUnmapMemory(m_pContext->GetVmaAllocator(), stagingBuffer.GetVmaAllocation());

	return stagingBuffer;
}