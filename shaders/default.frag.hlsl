SamplerState colorSampler : register(s10);
SamplerComparisonState shadowSampler : register(s11);
Texture2DArray<float> shadowMaps : register(t20);
Texture2D colorMap : register(t21);     // format = VK_FORMAT_R8G8B8A8_SRGB,
Texture2D roughnessMap : register(t22); // format = VK_FORMAT_R8_UNORM,         single channel unorm roughness map
Texture2D normalMap : register(t23);    // format = VK_FORMAT_R8G8B8A8_UNORM,   opengl style unorm normal map
#include "shadowMapping.hlsli"
#include "shadingPushConstant.hlsli"



cbuffer SurfaceProperties : register(b1)
{
    float4 diffuseColor;    // (1.0, 1.0, 1.0)
    float roughness;        // 0.5
    float3 reflectivity;    // 0.4
    bool metallic;
    float4 scaleOffset;     // .xy = scale, .zw = offset
};



struct FragmentInput
{
    float4 clipPosition : SV_POSITION;  // position in clip space: x,y�[-1,1] z�[0,1]
    float3 worldNormal : NORMAL;        // normal in world space
    float3 worldTangent : TANGENT;      // tangent in world space
    float4 vertexColor : COLOR;         // vertex color
    float4 uv : TEXCOORD0;              // texture coordinates
    float3 worldPosition : TEXCOORD1;   // position in world space
};



float4 main(FragmentInput input) : SV_TARGET
{
    // Mesh data:
    float2 uv = input.uv.xy * scaleOffset.xy + scaleOffset.zw;
    float3 tangentSpaceNormal = normalize(input.worldNormal);
    float3 tangentSpaceTangent = normalize(LinAlg_VectorToPlaneProjection(input.worldTangent, tangentSpaceNormal));
    float3 tangentSpaceBitangent = cross(tangentSpaceNormal, tangentSpaceTangent);
    float3x3 TBN = transpose(float3x3(tangentSpaceTangent, tangentSpaceBitangent, tangentSpaceNormal));
    float3 worldPos = input.worldPosition;
    
    // Shading:
    float4 color = diffuseColor * colorMap.Sample(colorSampler, uv);
    float3 localNormal = 2.0 * normalMap.Sample(colorSampler, uv).xyz - 1.0;
    float3 worldNormal = normalize(mul(TBN, localNormal));
    float finalRoughness = roughness * roughnessMap.Sample(colorSampler, uv).x;
    
    // Lighting:
    float ambient = 0.1f;
    float3 finalColor = ambient * color.xyz;
    finalColor += PhysicalLighting(worldPos, pc.cameraPosition.xyz, worldNormal, color.xyz, finalRoughness, reflectivity, metallic, pc.dLightsCount, pc.sLightsCount, pc.pLightsCount, directionalLightData, spotLightData, pointLightData, shadowMaps, shadowSampler);
    
    return float4(finalColor, 1.0f);
}