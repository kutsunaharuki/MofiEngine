
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
// ノーマルマップ(マテリアルが t1 に繋ぐ。model.fx と同じ)
Texture2D<float4> normalMap : register(t1);
// スペキュラマップ(マテリアルが t2 に繋ぐ。model.fx と同じ)
Texture2D<float4> specularMap : register(t2);
sampler Sampler : register(s0);


///////////////////////////////////////////////
// 法線マップからワールド空間の法線を復元する(model.fx と同じ)
///////////////////////////////////////////////
float3 CalcNormalFromMap(Texture2D<float4> map, sampler samp, float2 uv, float3 tangent, float3 biNormal, float3 normal)
{
    float3 localNormal = map.Sample(samp, uv).xyz;
    localNormal = (localNormal * 2.0f) - 1.0f;

    return normalize(
          tangent  * localNormal.x
        + biNormal * localNormal.y
        + normal   * localNormal.z
    );
}


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

    // 法線 : ノーマルマップで凹凸を足してから、(-1 ~ 1)を(0 ~ 1)に詰める
    float3 normal = CalcNormalFromMap(normalMap, Sampler, In.uv, In.tangent, In.biNormal, In.normal);
    psOut.normal.xyz = normal * 0.5f + 0.5f;
    // 法線の w には、スペキュラの強さを入れる(ライティングで使う)
    psOut.normal.w = specularMap.Sample(Sampler, In.uv).r;

    // ワールド座標 : R32G32B32A32_FLOAT なので生の値をそのまま入れられる
    // w = 1 は「ここにモデルが描かれた」という印。クリア色の w = 0 のところは背景(空)になる
    psOut.worldPos = float4(In.worldPos, 1.0f);
    return psOut;
}