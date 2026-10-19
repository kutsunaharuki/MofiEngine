/*!
 * @brief ディファードライティング
 *
 * G-Buffer(アルベド・法線・ワールド座標)とシャドウマップを読んで、
 * 画面のピクセルごとにライティングと影を計算する。
 * 画面全体に1枚のスプライトとして描画される(RenderingEngine::Execute のライティングパス)。
 *
 * ライティングの計算は model.fx と同じ。
 * 違うのは、法線・ワールド座標などを、頂点シェーダーからではなく G-Buffer から読むところ。
 */

////////////////////////////////////////////////
// スプライト共通の定数バッファ(Sprite クラスが b0 に繋ぐ)
////////////////////////////////////////////////
cbuffer cb : register(b0)
{
    float4x4 mvp;       // モデルビュープロジェクション行列
    float4 mulColor;    // 乗算カラー(使わない)
};


////////////////////////////////////////////////
// ライトの構造体(model.fx / SceneLight.h と同じ並び)
////////////////////////////////////////////////
struct DirectionLight
{
    float3 direction;
    float pad0;
    float3 color;
    float pad1;
};

struct AmbientLight
{
    float3 ambient;
    float pad2;
};

struct Light
{
    float3 eyePos;
    float specPower;
    float reflectPower;
    float3 pad3;
};

struct ShadowParam
{
    float shadowBias;
    float3 pad4;
    float shadowBiasMin;
    float3 pad5;
};

struct PTLight
{
    float3 ptPosition;
    float ptRange;
    float3 ptColor;
    float pad6;
    float3 ptDirection;
    float ptAngle;
};

// ライトの定数バッファ(Sprite の拡張定数バッファとして b1 に繋ぐ)
cbuffer LightCB : register(b1)
{
    DirectionLight directionLight;
    AmbientLight ambientLight;
    Light light;
    float4x4 mLVP[4];
    ShadowParam shadowParam;
    PTLight ptLight[4];
    int numPtLights;
    float3 pad7;
};


////////////////////////////////////////////////
// テクスチャ(SpriteInitData::m_textures の順番で t0 から繋がる)
////////////////////////////////////////////////
Texture2D<float4> g_albedoTexture   : register(t0);   // アルベド(素材の色)
Texture2D<float4> g_normalTexture   : register(t1);   // 法線(xyz = 0~1に詰めた法線、w = スペキュラの強さ)
Texture2D<float4> g_worldPosTexture : register(t2);   // ワールド座標(xyz = 座標、w = 1ならモデルがある)
Texture2D<float>  g_shadowMap0      : register(t3);   // シャドウマップ(ライトから見た深度)
Texture2D<float>  g_shadowMap1      : register(t4);
Texture2D<float>  g_shadowMap2      : register(t5);
Texture2D<float>  g_shadowMap3      : register(t6);

sampler Sampler : register(s0);


////////////////////////////////////////////////
// 頂点シェーダー(sprite.fx と同じ)
////////////////////////////////////////////////
struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
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


/////////////////////////////////////////////////
// 拡散反射光(model.fx と同じ)
/////////////////////////////////////////////////
float3 CalcDiffuseLight(DirectionLight dirLig, float3 normal)
{
    float t = max(0.0f, dot(normal, -dirLig.direction));
    return dirLig.color * t;
}

/////////////////////////////////////////////////
// 鏡面反射光(model.fx と同じ)
/////////////////////////////////////////////////
float3 CalcSpecularLight(DirectionLight dirLig, Light lig, float3 normal, float3 worldPos, float specPower)
{
    float3 refVec = reflect(dirLig.direction, normal);
    float3 toEye = normalize(lig.eyePos - worldPos);

    float t = pow(max(0.0f, dot(refVec, toEye)), lig.specPower);

    float3 specLig = dirLig.color * t;
    specLig *= specPower * lig.reflectPower;
    return specLig;
}

//////////////////////////////////////////////////
// スポットライトの拡散反射光(model.fx と同じ)
//////////////////////////////////////////////////
float3 CalcSpotLight(PTLight pt, float3 normal, float3 worldPos)
{
    float3 ligDir = normalize(worldPos - pt.ptPosition);
    float ligDis = length(worldPos - pt.ptPosition);

    // 距離による減衰
    float disAffect = saturate(1.0f - ligDis / pt.ptRange);
    disAffect = pow(disAffect, 3.0f);

    // 角度による減衰
    float cosAngle = dot(ligDir, normalize(pt.ptDirection));
    float angle = abs(acos(cosAngle));
    float angleAffect = saturate(1.0f - angle / radians(pt.ptAngle));
    angleAffect = pow(angleAffect, 0.5f);

    float t = max(0.0f, dot(normal, -ligDir));
    return pt.ptColor * t * disAffect * angleAffect;
}


