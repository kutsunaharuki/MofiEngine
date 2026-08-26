
// スプライト / ポストプロセス専用の共通
#include "PostProcess.hlsli"

// ブルームの定数バッファ
cbuffer BloomCB : register(b1)
{
    float threshold;
};

Texture2D<float4> mainTexture : register(t0);
sampler Sampler : register(s0);

float4 PSMain(PSInput In) : SV_Target0
{
    float4 color = mainTexture.Sample(Sampler, In.uv);
    float t = dot(color.xyz, float3(0.2125f, 0.7154f, 0.0721f)); // 輝度(明るさ)を計算
    // 閾値未満のピクセルはここで捨てられる(何も描かれない)
    clip(t - threshold);
    return color;
};