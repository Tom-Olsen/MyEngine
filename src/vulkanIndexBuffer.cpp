#include "vulkanIndexBuffer.h"
#include "macros.h"



// Constructor:
#ifdef RESIZEABLE_BAR // No staging buffer:
VulkanIndexBuffer::VulkanIndexBuffer(VulkanLogicalDevice* logicalDevice, VulkanPhysicalDevice* physicalDevice, Mesh* mesh)
{
	this->logicalDevice = logicalDevice;
	this->physicalDevice = physicalDevice;

	// Create buffer:
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	VkDeviceSize bufferSize = 3 * mesh->SizeOfTriangles();
	this->buffer = std::make_unique<VulkanBuffer>(logicalDevice, physicalDevice, bufferSize, usage, memoryPropertyFlags);

	// Copy triangle indexes:
	void* data;
	VKA(vkMapMemory(logicalDevice->device, buffer->memory, 0, bufferSize, 0, &data));
	memcpy(data, mesh->GetTrianglesUnrolled(), bufferSize);
	vkUnmapMemory(logicalDevice->device, buffer->memory);
}
void VulkanIndexBuffer::UpdateBuffer(Mesh* mesh)
{
	VkDeviceSize bufferSize = mesh->SizeOfBuffer();
	vkQueueWaitIdle(logicalDevice->graphicsQueue.queue);	// wait for previous render calls to finish

	// Old buffer is big enough:
	if (bufferSize != buffer->size)
	{
		// Copy triangle indexes:
		void* data;
		VKA(vkMapMemory(logicalDevice->device, buffer->memory, 0, bufferSize, 0, &data));
		memcpy(data, mesh->GetTrianglesUnrolled(), bufferSize);
		vkUnmapMemory(logicalDevice->device, buffer->memory);
	}
	// Old buffer is too small:
	else
	{
		// Recreate buffer:
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		VkDeviceSize bufferSize = 3 * mesh->SizeOfTriangles();
		this->buffer = std::make_unique<VulkanBuffer>(logicalDevice, physicalDevice, bufferSize, usage, memoryPropertyFlags);

		// Copy triangle indexes:
		void* data;
		VKA(vkMapMemory(logicalDevice->device, buffer->memory, 0, bufferSize, 0, &data));
		memcpy(data, mesh->GetTrianglesUnrolled(), bufferSize);
		vkUnmapMemory(logicalDevice->device, buffer->memory);
	}
}
#else // With Staging buffer:
VulkanIndexBuffer::VulkanIndexBuffer(VulkanLogicalDevice* logicalDevice, VulkanPhysicalDevice* physicalDevice, Mesh* mesh)
{
	this->logicalDevice = logicalDevice;
	this->physicalDevice = physicalDevice;

	// Create buffer:
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkDeviceSize bufferSize = 3 * mesh->GetSizeOfTriangles();
	std::vector<uint32_t> queueFamilyIndices = { logicalDevice->graphicsQueue.familyIndex, logicalDevice->transferQueue.familyIndex };
	this->buffer = std::make_unique<VulkanBuffer>(logicalDevice, physicalDevice, bufferSize, usage, memoryPropertyFlags, queueFamilyIndices);

	// Load data into staging buffer:
	VulkanBuffer stagingBuffer(logicalDevice, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data;
	VKA(vkMapMemory(logicalDevice->device, stagingBuffer.memory, 0, bufferSize, 0, &data));
	memcpy(data, mesh->GetTrianglesUnrolled(), bufferSize);
	vkUnmapMemory(logicalDevice->device, stagingBuffer.memory);

	// Copy data from staging to vertex buffer:
	VulkanBuffer::CopyBuffer(logicalDevice, &stagingBuffer, buffer.get(), bufferSize);
}
void VulkanIndexBuffer::UpdateBuffer(Mesh* mesh)
{
	VkDeviceSize bufferSize = 3 * mesh->GetSizeOfTriangles();
	vkQueueWaitIdle(logicalDevice->graphicsQueue.queue);	// wait for previous render calls to finish

	// Old buffer is big enough:
	if (bufferSize != buffer->size)
	{
		// Load data into staging buffer:
		VulkanBuffer stagingBuffer(logicalDevice, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		void* data;
		VKA(vkMapMemory(logicalDevice->device, stagingBuffer.memory, 0, bufferSize, 0, &data));
		memcpy(data, mesh->GetTrianglesUnrolled(), bufferSize);
		vkUnmapMemory(logicalDevice->device, stagingBuffer.memory);

		// Copy data from staging to vertex buffer:
		VulkanBuffer::CopyBuffer(logicalDevice, &stagingBuffer, buffer.get(), bufferSize);
	}
	// Old buffer is too small:
	else
	{
		// Recreate buffer:
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		VkDeviceSize bufferSize = 3 * mesh->GetSizeOfTriangles();
		std::vector<uint32_t> queueFamilyIndices = { logicalDevice->graphicsQueue.familyIndex, logicalDevice->transferQueue.familyIndex };
		this->buffer = std::make_unique<VulkanBuffer>(logicalDevice, physicalDevice, bufferSize, usage, memoryPropertyFlags, queueFamilyIndices);

		// Load data into staging buffer:
		VulkanBuffer stagingBuffer(logicalDevice, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		void* data;
		VKA(vkMapMemory(logicalDevice->device, stagingBuffer.memory, 0, bufferSize, 0, &data));
		memcpy(data, mesh->GetTrianglesUnrolled(), bufferSize);
		vkUnmapMemory(logicalDevice->device, stagingBuffer.memory);

		// Copy data from staging to vertex buffer:
		VulkanBuffer::CopyBuffer(logicalDevice, &stagingBuffer, buffer.get(), bufferSize);
	}
}
#endif



// Destructor:
VulkanIndexBuffer::~VulkanIndexBuffer()
{

}