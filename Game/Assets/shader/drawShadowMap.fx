/**
 * @file drawShadow.fx
 * @brief 何が来ても絶対に黒色にする(値を黒で返す)
 */
////////////////////////////////////////////////
// Pixel shader input.
////////////////////////////////////////////////
struct SPSIn
{
    float4 pos      : SV_POSITION;  // Clip-space position.
    float3 normal   : NORMAL;       // World-space normal.
    float3 tangent  : TANGENT;      // World-space tangent   (for normal mapping later).
    float3 biNormal : BINORMAL;     // World-space binormal  (for normal mapping later).
    float2 uv       : TEXCOORD0;    // UV.
    float3 worldPos : TEXCOORD1;    // World-space position  (for specular later).
};


///////////////////////////////////////
// Common vertex shader code.
// Provides: ModelCb(b0: mWorld/mView/mProj), SVSIn, bone matrices (t3),
//           and the entry points VSMain / VSMainSkin / VSMainInstancing, etc.
///////////////////////////////////////
#include "ModelVSCommon.h" // ModelCB : register(b0)はこの中に入ってる ※ b0は共通で使っているので被ってはならない



////////////////////////////////////////////////
// Vertex shader core (called by the VSMain* entry points in ModelVSCommon.h).
////////////////////////////////////////////////
SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal, uniform bool isUsePreComputedVertexBuffer)
{
    SPSIn psIn;
    psIn.pos = CalcVertexPositionInWorldSpace(vsIn.pos, mWorldLocal, isUsePreComputedVertexBuffer);
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);
    psIn.uv = vsIn.uv;
    psIn.normal = mul((float3x3)mWorldLocal, vsIn.normal);
    return psIn;
}

////////////////////////////////////////////////
// Pixel shader.
// For now: just output the albedo texture. Add your lighting here.
////////////////////////////////////////////////
float4 PSMain(SPSIn In) : SV_Target0
{
    return float4(0.0f,0.0f,0.0f,1.0f); // 何が来ても絶対に黒
}