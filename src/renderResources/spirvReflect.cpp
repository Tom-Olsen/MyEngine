#include "spirvReflect.h"
#include "logger.h"



// Macro to check if a SPIRV-Reflect function returns SPV_REFLECT_RESULT_SUCCESS.
#define SPIRV_REFLECT_ASSERT(val) \
    if (val != SPV_REFLECT_RESULT_SUCCESS) { \
        LOG_CRITICAL("File: {}, Line: {} SPIRV-Reflect error: {}", __FILE__, __LINE__, std::to_string(val)); \
        std::abort(); \
    }

// Shorthand for SPIRV_REFLECT_ASSERT which can be disabled:
#ifndef SPVA
#define SPVA(f) SPIRV_REFLECT_ASSERT(f)
#endif



// UniformBufferMember public methods:
void UniformBufferMember::AddSubMember(std::string name, UniformBufferMember * subMember)
{
    // Take ownership of UniformBufferMember pointer:
    subMembers.emplace(name, subMember);
}
UniformBufferMember* UniformBufferMember::GetSubMember(const std::string & name) const
{
    auto it = subMembers.find(name);
    if (it != subMembers.end())
        return it->second;
    return nullptr;
}
std::string UniformBufferMember::ToString(const std::string& name, int indent) const
{
	std::string output(indent, ' ');
    output += name + ": offset=" + std::to_string(offset) + ", size=" + std::to_string(size) + "\n";
    for (const auto& [subName, subMember] : subMembers)
		output += subMember->ToString(subName, indent + 2);
	return output;
}


// UniformBufferBlock Constructor/Destructor:
UniformBufferBlock::UniformBufferBlock(const std::string& name, uint32_t size, uint32_t setIndex, uint32_t bindingIndex)
{
	this->name = name;
    this->size = size;
	this->setIndex = setIndex;
	this->bindingIndex = bindingIndex;
}
UniformBufferBlock::~UniformBufferBlock()
{
	for (auto& member : members)
		delete member.second;
}



// UniformBufferBlock public methods:
void UniformBufferBlock::AddMember(std::string name, UniformBufferMember* member)
{
    // Take ownership of UniformBufferMember pointer:
    members.emplace(name, member);

	// Adjust size:
    uint32_t newSize = member->offset + member->size;
    if (newSize > size)
        size = newSize;
}
UniformBufferMember* UniformBufferBlock::GetMember(const std::string& name) const
{
    auto it = members.find(name);
    if (it != members.end())
        return it->second;
    return nullptr;
}
std::string UniformBufferBlock::ToString() const
{
    std::string output = name + "(binding=" + std::to_string(bindingIndex) + ", size=" + std::to_string(size) + "):\n";
    for (const auto& [name, member] : members)
        output += member->ToString(name, 0);
    return output;
}



// SpirvReflect Constructor/Destructor:
SpirvReflect::SpirvReflect(std::vector<char> code)
{
    SPVA(spvReflectCreateShaderModule(code.size(), code.data(), &module));
}
SpirvReflect::~SpirvReflect()
{
    spvReflectDestroyShaderModule(&module);
}



// SpirvReflect public methods:
void SpirvReflect::GetDescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& bindings, std::vector<std::string>& bindingNames, std::unordered_map<std::string, UniformBufferBlock*>& uniformBufferBlockMap)
{
    // Shader descriptor set reflection:
    std::vector<SpvReflectDescriptorSet*> descriptorSetsReflection = GetDescriptorSetsReflection();

    for (uint32_t setIndex = 0; setIndex < descriptorSetsReflection.size(); setIndex++)
    {
        SpvReflectDescriptorSet* setReflection = descriptorSetsReflection[setIndex];
        for (uint32_t bindingIndex = 0; bindingIndex < setReflection->binding_count; bindingIndex++)
        {
            SpvReflectDescriptorBinding* bindingReflection = setReflection->bindings[bindingIndex];
            VkDescriptorSetLayoutBinding layoutBinding = {};
            layoutBinding.binding = bindingReflection->binding;
            layoutBinding.descriptorType = VkDescriptorType((int)bindingReflection->descriptor_type);
            layoutBinding.descriptorCount = bindingReflection->count;
            layoutBinding.stageFlags = VkShaderStageFlagBits((int)module.shader_stage);
            layoutBinding.pImmutableSamplers = nullptr;

            // Add binding and name to lists:
            bindings.push_back(layoutBinding);
            bindingNames.push_back(bindingReflection->name);

            // In case of uniform buffer create UniformBufferBlock:
            if (bindingReflection->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                SpvReflectBlockVariable& blockReflection = bindingReflection->block;
                UniformBufferBlock* uniformBufferBlock = GetUniformBufferBlock(blockReflection, setReflection->set, bindingReflection->binding);
                uniformBufferBlockMap.emplace(uniformBufferBlock->name, uniformBufferBlock);
            }
        }
    }
}



