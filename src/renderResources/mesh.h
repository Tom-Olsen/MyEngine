#ifndef __INCLUDE_GUARD_mesh_h__
#define __INCLUDE_GUARD_mesh_h__
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "mathf.h"
#include "vulkanContext.h"
#include "vmaBuffer.h"


// TODO:
// - add logic to only update the parts of the buffer that have changed (e.g. pos, normal, ...)

/// <summary>
/// Mesh class for storing vertex positions, colors, triangles, etc.
/// Mesh class takes ownership of vectors and takes care of cleanup.
/// Normalization of normals and tangets must be done manually.
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
	std::vector<Float3> tangents;
	std::vector<Float4> colors;
	std::vector<Float4> uvs;
	std::vector<Uint3> triangles;
	VkDeviceSize offsets[5];
	VkBuffer buffers[5];

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
	void SetTangents(std::vector<Float3>& tangents);
	void SetColors(std::vector<Float4>& colors);
	void SetUniformColor(const Float4& color);
	void SetUVs(std::vector<Float4>& uvs);
	void SetTriangles(std::vector<Uint3>& triangles);

	// Movers:
	void MovePositions(std::vector<Float3>& positions);
	void MoveNormals(std::vector<Float3>& normals);
	void MoveTangents(std::vector<Float3>& tangents);
	void MoveColors(std::vector<Float4>& colors);
	void MoveUVs(std::vector<Float4>& uvs);
	void MoveTriangles(std::vector<Uint3>& triangles);

	// Getters:
	uint32_t GetVertexCount() const;
	uint32_t GetTriangleCount() const;
	std::vector<Float3>& GetPositions();
	std::vector<Float3>& GetNormals();
	std::vector<Float3>& GetTangents();
	std::vector<Float4>& GetColors();
	std::vector<Float4>& GetUVs();
	std::vector<Uint3>& GetTriangles();
	uint32_t* GetTrianglesUnrolled();
	uint32_t GetSizeOfPositions() const;
	uint32_t GetSizeOfNormals() const;
	uint32_t GetSizeOfTangents() const;
	uint32_t GetSizeOfColors() const;
	uint32_t GetSizeOfUVs() const;
	uint32_t GetSizeOfTriangles() const;
	uint32_t GetBindingCount();
	VkBuffer* GetBuffers(VulkanContext* context);
	VkDeviceSize* GetOffsets();
	VmaBuffer* GetVertexBuffer(VulkanContext* context);
	VmaBuffer* GetIndexBuffer(VulkanContext* context);
	bool IsLoaded();
	bool HasNormals();
	bool HasTangents();
	bool HasColors();
	bool HasUVs();
	Mesh* GetCopy(std::string newName = "");

	// Mesh transformation (changes *this!):
	Mesh* Translate(const Float3& translation);
	Mesh* Rotate(const Float3x3& rotation);
	Mesh* Rotate(const Float4x4& rotation);
	Mesh* Scale(const Float3& scale);
	Mesh* Scale(float scale);
	Mesh* Subdivide();
	Mesh* Spherify(float factor, float radius);
	void ComputeNormals();
	void ComputeTangents();

	// Static binding descriptions:
	static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
	static VkVertexInputBindingDescription GetPositionBindingDescription();
	static VkVertexInputBindingDescription GetNormalBindingDescription();
	static VkVertexInputBindingDescription GetTangentBindingDescription();
	static VkVertexInputBindingDescription GetColorBindingDescription();
	static VkVertexInputBindingDescription GetUvBindingDescription(uint32_t channel);

	// Static attribute descriptions:
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	static VkVertexInputAttributeDescription GetPositionAttributeDescription();
	static VkVertexInputAttributeDescription GetNormalAttributeDescription();
	static VkVertexInputAttributeDescription GetTangentAttributeDescription();
	static VkVertexInputAttributeDescription GetColorAttributeDescription();
	static VkVertexInputAttributeDescription GetUvAttributeDescription(uint32_t channel);

	// Static methods:
	static VkIndexType GetIndexType();
	static Mesh* Merge(std::vector<Mesh*>& meshes, const std::string& name = "");

	// Debugging:
	std::string ToString();

private: // Methods:
	void UpdateVertexBuffer(VulkanContext* context);
	void UpdateIndexBuffer(VulkanContext* context);
};



#endif // __INCLUDE_GUARD_mesh_h__