SamplerState colorSampler : register(s10);
TextureCube colorMap : register(t28);
#include "shadingPushConstant.hlsli"



struct FragmentInput
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD1;
};



struct FragmentOutput
{
    float4 color : SV_Target;
    float depth : SV_Depth;
};



FragmentOutput main(FragmentInput input)
{
    // Mesh data:
    float3 direction = normalize(input.localPos.xyz);
    
    FragmentOutput output;
    output.color = colorMap.Sample(colorSampler, direction);
    output.depth = 1.0f;
    return output;
}