
struct SPSIn
{
    float4 pos      : SV_POSITION;
    float3 normal   : NORMAL;      
    float3 tangent  : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv       : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

#include "ModelVSCommon.hlsli"

struct SPSOut
{
    float4 albedo   : SV_Target0;
    float4 normal   : SV_Target1;
    float4 worldPos : SV_Target2;
};

Texture2D<float4> albedoTexture : register(t0);
sampler Sampler : register(s0);


SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal, uniform bool isUsePreComputedVertexBuffer)
{
    SPSIn psIn;

    psIn.pos = CalcVertexPositionInWorldSpace(vsIn.pos, mWorldLocal, isUsePreComputedVertexBuffer);
    psIn.worldPos = psIn.pos;

    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);

    CalcVertexNormalTangentBiNormalInWorldSpace(
        psIn.normal, psIn.tangent, psIn.biNormal,
        mWorldLocal, vsIn.normal, vsIn.tangent, vsIn.biNormal,
        isUsePreComputedVertexBuffer
    );

    psIn.uv = vsIn.uv;
    return psIn;
}


SPSOut PSMain(SPSIn In)
{
    SPSOut psOut;
    // アルベド : テクスチャの色をそのまま
    psOut.albedo = albedoTexture.Sample(Sampler, In.uv);

    // 法線 : (-1 ~ 1)のままでは色として保存できないので(0 ~ 1)に詰める
    psOut.normal.xyz = normalize(In.normal) * 0.5f + 0.5f;
    psOut.normal.w = 1.0f;

    // ワールド座標 : R32G32B32A32_FLOAT なので生の値をそのまま入れられる
    psOut.worldPos = float4(In.worldPos, 1.0f);
    return psOut;
}