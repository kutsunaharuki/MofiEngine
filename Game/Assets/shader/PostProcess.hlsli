
#ifndef POST_PROCESS_HLSLI
#define POST_PROCESS_HLSLI

cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4 mulColor;
};

struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
} 

#endif // POST_PROCESS_HLSLI