/*!
 * @brief   Minimal model shader (STARTING POINT).
 *
 * Right now this only draws the albedo (base color) texture. There is NO lighting.
 * Your job is to add lighting here, step by step:
 *   1. ambient        (add a constant brightness)
 *   2. diffuse         (directional light + Lambert:  dot(normal, -lightDir))
 *   3. specular        (Phong / Blinn-Phong highlight)
 *   4. specular map    (control the highlight strength per-pixel with a texture)
 *   5. normal map      (per-pixel normals using the tangent space / TBN)
 *
 * See: 学習ガイド_自作レンダリングエンジン.md  (Phase 1)
 *
 * NOTE: To pass values (light direction/color, ambient, etc.) from C++ into this
 *       shader, add your own cbuffer here and fill it from a ConstantBuffer on the
 *       C++ side. The engine does NOT feed lighting data anymore (that used to be
 *       k2Engine's job, which you are now replacing).
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

/////////////////////////////////////////////////////////////////

/** ディレクションライト */
// ライトの方向とライトの色のみの情報を持つ
struct DirectionLight
{
    float3 direction;  // ライトの方向
    float pad0;        // パディング(そろえるための空き)
    float3 color;      // ライトの色(RGB)
    float pad1;        // パディング(そろえるだけの空き)
};


/** 環境光 */
// 色のみの情報を持つ
struct AmbientLight
{
    float3 ambient;   // 環境光
    float pad2;       // パディング
};

/** ライト */
struct Light
{
    float3 eyePos;                 // 視点の位置
    float specPower;               // スぺキュラの絞り
    float reflectPower;            // 反射の強さ
    float3 pad3;                   // 空き
};

/** 影 */
struct ShadowParam
{
    float shadowBias;               // 傾斜に応じたバイアス
    float3 pad4;                    // 空き
    float shadowBiasMin;            // バイアスが0にならないようにする値
    float3 pad5;                    // 空き
};

/** ポイントライト(点光源) */
struct PTLight
{
    float3 ptPosition;              // ポイントライトの位置
    float ptRange;                  // 影響範囲
    float3 ptColor;                 // 色
    float pad6;                     // パディング
    float3 ptDirection;             // ポイントライトの向き
    float ptAngle;                  // ポイントライトの角度(照らす円錐の広さ)
};

///////////////////////////////////////
// Common vertex shader code.
// Provides: ModelCb(b0: mWorld/mView/mProj), SVSIn, bone matrices (t3),
//           and the entry points VSMain / VSMainSkin / VSMainInstancing, etc.
///////////////////////////////////////
#include "ModelVSCommon.hlsli" // ModelCB : register(b0)はこの中に入ってる ※ b0は共通で使っているので被ってはならない

/** ライトの定数バッファ */
// 環境光とか平行光源とかをまとめて所持する
cbuffer LightCB : register(b1)
{
    DirectionLight directionLight;  // ディレクションライト
    AmbientLight ambientLight;      // 環境光
    Light light;                    // ライト
    float4x4 mLVP[4];               // ライトビュープロジェクション行列
    ShadowParam shadowParam;        // 影の調整
    PTLight ptLight[4];             // ポイントライトの配列(4個)
    int numPtLights;                // 今使っているポイントライトの数
    float3 pad7;                    // 空き(パディング)
};


///////////////////////////////////////
// Shader resources.
// The tkm material binds the albedo texture to t0.
// (t1 = normal map, t2 = metallic/smooth — you can add them when you need them.)
///////////////////////////////////////
Texture2D<float4> albedoTexture : register(t0); // テクスチャ

// ノーマルマップ
Texture2D<float4> normalMap : register(t1);     // テクスチャ

// スぺキュラマップ
Texture2D<float4> specularMap : register(t2);   // テクスチャ2

// シャドウマップ
Texture2D<float4> g_shadowMap : register(t10);  // ModelRenderが t10に繋いだシャドウマップ

sampler Sampler : register(s0);                 // サンプラー

// シャドウマップサンプリング用のサンプラーステート
SamplerComparisonState g_shadowMapSampler : register(s1);  // サンプラーステート

////////////////////////////////////////////////
// Vertex shader core (called by the VSMain* entry points in ModelVSCommon.h).
////////////////////////////////////////////////
SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal, uniform bool isUsePreComputedVertexBuffer)
{
    SPSIn psIn;

    // Local space -> world space.
    psIn.pos = CalcVertexPositionInWorldSpace(vsIn.pos, mWorldLocal, isUsePreComputedVertexBuffer);
    psIn.worldPos = psIn.pos;

    // World -> view -> projection (clip) space.
    psIn.pos = mul(mView, psIn.pos); // View
    psIn.pos = mul(mProj, psIn.pos); // Projection

    // World-space normal / tangent / binormal.
    CalcVertexNormalTangentBiNormalInWorldSpace(
        psIn.normal,
        psIn.tangent,
        psIn.biNormal,
        mWorldLocal,
        vsIn.normal,
        vsIn.tangent,
        vsIn.biNormal,
        isUsePreComputedVertexBuffer
    );

    psIn.uv = vsIn.uv;
    return psIn;
}

///////////////////////////////////////////////
// 法線マップからワールド空間の法線を復元する
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

/////////////////////////////////////////////////
// 拡散反射光を計算
/////////////////////////////////////////////////
float3 CalcDiffuseLight(DirectionLight dirLig, float3 normal)
{
    float t = max(0.0f,(dot(normal, -dirLig.direction)));
    return dirLig.color * t;
}

