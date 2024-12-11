#ifndef __INCLUDE_GUARD_linearAlgebra_hlsli__
#define __INCLUDE_GUARD_linearAlgebra_hlsli__



// Vector operations:
float LinAlg_VectorToPlaneDistance(float3 vec, float3 planeNormal)
{
    return dot(vec, planeNormal) / dot(planeNormal, planeNormal);
}
float3 LinAlg_VectorToPlaneProjection(float3 vec, float3 planeNormal)
{
    return vec - LinAlg_VectorToPlaneDistance(vec, planeNormal) * planeNormal;
}


// Matrix inversion:
float4x4 LinAlg_Invert(float4x4 input)
{
#define minor(a,b,c) determinant(float3x3(input.a, input.b, input.c))
    float4x4 cofactors = float4x4(
         minor(_22_23_24, _32_33_34, _42_43_44),
        -minor(_21_23_24, _31_33_34, _41_43_44),
         minor(_21_22_24, _31_32_34, _41_42_44),
        -minor(_21_22_23, _31_32_33, _41_42_43),

        -minor(_12_13_14, _32_33_34, _42_43_44),
         minor(_11_13_14, _31_33_34, _41_43_44),
        -minor(_11_12_14, _31_32_34, _41_42_44),
         minor(_11_12_13, _31_32_33, _41_42_43),

         minor(_12_13_14, _22_23_24, _42_43_44),
        -minor(_11_13_14, _21_23_24, _41_43_44),
         minor(_11_12_14, _21_22_24, _41_42_44),
        -minor(_11_12_13, _21_22_23, _41_42_43),

        -minor(_12_13_14, _22_23_24, _32_33_34),
         minor(_11_13_14, _21_23_24, _31_33_34),
        -minor(_11_12_14, _21_22_24, _31_32_34),
         minor(_11_12_13, _21_22_23, _31_32_33));
#undef minor
    return transpose(cofactors) / determinant(input);
}



#endif //__INCLUDE_GUARD_linearAlgebra_hlsli__