// SpirvReflect private static methods:
std::vector<SpvReflectDescriptorSet*> SpirvReflect::GetDescriptorSetsReflection() const
{
    uint32_t setCount = 0;
    SPVA(spvReflectEnumerateDescriptorSets(&module, &setCount, nullptr));
    std::vector<SpvReflectDescriptorSet*> sets(setCount);
    SPVA(spvReflectEnumerateDescriptorSets(&module, &setCount, sets.data()));
    return sets;
}
UniformBufferBlock* SpirvReflect::GetUniformBufferBlock(const SpvReflectBlockVariable& blockReflection, uint32_t setIndex, uint32_t bindingIndex) const
{
    UniformBufferBlock* block = new UniformBufferBlock(blockReflection.name, blockReflection.size, setIndex, bindingIndex);

    for (uint32_t memberIndex = 0; memberIndex < blockReflection.member_count; memberIndex++)
    {
        // Create UniformBufferMember:
        SpvReflectBlockVariable& memberReflection = blockReflection.members[memberIndex];
        UniformBufferMember* member = new UniformBufferMember();

        // Base member:
        member->offset = memberReflection.offset;
        member->size = memberReflection.size;

        // Submember struct reflection:
        if (IsStruct(memberReflection) && !IsArray(memberReflection))
			StructReflection(memberReflection, member);

        // Submember array reflection:
        if (IsArray(memberReflection))
			ArrayReflection(memberReflection.name, memberReflection, member);

        block->AddMember(memberReflection.name, member);
    }

    return block;
}
bool SpirvReflect::IsStruct(const SpvReflectBlockVariable& memberReflection) const
{
    return memberReflection.member_count > 0;
}
bool SpirvReflect::IsArray(const SpvReflectBlockVariable& memberReflection) const
{
	return memberReflection.array.dims_count > 0;
}
void SpirvReflect::StructReflection(const SpvReflectBlockVariable& memberReflection, UniformBufferMember* member) const
{
    for (uint32_t subMemberIndex = 0; subMemberIndex < memberReflection.member_count; subMemberIndex++)
    {
        SpvReflectBlockVariable& subMemberReflection = memberReflection.members[subMemberIndex];
        UniformBufferMember* subMember = new UniformBufferMember();

		// Base struct:
        subMember->offset = subMemberReflection.offset + member->offset;
        subMember->size = subMemberReflection.size;

        // Submember struct reflection:
        if (IsStruct(subMemberReflection) && !IsArray(subMemberReflection))
            StructReflection(subMemberReflection, subMember);

        // Submember array or arrayOfStruct reflection:
        if (IsArray(subMemberReflection))
            ArrayReflection(subMemberReflection.name, subMemberReflection, subMember);

        member->AddSubMember(subMemberReflection.name, subMember);
    }
}
void SpirvReflect::ArrayReflection(const std::string& arrayName, const SpvReflectBlockVariable& arrayReflection, UniformBufferMember* member) const
{
    for (uint32_t arrayIndex = 0; arrayIndex < arrayReflection.array.dims[0]; arrayIndex++)
    {
        UniformBufferMember* element = new UniformBufferMember();
        element->offset = member->offset + arrayReflection.array.stride * arrayIndex;
        element->size = arrayReflection.array.stride;

        // Submember struct reflection:
        if (IsStruct(arrayReflection))
            StructReflection(arrayReflection, element);
    
        member->AddSubMember(arrayName + "[" + std::to_string(arrayIndex) + "]", element);
    }
}
std::string SpirvReflect::GetSpvReflectDescriptorTypeName(SpvReflectDescriptorType spvReflectDescriptorType)
{
    switch (spvReflectDescriptorType)
    {
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: return "VK_DESCRIPTOR_TYPE_SAMPLER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
        case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";
        case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return "VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR";
        default: return "Unknown SpvReflectDescriptorType";
    }
}
std::string SpirvReflect::GetSpvStorageClassName(SpvStorageClass spvStorageClass)
{
    switch (spvStorageClass)
    {
        case SpvStorageClassUniformConstant: return "SpvStorageClassUniformConstant";
        case SpvStorageClassInput: return "SpvStorageClassInput";
        case SpvStorageClassUniform: return "SpvStorageClassUniform";
        case SpvStorageClassOutput: return "SpvStorageClassOutput";
        case SpvStorageClassWorkgroup: return "SpvStorageClassWorkgroup";
        case SpvStorageClassCrossWorkgroup: return "SpvStorageClassCrossWorkgroup";
        case SpvStorageClassPrivate: return "SpvStorageClassPrivate";
        case SpvStorageClassFunction: return "SpvStorageClassFunction";
        case SpvStorageClassGeneric: return "SpvStorageClassGeneric";
        case SpvStorageClassPushConstant: return "SpvStorageClassPushConstant";
        case SpvStorageClassAtomicCounter: return "SpvStorageClassAtomicCounter";
        case SpvStorageClassImage: return "SpvStorageClassImage";
        case SpvStorageClassStorageBuffer: return "SpvStorageClassStorageBuffer";
        case SpvStorageClassTileImageEXT: return "SpvStorageClassTileImageEXT";
        case SpvStorageClassCallableDataKHR: return "SpvStorageClassCallableDataKHR |SpvStorageClassCallableDataNV";
        case SpvStorageClassIncomingCallableDataKHR: return "SpvStorageClassIncomingCallableDataKHR | SpvStorageClassIncomingCallableDataNV";
        case SpvStorageClassRayPayloadKHR: return "SpvStorageClassRayPayloadKHR | SpvStorageClassRayPayloadNV";
        case SpvStorageClassHitAttributeKHR: return "SpvStorageClassHitAttributeKHR | SpvStorageClassHitAttributeNV";
        case SpvStorageClassIncomingRayPayloadKHR: return "SpvStorageClassIncomingRayPayloadKHR | SpvStorageClassIncomingRayPayloadNV";
        case SpvStorageClassShaderRecordBufferKHR: return "SpvStorageClassShaderRecordBufferKHR | SpvStorageClassShaderRecordBufferNV";
        case SpvStorageClassPhysicalStorageBuffer: return "SpvStorageClassPhysicalStorageBuffer | SpvStorageClassPhysicalStorageBufferEXT";
        case SpvStorageClassHitObjectAttributeNV: return "SpvStorageClassHitObjectAttributeNV";
        case SpvStorageClassTaskPayloadWorkgroupEXT: return "SpvStorageClassTaskPayloadWorkgroupEXT";
        case SpvStorageClassCodeSectionINTEL: return "SpvStorageClassCodeSectionINTEL";
        case SpvStorageClassDeviceOnlyINTEL: return "SpvStorageClassDeviceOnlyINTEL";
        case SpvStorageClassHostOnlyINTEL: return "SpvStorageClassHostOnlyINTEL";
        case SpvStorageClassMax: return "SpvStorageClassMax";
        default: return "Unknown SpvStorageClass";
    }
}
std::string SpirvReflect::GetSpvBuiltInName(SpvBuiltIn spvBuiltIn)
{
    switch (spvBuiltIn)
    {
        case SpvBuiltInPosition: return "SpvBuiltInPosition";
        case SpvBuiltInPointSize: return "SpvBuiltInPointSize";
        case SpvBuiltInClipDistance: return "SpvBuiltInClipDistance";
        case SpvBuiltInCullDistance: return "SpvBuiltInCullDistance";
        case SpvBuiltInVertexId: return "SpvBuiltInVertexId";
        case SpvBuiltInInstanceId: return "SpvBuiltInInstanceId";
        case SpvBuiltInPrimitiveId: return "SpvBuiltInPrimitiveId";
        case SpvBuiltInInvocationId: return "SpvBuiltInInvocationId";
        case SpvBuiltInLayer: return "SpvBuiltInLayer";
        case SpvBuiltInViewportIndex: return "SpvBuiltInViewportIndex";
        case SpvBuiltInTessLevelOuter: return "SpvBuiltInTessLevelOuter";
        case SpvBuiltInTessLevelInner: return "SpvBuiltInTessLevelInner";
        case SpvBuiltInTessCoord: return "SpvBuiltInTessCoord";
        case SpvBuiltInPatchVertices: return "SpvBuiltInPatchVertices";
        case SpvBuiltInFragCoord: return "SpvBuiltInFragCoord";
        case SpvBuiltInPointCoord: return "SpvBuiltInPointCoord";
        case SpvBuiltInFrontFacing: return "SpvBuiltInFrontFacing";
        case SpvBuiltInSampleId: return "SpvBuiltInSampleId";
        case SpvBuiltInSamplePosition: return "SpvBuiltInSamplePosition";
        case SpvBuiltInSampleMask: return "SpvBuiltInSampleMask";
        case SpvBuiltInFragDepth: return "SpvBuiltInFragDepth";
        case SpvBuiltInHelperInvocation: return "SpvBuiltInHelperInvocation";
        case SpvBuiltInNumWorkgroups: return "SpvBuiltInNumWorkgroups";
        case SpvBuiltInWorkgroupSize: return "SpvBuiltInWorkgroupSize";
        case SpvBuiltInWorkgroupId: return "SpvBuiltInWorkgroupId";
        case SpvBuiltInLocalInvocationId: return "SpvBuiltInLocalInvocationId";
        case SpvBuiltInGlobalInvocationId: return "SpvBuiltInGlobalInvocationId";
        case SpvBuiltInLocalInvocationIndex: return "SpvBuiltInLocalInvocationIndex";
        case SpvBuiltInWorkDim: return "SpvBuiltInWorkDim";
        case SpvBuiltInGlobalSize: return "SpvBuiltInGlobalSize";
        case SpvBuiltInEnqueuedWorkgroupSize: return "SpvBuiltInEnqueuedWorkgroupSize";
        case SpvBuiltInGlobalOffset: return "SpvBuiltInGlobalOffset";
        case SpvBuiltInGlobalLinearId: return "SpvBuiltInGlobalLinearId";
        case SpvBuiltInSubgroupSize: return "SpvBuiltInSubgroupSize";
        case SpvBuiltInSubgroupMaxSize: return "SpvBuiltInSubgroupMaxSize";
        case SpvBuiltInNumSubgroups: return "SpvBuiltInNumSubgroups";
        case SpvBuiltInNumEnqueuedSubgroups: return "SpvBuiltInNumEnqueuedSubgroups";
        case SpvBuiltInSubgroupId: return "SpvBuiltInSubgroupId";
        case SpvBuiltInSubgroupLocalInvocationId: return "SpvBuiltInSubgroupLocalInvocationId";
        case SpvBuiltInVertexIndex: return "SpvBuiltInVertexIndex";
        case SpvBuiltInInstanceIndex: return "SpvBuiltInInstanceIndex";
        case SpvBuiltInCoreIDARM: return "SpvBuiltInCoreIDARM";
        case SpvBuiltInCoreCountARM: return "SpvBuiltInCoreCountARM";
        case SpvBuiltInCoreMaxIDARM: return "SpvBuiltInCoreMaxIDARM";
        case SpvBuiltInWarpIDARM: return "SpvBuiltInWarpIDARM";
        case SpvBuiltInWarpMaxIDARM: return "SpvBuiltInWarpMaxIDARM";
        case SpvBuiltInSubgroupEqMask: return "SpvBuiltInSubgroupEqMask | SpvBuiltInSubgroupEqMaskKHR";
        case SpvBuiltInSubgroupGeMask: return "SpvBuiltInSubgroupGeMask | SpvBuiltInSubgroupGeMaskKHR";
        case SpvBuiltInSubgroupGtMask: return "SpvBuiltInSubgroupGtMask | SpvBuiltInSubgroupGtMaskKHR";
        case SpvBuiltInSubgroupLeMask: return "SpvBuiltInSubgroupLeMask | SpvBuiltInSubgroupLeMaskKHR";
        case SpvBuiltInSubgroupLtMask: return "SpvBuiltInSubgroupLtMask | SpvBuiltInSubgroupLtMaskKHR";
        case SpvBuiltInBaseVertex: return "SpvBuiltInBaseVertex";
        case SpvBuiltInBaseInstance: return "SpvBuiltInBaseInstance";
        case SpvBuiltInDrawIndex: return "SpvBuiltInDrawIndex";
        case SpvBuiltInPrimitiveShadingRateKHR: return "SpvBuiltInPrimitiveShadingRateKHR";
        case SpvBuiltInDeviceIndex: return "SpvBuiltInDeviceIndex";
        case SpvBuiltInViewIndex: return "SpvBuiltInViewIndex";
        case SpvBuiltInShadingRateKHR: return "SpvBuiltInShadingRateKHR";
        case SpvBuiltInBaryCoordNoPerspAMD: return "SpvBuiltInBaryCoordNoPerspAMD";
        case SpvBuiltInBaryCoordNoPerspCentroidAMD: return "SpvBuiltInBaryCoordNoPerspCentroidAMD";
        case SpvBuiltInBaryCoordNoPerspSampleAMD: return "SpvBuiltInBaryCoordNoPerspSampleAMD";
        case SpvBuiltInBaryCoordSmoothAMD: return "SpvBuiltInBaryCoordSmoothAMD";
        case SpvBuiltInBaryCoordSmoothCentroidAMD: return "SpvBuiltInBaryCoordSmoothCentroidAMD";
        case SpvBuiltInBaryCoordSmoothSampleAMD: return "SpvBuiltInBaryCoordSmoothSampleAMD";
        case SpvBuiltInBaryCoordPullModelAMD: return "SpvBuiltInBaryCoordPullModelAMD";
        case SpvBuiltInFragStencilRefEXT: return "SpvBuiltInFragStencilRefEXT";
        case SpvBuiltInViewportMaskNV: return "SpvBuiltInViewportMaskNV";
        case SpvBuiltInSecondaryPositionNV: return "SpvBuiltInSecondaryPositionNV";
        case SpvBuiltInSecondaryViewportMaskNV: return "SpvBuiltInSecondaryViewportMaskNV";
        case SpvBuiltInPositionPerViewNV: return "SpvBuiltInPositionPerViewNV";
        case SpvBuiltInViewportMaskPerViewNV: return "SpvBuiltInViewportMaskPerViewNV";
        case SpvBuiltInFullyCoveredEXT: return "SpvBuiltInFullyCoveredEXT";
        case SpvBuiltInTaskCountNV: return "SpvBuiltInTaskCountNV";
        case SpvBuiltInPrimitiveCountNV: return "SpvBuiltInPrimitiveCountNV";
        case SpvBuiltInPrimitiveIndicesNV: return "SpvBuiltInPrimitiveIndicesNV";
        case SpvBuiltInClipDistancePerViewNV: return "SpvBuiltInClipDistancePerViewNV";
        case SpvBuiltInCullDistancePerViewNV: return "SpvBuiltInCullDistancePerViewNV";
        case SpvBuiltInLayerPerViewNV: return "SpvBuiltInLayerPerViewNV";
        case SpvBuiltInMeshViewCountNV: return "SpvBuiltInMeshViewCountNV";
        case SpvBuiltInMeshViewIndicesNV: return "SpvBuiltInMeshViewIndicesNV";
        case SpvBuiltInBaryCoordKHR: return "SpvBuiltInBaryCoordKHR | SpvBuiltInBaryCoordNV";
        case SpvBuiltInBaryCoordNoPerspKHR: return "SpvBuiltInBaryCoordNoPerspKHR | SpvBuiltInBaryCoordNoPerspNV";
        case SpvBuiltInFragSizeEXT: return "SpvBuiltInFragSizeEXT | SpvBuiltInFragmentSizeNV";
        case SpvBuiltInFragInvocationCountEXT: return "SpvBuiltInFragInvocationCountEXT | SpvBuiltInInvocationsPerPixelNV";
        case SpvBuiltInPrimitivePointIndicesEXT: return "SpvBuiltInPrimitivePointIndicesEXT";
        case SpvBuiltInPrimitiveLineIndicesEXT: return "SpvBuiltInPrimitiveLineIndicesEXT";
        case SpvBuiltInPrimitiveTriangleIndicesEXT: return "SpvBuiltInPrimitiveTriangleIndicesEXT";
        case SpvBuiltInCullPrimitiveEXT: return "SpvBuiltInCullPrimitiveEXT";
        case SpvBuiltInLaunchIdKHR: return "SpvBuiltInLaunchIdKHR | SpvBuiltInLaunchIdNV";
        case SpvBuiltInLaunchSizeKHR: return "SpvBuiltInLaunchSizeKHR | SpvBuiltInLaunchSizeNV";
        case SpvBuiltInWorldRayOriginKHR: return "SpvBuiltInWorldRayOriginKHR | SpvBuiltInWorldRayOriginNV";
        case SpvBuiltInWorldRayDirectionKHR: return "SpvBuiltInWorldRayDirectionKHR | SpvBuiltInWorldRayDirectionNV";
        case SpvBuiltInObjectRayOriginKHR: return "SpvBuiltInObjectRayOriginKHR | SpvBuiltInObjectRayOriginNV";
        case SpvBuiltInObjectRayDirectionKHR: return "SpvBuiltInObjectRayDirectionKHR | SpvBuiltInObjectRayDirectionNV";
        case SpvBuiltInRayTminKHR: return "SpvBuiltInRayTminKHR | SpvBuiltInRayTminNV";
        case SpvBuiltInRayTmaxKHR: return "SpvBuiltInRayTmaxKHR | SpvBuiltInRayTmaxNV";
        case SpvBuiltInInstanceCustomIndexKHR: return "SpvBuiltInInstanceCustomIndexKHR | SpvBuiltInInstanceCustomIndexNV";
        case SpvBuiltInObjectToWorldKHR: return "SpvBuiltInObjectToWorldKHR | SpvBuiltInObjectToWorldNV";
        case SpvBuiltInWorldToObjectKHR: return "SpvBuiltInWorldToObjectKHR | SpvBuiltInWorldToObjectNV";
        case SpvBuiltInHitTNV: return "SpvBuiltInHitTNV";
        case SpvBuiltInHitKindKHR: return "SpvBuiltInHitKindKHR | SpvBuiltInHitKindNV";
        case SpvBuiltInCurrentRayTimeNV: return "SpvBuiltInCurrentRayTimeNV";
        case SpvBuiltInHitTriangleVertexPositionsKHR: return "SpvBuiltInHitTriangleVertexPositionsKHR";
        case SpvBuiltInIncomingRayFlagsKHR: return "SpvBuiltInIncomingRayFlagsKHR | SpvBuiltInIncomingRayFlagsNV";
        case SpvBuiltInRayGeometryIndexKHR: return "SpvBuiltInRayGeometryIndexKHR";
        case SpvBuiltInWarpsPerSMNV: return "SpvBuiltInWarpsPerSMNV";
        case SpvBuiltInSMCountNV: return "SpvBuiltInSMCountNV";
        case SpvBuiltInWarpIDNV: return "SpvBuiltInWarpIDNV";
        case SpvBuiltInSMIDNV: return "SpvBuiltInSMIDNV";
        case SpvBuiltInCullMaskKHR: return "SpvBuiltInCullMaskKHR";
        case SpvBuiltInMax: return "SpvBuiltInMax";
        default: return "Unknown SpvBuildIn";
    }
}