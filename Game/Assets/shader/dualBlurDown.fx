
#include "PostProcess.hlsli"

Texture2D<float4> srcTexture : register(t0);
sampler Sampler : register(s0);

float4 PSMain(PSInput In) : SV_Target0
{
    float2 size;
    srcTexture.GetDimensions(size.x,size.y);
    float2 o = 0.5f / size;

    // ダウンサンプル(5タップ) : 中心を重く、四隅を混ぜながら縮小
    float4 sum = srcTexture.Sample(Sampler, In.uv) * 4.0f;
    sum += srcTexture.Sample(Sampler, In.uv + float2(-o.x, -o.y));
    sum += srcTexture.Sample(Sampler, In.uv + float2( o.x, -o.y));
    sum += srcTexture.Sample(Sampler, In.uv + float2(-o.x,  o.y));
    sum += srcTexture.Sample(Sampler, In.uv + float2( o.x,  o.y));
    return sum / 8.0f;
}