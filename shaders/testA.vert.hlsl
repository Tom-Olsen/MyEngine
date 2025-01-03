cbuffer RenderMatrizes : register(b0)
{
    float4x4 modelMatrix;       // mesh local to world matrix
    float4x4 viewMatrix;        // camera world to local matrix
    float4x4 projMatrix;        // camera projection matrix
    float4x4 normalMatrix;      // rotation matrix for directions: (model^-1)^T
    float4x4 localToClipMatrix; // local to camera clip space matrix: (projection * view * model)
};


struct VertexInput
{
    float3 position : POSITION;
    float4 uv : TEXCOORD0;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float4 uv : TEXCOORD0;
};



VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.position = mul(localToClipMatrix, float4(input.position, 1.0));
    output.uv = input.uv;
    return output;
}