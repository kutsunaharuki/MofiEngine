////////////////////////////////////////////////
// Pixel shader input.
////////////////////////////////////////////////
struct SPSIn
{
    float4 pos      : SV_POSITION;  // Clip-space position.
    float4 posInLVP : TEXCOORD2;    // ライトビュープロジェクション空間の座標
};

#include "ModelVSCommon.hlsli"

/// <summary>
/// 頂点シェーダー
/// <summary>
SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal, uniform bool isUsePreComputedVertexBuffer)
{
    SPSIn psIn;

    psIn.pos = mul(mWorldLocal, vsIn.pos);
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);

    // ライトカメラで描いているため、pos　が「ライトから見た座標」になる
    psIn.posInLVP = psIn.pos;
    return psIn;
}


////////////////////////////////////////////////
// Pixel shader.
// For now: just output the albedo texture. Add your lighting here.
////////////////////////////////////////////////

float4 PSMain(SPSIn In) : SV_Target0
{
    // 深度(ライトから近いほど小さい値) を 「色」 として書き込む
    return In.posInLVP.z / In.posInLVP.w;   // R32_FLOAT なので .rだけ記録される
    //return float4(0.0f,0.0f,0.0f,1.0f); // 何が来ても絶対に黒
}