/////////////////////////////////////////////////
// シャドウマップ1枚分の影の強さ(0 = 影なし ~ 1 = 影)
//
// model.fx はハードウェアの比較サンプラー(s1)で比べているが、
// スプライトには比較サンプラーが無いので、深度を Load で読んで自分で比べる。
// まわり 3x3 ピクセルの結果を平均して、影のふちをなめらかにする(PCF)。
/////////////////////////////////////////////////
float CalcShadow(Texture2D<float> shadowMap, float4x4 lvp, float3 worldPos, float3 normal)
{
    // ライトから見た位置
    float4 posInLVP = mul(lvp, float4(worldPos, 1.0f));
    float2 shadowMapUV = posInLVP.xy / posInLVP.w;
    shadowMapUV *= float2(0.5f, -0.5f);
    shadowMapUV += 0.5f;

    // シャドウマップの範囲外は、影なし
    if (shadowMapUV.x < 0.0f || shadowMapUV.x > 1.0f || shadowMapUV.y < 0.0f || shadowMapUV.y > 1.0f)
    {
        return 0.0f;
    }

    // ライトから見た深度と、傾きに応じたバイアス(影のギザギザ(シャドウアクネ)を防ぐ)
    float zInLVP = posInLVP.z / posInLVP.w;
    float3 L = normalize(directionLight.direction);
    float bias = max(shadowParam.shadowBias * (1.0f - dot(normal, -L)), shadowParam.shadowBiasMin);
    float compareZ = zInLVP - bias;

    // シャドウマップのピクセル位置
    uint width, height;
    shadowMap.GetDimensions(width, height);
    int2 center = int2(shadowMapUV * float2(width, height));

    // 3x3 ピクセルで比べて平均する
    float shadow = 0.0f;
    [unroll]
    for (int y = -1; y <= 1; y++)
    {
        [unroll]
        for (int x = -1; x <= 1; x++)
        {
            int2 texel = clamp(center + int2(x, y), int2(0, 0), int2(width - 1, height - 1));
            float depthInShadowMap = shadowMap.Load(int3(texel, 0));
            // シャドウマップより奥にある(ライトから遠い) = 何かに遮られている = 影
            shadow += (compareZ > depthInShadowMap) ? 1.0f : 0.0f;
        }
    }
    return shadow / 9.0f;
}


////////////////////////////////////////////////
// ピクセルシェーダー
////////////////////////////////////////////////
float4 PSMain(PSInput In) : SV_Target0
{
    // G-Buffer は画面と同じ大きさなので、ピクセルの位置でそのまま読む
    int3 texel = int3(In.pos.xy, 0);

    float4 worldPosData = g_worldPosTexture.Load(texel);
    // モデルが描かれていないところ(背景)は何もしない(メインレンダーターゲットのクリア色(空の色)が残る)
    if (worldPosData.w <= 0.0f)
    {
        discard;
    }

    float3 worldPos = worldPosData.xyz;
    float4 albedoColor = g_albedoTexture.Load(texel);
    float4 normalData = g_normalTexture.Load(texel);
    // 0~1 に詰めた法線を -1~1 に戻す
    float3 normal = normalize(normalData.xyz * 2.0f - 1.0f);
    float specularPower = normalData.w;

    // ディレクションライト
    float3 diffuseLig = CalcDiffuseLight(directionLight, normal);
    float3 specularLig = CalcSpecularLight(directionLight, light, normal, worldPos, specularPower);

    // スポットライト
    float3 ptDiffuse = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < numPtLights; i++)
    {
        ptDiffuse += CalcSpotLight(ptLight[i], normal, worldPos);
    }

    // 影(4枚のシャドウマップ)
    // ※ 足すと 1 を超えて、(1 - 影) がマイナスになり真っ黒になるので、1 までに収める
    float shadowSum = 0.0f;
    shadowSum += CalcShadow(g_shadowMap0, mLVP[0], worldPos, normal);
    shadowSum += CalcShadow(g_shadowMap1, mLVP[1], worldPos, normal);
    shadowSum += CalcShadow(g_shadowMap2, mLVP[2], worldPos, normal);
    shadowSum += CalcShadow(g_shadowMap3, mLVP[3], worldPos, normal);
    shadowSum = saturate(shadowSum);

    // 最終的な色(model.fx と同じ)
    float3 nonAmbientLig = diffuseLig + specularLig;
    float3 finalColor = albedoColor.xyz * (ambientLight.ambient + nonAmbientLig * (1.0f - shadowSum)) + ptDiffuse;

    // α には、カメラからの距離を入れる(DoF がピントの計算に使う。model.fx と同じ)
    return float4(finalColor, length(worldPos - light.eyePos));
}
