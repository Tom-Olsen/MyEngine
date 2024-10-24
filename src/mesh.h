#ifndef __INCLUDE_GUARD_mesh_h__
#define __INCLUDE_GUARD_mesh_h__
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "glmTypes.h"
#include "vulkanContext.h"
#include "vmaBuffer.h"


// TODO:
// - add logic to only update the parts of the buffer that have changed (e.g. pos, normal, ...)

/// <summary>
/// Mesh class for storing vertex positions, colors and triangles.
/// Mesh class takes ownership of vectors and takes care of cleanup.
/// Static meshes should be stored as pointers in the static MeshManager class.
/// </summary>
class Mesh
{
public: // Members:
	std::string name;

private: // Members:
	bool isLoaded = false;
	bool verticesUpdated = false;
	bool indicesUpdated = false;
	uint32_t vertexCount = 0;
	uint32_t triangleCount = 0;
	std::unique_ptr<VmaBuffer> vertexBuffer;
	std::unique_ptr<VmaBuffer> indexBuffer;
	std::vector<Float3> positions;
	std::vector<Float3> normals;
	std::vector<Float4> colors;
	std::vector<Float4> uvs;
	std::vector<Int3> triangles;

public: // Methods:
	Mesh(std::string name = "");
	Mesh(const Mesh& other) = default;
	Mesh& operator=(const Mesh& other) = default;
	Mesh(Mesh&& other) noexcept = default;
	Mesh& operator=(Mesh&& other) noexcept = default;
	~Mesh();

	// Load/Unload:
	void Load(VulkanContext* context);
	void Unload();
	
	// Setters:
	void SetPositions(std::vector<Float3>& positions);
	void SetNormals(std::vector<Float3>& normals);
	void SetColors(std::vector<Float4>& colors);
	void SetUVs(std::vector<Float4>& uvs);
	void SetTriangles(std::vector<Int3>& triangles);

	// Movers:
	void MovePositions(std::vector<Float3>& positions);
	void MoveNormals(std::vector<Float3>& normals);
	void MoveColors(std::vector<Float4>& colors);
	void MoveUVs(std::vector<Float4>& uvs);
	void MoveTriangles(std::vector<Int3>& triangles);

	// Getters:
	uint32_t GetVertexCount() const;
	uint32_t GetTriangleCount() const;
	std::vector<Float3>& GetPositions();
	std::vector<Float3>& GetNormals();
	std::vector<Float4>& GetColors();
	std::vector<Float4>& GetUVs();
	std::vector<Int3>& GetTriangles();
	uint32_t* GetTrianglesUnrolled();
	uint32_t GetSizeOfPositions() const;
	uint32_t GetSizeOfNormals() const;
	uint32_t GetSizeOfColors() const;
	uint32_t GetSizeOfUVs() const;
	uint32_t GetSizeOfTriangles() const;
	std::vector<VkDeviceSize> GetOffsets();
	VmaBuffer* GetVertexBuffer(VulkanContext* context);
	VmaBuffer* GetIndexBuffer(VulkanContext* context);
	bool IsLoaded();
	Mesh* GetCopy(std::string newName = "");

	// Mesh transformation:
	Mesh* Translate(Float3 translation);
	Mesh* Rotate(Float3 eulerAngles);
	Mesh* Scale(Float3 scale);

	// Static methods:
	static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	static VkIndexType GetIndexType();
	static uint32_t GetBindingCount();
	static Mesh* Merge(std::vector<Mesh*>& meshes, std::string name = "");

	// Debugging:
	std::string ToString();

private: // Methods:
	void UpdateVertexBuffer(VulkanContext* context);
	void UpdateIndexBuffer(VulkanContext* context);
};



#endif // __INCLUDE_GUARD_mesh_h__