/////////////////////////////////////////////////
// 鏡面反射光を計算
/////////////////////////////////////////////////
float3 CalcSpecularLight(DirectionLight dirLig, Light light, float3 normal, float3 worldPos, float specPower)
{
    float3 refVec = reflect(dirLig.direction, normal);
    float3 toEye = normalize(light.eyePos - worldPos);

    float t = pow(max(0.0f,dot(refVec, toEye)), light.specPower);

    float3 specLig = dirLig.color * t;
    specLig *= specPower * light.reflectPower;
    return specLig;
}

/////////////////////////////////////////////////
// シャドウマップから影の強さ(0 ~ 1)を計算する
// 0 = 影なし 1 = 影
/////////////////////////////////////////////////
float CalcShadow(float4 posInLVP, float3 normal, float3 lightDir, float shadowBias , float shadowBiasMin ,Texture2D<float4> shadowMap, SamplerComparisonState shadowSampler)
{
    float2 shadowMapUV = posInLVP.xy / posInLVP.w;
    shadowMapUV *= float2 (0.5, -0.5f);
    shadowMapUV += 0.5f;

    // 範囲外は何もなし
    if(shadowMapUV.x < 0.0f || shadowMapUV.x > 1.0f || shadowMapUV.y < 0.0f || shadowMapUV.y > 1.0f)
    {
        return 0.0f;
    }

    float zInLVP = posInLVP.z / posInLVP.w;

    float3 L = normalize(lightDir);
    float bias = max(shadowBias * (1.0f - dot(normal, -L)), shadowBiasMin);

    return shadowMap.SampleCmpLevelZero(shadowSampler, shadowMapUV, zInLVP - bias);
}

//////////////////////////////////////////////////
// ポイントライトの拡散反射光を計算
//////////////////////////////////////////////////
float3 CalcPointLight(PTLight pt, float3 normal, float3 worldPos)
{
    // ライト → ピクセルの向き
    float3 ptLigDir = normalize(worldPos - pt.ptPosition);
    // ライトまでの距離
    float ptDistance = length(worldPos - pt.ptPosition);
    
    // 距離減衰 近い = 1 範囲の端 = 0
    // saturateは0~1に切り詰め
    float affect = saturate(1.0f - ptDistance / pt.ptRange);
    // 減衰カーブ
    affect = pow(affect, 3.0f);
    float t = max(0.0f, dot(normal, -ptLigDir));
    return pt.ptColor * t * affect;
}


///////////////////////////////////////////////////
// スポットライトの拡散反射光を計算
///////////////////////////////////////////////////
float3 CalcSpotLight(PTLight pt, float3 normal, float3 worldPos)
{
    float3 ligDir = normalize(worldPos - pt.ptPosition);
    float ligDis = length(worldPos - pt.ptPosition);

    // 距離による減衰
    float disAffect = saturate(1.0f - ligDis / pt.ptRange);
    disAffect = pow(disAffect, 3.0f);

    // 角度による減衰
    float cosAngle = dot(ligDir, normalize(pt.ptDirection));
    //cosAngle = clamp(cosAngle, -1.0f, 1.0f);
    float angle = abs(acos(cosAngle));
    
    float angleAffect = saturate(1.0f - angle / radians(pt.ptAngle));
    angleAffect = pow(angleAffect, 0.5f);

    //return float3(angleAffect, angleAffect, angleAffect);

    // 拡散反射光
    float t = max(0.0f,dot(normal, -ligDir));
    float3 diffuse = pt.ptColor * t;
    
    // 距離と角度の両方の影響をかける
    return diffuse * disAffect * angleAffect;
}


////////////////////////////////////////////////
// Pixel shader.
// For now: just output the albedo texture. Add your lighting here.
////////////////////////////////////////////////
float4 PSMain(SPSIn In) : SV_Target0
{
    // 長かったから関数を作って省略
    // 法線を復元
    float3 normal = CalcNormalFromMap(normalMap, Sampler, In.uv, In.tangent, In.biNormal, In.normal);
    // 拡散反射光
    float3 diffuseLig = CalcDiffuseLight(directionLight, normal);
    // スぺキュラマップをサンプリング
    float specularPower = specularMap.Sample(Sampler, In.uv).r;
    // 鏡面反射光
    float3 specularLig = CalcSpecularLight(directionLight,light, normal, In.worldPos, specularPower);
    
    float3 ptDiffuse = float3(0.0f,0.0f,0.0f);

    // ポイントライトの数分ループする
    for(int i = 0; i < numPtLights; i++)
    {
        // ポイントライトの拡散反射光を計算
        //ptDiffuse = CalcPointLight(ptLight[i], normal, In.worldPos);
        // スポットライトの拡散反射光を計算
        ptDiffuse += CalcSpotLight(ptLight[i], normal, In.worldPos);
    }

    // 最大数
    const int MAX_SHADOW = 4;
    // 合計値
    float shadowSum = 0.0f;
    for(int k = 0; k < MAX_SHADOW; k++)
    {
        // ライトビュープロジェクション行列
        float4 posInLVP = mul(mLVP[k], float4(In.worldPos, 1.0f));
        // 影の強さ
        float shadow = CalcShadow(posInLVP, normal, directionLight.direction, shadowParam.shadowBias, shadowParam.shadowBiasMin, g_shadowMap, g_shadowMapSampler);

        shadowSum += shadow;
    }
    
    // アルベドカラー
    float4 albedoColor = albedoTexture.Sample(Sampler, In.uv);
    // 環境光を除いたライト
    float3 nonAmbientLig = diffuseLig + specularLig;

    // 最終的な色
    float4 finalColor = albedoColor;
    finalColor.xyz = albedoColor.xyz * (ambientLight.ambient + nonAmbientLig * (1.0f - shadowSum)) + ptDiffuse;
    
    // TODO: add lighting. For example, start with ambient:
    //   float3 ambient = float3(0.3, 0.3, 0.3);
    //   albedoColor.xyz *= ambient;

    return finalColor;
}