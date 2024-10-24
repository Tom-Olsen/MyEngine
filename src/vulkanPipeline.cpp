#include <fstream>
#include <vector>
#include "logger.h"
#include "macros.h"
#include "vulkanPipeline.h"
#include "mesh.h"



// Constructor:
VulkanPipeline::VulkanPipeline(VulkanContext* context, VkRenderPass* renderPass,
    const std::vector<char>& vertexCode,
    const std::vector<char>& fragmentCode,
    const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    this->context = context;

    // Create pipeline Layout:
	CreatePipelineLayout(bindings);

    // Create vertex and fragment shader modules from .spv files:
    VkShaderModule vertexShaderModule = CreateShaderModule(vertexCode);
    VkShaderModule fragmentShaderModule = CreateShaderModule(fragmentCode);

    // Create pipeline:
    CreatePipeline(renderPass, vertexShaderModule, fragmentShaderModule);

    // Destroy shader modules (only needed for pipeline creation):
    vkDestroyShaderModule(context->LogicalDevice(), vertexShaderModule, nullptr);
    vkDestroyShaderModule(context->LogicalDevice(), fragmentShaderModule, nullptr);
}



// Destructor:
VulkanPipeline::~VulkanPipeline()
{
    vkDestroyDescriptorSetLayout(context->LogicalDevice(), descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(context->LogicalDevice(), pipelineLayout, nullptr);
    vkDestroyPipeline(context->LogicalDevice(), pipeline, nullptr);
}



// Private:
void VulkanPipeline::CreatePipelineLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	// Descriptor set layout:
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetLayoutCreateInfo.pBindings = bindings.data();
    VKA(vkCreateDescriptorSetLayout(context->LogicalDevice(), &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));

	// Push constants layout:
	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushConstantObject);

	// Pipeline layout:
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    vkCreatePipelineLayout(context->LogicalDevice(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
}
VkShaderModule VulkanPipeline::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    createInfo.codeSize = code.size();
    createInfo.pCode = (uint32_t*)(code.data());

    VkShaderModule shaderModule;
    VKA(vkCreateShaderModule(context->LogicalDevice(), &createInfo, nullptr, &shaderModule));
    return shaderModule;
}

void VulkanPipeline::CreatePipeline(VkRenderPass* renderPass, const VkShaderModule& vertexShaderModule, const VkShaderModule& fragmentShaderModule)
{
    // Vertex shader:
    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageCreateInfo.module = vertexShaderModule;
    vertexShaderStageCreateInfo.pName = "main";

    // Fragment shader:
    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageCreateInfo.module = fragmentShaderModule;
    fragmentShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[2] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

    // Get mesh datalayout from mesh class:
    std::vector<VkVertexInputBindingDescription> bindingDescriptions = Mesh::GetBindingDescription();
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = Mesh::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

    // topology = how to interpret the vertices, triangle list is the most flexible
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Multiple viewports and scissors can be used for multiview rendering (VR). Requires multiview feature
    VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;  // fill=fill triangles, line=draw lines, point=draw points. Line is useful for wireframe rendering
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;    // which face to cull
    rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE; // which face of triangle is front: 123 or 132?
    rasterizationState.lineWidth = 1.0f;    // width of lines. Bigger 1.0f requires wideLines feature
    rasterizationState.depthClampEnable = VK_FALSE; // clamping fragments instead of discarding them is useful for shadow mapping. Requires depthClamp feature
    // Biasing depth values based on fragment's slope is useful for shadow mapping. Requires depthBias feature. We are not using it here
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.depthBiasConstantFactor = 0.0f;  // Optional
    rasterizationState.depthBiasClamp = 0.0f;           // Optional
    rasterizationState.depthBiasSlopeFactor = 0.0f;     // Optional

    // Multisampling:
    VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.rasterizationSamples = context->physicalDevice->maxMsaaSamples;
    multisampleState.minSampleShading = 1.0f;           // Optional
    multisampleState.pSampleMask = nullptr;             // Optional
    multisampleState.alphaToCoverageEnable = VK_FALSE;  // Optional
    multisampleState.alphaToOneEnable = VK_FALSE;       // Optional

	// Depth and stencil testing:
    VkPipelineDepthStencilStateCreateInfo depthState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depthState.depthTestEnable = VK_TRUE;             // depth of new fragments should be compared to the depth buffer to see if they should be discarded
	depthState.depthWriteEnable = VK_TRUE;            // new depth of fragments that pass the depth test should be written to the depth buffer
	depthState.depthCompareOp = VK_COMPARE_OP_LESS;   // comparison that is performed to keep or discard fragments. lower = closer to camera
	depthState.depthBoundsTestEnable = VK_FALSE;      // allows to keep only fragments in the below defined range
    depthState.minDepthBounds = 0.0f;                 // Optional
    depthState.maxDepthBounds = 1.0f;                 // Optional
	depthState.stencilTestEnable = VK_FALSE;          // stencil buffer operations (not used yet)
    depthState.front = {};                            // Optional
    depthState.back = {};                             // Optional

    // Configuration per attached framebuffer:
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.blendEnable = VK_FALSE;
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    // Color blending pseudo code:
    // if (blendEnable)
    // {
    //     finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
    //     finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
    // }
    // else
    //     finalColor = newColor;
    // finalColor = finalColor & colorWriteMask;

    // Current settings give standard alpha blending:
    // finalColor.rgb = (srcAlpha * newColor.rgb) + ((1 - srcAlpha) * oldColor.rgb);
    // finalColor.a = newAlpha.a;

    // Global color blending settings:
    VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachmentState;
    colorBlendState.logicOpEnable = VK_FALSE;   // setting this to true overwrites the colorBlendAttachmentState settings and uses the logicOp
    colorBlendState.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlendState.blendConstants[0] = 0.0f;   // Optional
    colorBlendState.blendConstants[1] = 0.0f;   // Optional
    colorBlendState.blendConstants[2] = 0.0f;   // Optional
    colorBlendState.blendConstants[3] = 0.0f;   // Optional

    // Dynamic states, can be changed without recreating the pipeline:
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineInfo.stageCount = 2;                              // vertex and fragment shaders
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputState;     // Buffer format
    pipelineInfo.pInputAssemblyState = &inputAssemblyState; // Input assembler
    pipelineInfo.pViewportState = &viewportState;           // Viewport and scissor
    pipelineInfo.pRasterizationState = &rasterizationState; // Rasterizer
    pipelineInfo.pMultisampleState = &multisampleState;     // Multisampling
    pipelineInfo.pDepthStencilState = &depthState;          // Depth and stencil testing
    pipelineInfo.pColorBlendState = &colorBlendState;       // Color blending
    pipelineInfo.pDynamicState = &dynamicState;             // Dynamic states
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = *renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;       // can be used to create a new pipeline based on an existing one
    pipelineInfo.basePipelineIndex = -1;

    // Second parameter is a VkPipelineCache. It can be used to store and reuse pipelines; even across runs of the application by saving it to a file.
    VKA(vkCreateGraphicsPipelines(context->LogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
}