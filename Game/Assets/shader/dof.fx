
#include "PostProcess.hlsli"

cbuffer DofCB : register(b1)
{
    float focusDistance; // ピントが合う距離
    float focusRange;    // ピントの合う幅(この外は全ボケ)
    float2 pad0;
};

Texture2D<float4> mainTexture : register(t0);
Texture2D<float4> bokeTexture : register(t1);
sampler Sampler : register(s0);

float4 PSMain(PSInput In) : SV_Target0
{
    // xyz = くっきり色、w = 距離
    float4 sharp = mainTexture.Sample(Sampler, In.uv);
    float3 boke = bokeTexture.Sample(Sampler, In.uv).xyz;

    // ピントから離れるほど1
    float rate = saturate(abs(sharp.w - focusDistance) / focusRange);
    return float4(lerp(sharp.xyz, boke, rate), 1.0f);
}