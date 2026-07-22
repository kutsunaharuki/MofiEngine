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
};


///////////////////////////////////////
// Common vertex shader code.
// Provides: ModelCb(b0: mWorld/mView/mProj), SVSIn, bone matrices (t3),
//           and the entry points VSMain / VSMainSkin / VSMainInstancing, etc.
///////////////////////////////////////
#include "ModelVSCommon.h" // ModelCB : register(b0)はこの中に入ってる ※ b0は共通で使っているので被ってはならない

/** 環境光の定数バッファ */
// cbuffer AmbientLightCB : register(b1)
// {
//     AmbientLight ambientLight; // 環境光
// };


/** ディレクションライトの定数バッファ */
// cbuffer DirectionLightCB : register(b1)
// {
//    DirectionLight directionLight; // ディレクションライト
// };


/** ライトの定数バッファ */
// 環境光とか平行光源とかをまとめて所持する
cbuffer LightCB : register(b1)
{
    DirectionLight directionLight;  // ディレクションライト
    AmbientLight ambientLight;      // 環境光
    Light light;                    // ライト
    float4x4 mLVP;                  // ライトビュープロジェクション行列
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

////////////////////////////////////////////////
// Pixel shader.
// For now: just output the albedo texture. Add your lighting here.
////////////////////////////////////////////////
float4 PSMain(SPSIn In) : SV_Target0
{
    // Step1-6よりnormalを使っていたのをlocalNormalに変更する
    float3 localNormal = normalMap.Sample(Sampler,In.uv).xyz;

    // 法線マップに書き込まれている法線は0.0~-1.0で、
    // 負の数になっていないので負の数にする
    localNormal = (localNormal, -0.5f) * 2.0f;

    // 法線を回転させて、オブジェクト空間からワールド空間に変換する
    float3 normal = normalize(
          In.tangent  * localNormal.x  // 接線
        + In.biNormal * localNormal.y  // 従法線
        + In.normal   * localNormal.z  // 元の法線
    );
    // 上記のnormalをdiffuse と specular で扱う


    //-----------------------------------------//
    // Step1-6完成
    // 補間で長さが縮むので正規化する
    //float3 normal = normalize(In.normal);

    // ピクセルの法線とライトの方向の内積を計算
    float t0 = max(0.0f,dot(normal, -directionLight.direction));

    // 拡散反射光を求める
    float3 diffuseLig = directionLight.color * t0;
    //-----------------------------------------//

    // Step1-7完成
    // 反射ベクトルを求める
    float3 refVec = reflect(directionLight.direction, normal);

    // 視点に伸びるベクトルを求めてついでに正規化する
    float3 toEye = normalize(light.eyePos - In.worldPos);

    // refVec と toEyeの内積から鏡面反射の光を絞る(specPowerはC++側で値の変更を容易にした)
    float t1 = pow(max(0.0f,dot(refVec, toEye)), light.specPower);

    // Step1-8完成
    // スぺキュラマップからスぺキュラ反射光をサンプリングする
    float specularPower = specularMap.Sample(Sampler, In.uv).r;

    // 鏡面反射光を求める
    float3 specularLig = directionLight.color * t1;

    // 鏡面反射光に乗算する
    specularLig *= specularPower * light.reflectPower;
    //------------------------------------------//

    // 3. 環境光と拡散反射光と鏡面反射光を足し算して、最終的な光を求める
    float3 lig = ambientLight.ambient + diffuseLig + specularLig;
    //------------------------------------------//

    // (お試し) 鏡面反射光と拡散反射光を足し算してみた
    //float3 lig = diffuseLig + specularLig;

    // 2.環境光と拡散反射光を足し算して光を求める
    //float3 lig = ambient + diffuseLig;

    // 1. 環境光を渡す
    //float3 lig = ambientLight.ambient;

    //------------------------------------------//
    // Step2-2
    // ライトから見た位置へ変換
    float4 posInLVP = mul(mLVP,float4(In.worldPos,1.0f));

    // -1 ~ 1 の範囲に
    float2 shadowMapUV = posInLVP.xy / posInLVP.w;
    shadowMapUV *= float2(0.5f, -0.5f);

    // 0~1のUVに変換(Yは上下反転)
    shadowMapUV += 0.5f;

    // 素のモデルのテクスチャの色
    float4 albedoColor = albedoTexture.Sample(Sampler, In.uv);

    // 最終的な色
    float4 finalColor = albedoColor;

    // シャドウマップに写っている範囲だけ
    if(shadowMapUV.x > 0.0f && shadowMapUV.x < 1.0f && shadowMapUV.y > 0.0f && shadowMapUV.y < 1.0f)
    {
        float3 shadow = g_shadowMap.Sample(Sampler,shadowMapUV).xyz;
        // 黒(0)が描かれていたら暗くなって、白(1)ならそのまま
        finalColor.xyz *= shadow;
    }


    //-----------------------------------------//
    // Step1-5完成
    //float3 lig = ambientLight.ambient;


    // 3. 環境光 + 拡散反射光 + 鏡面反射光 の結果の lig を渡す
    finalColor.xyz *= lig;

    // 2. 環境光 + 拡散反射光 の結果の lig を渡す
    //albedoColor.xyz *= lig;

    // 1. 環境光そのままのものの lig を渡す(一律で環境光を上げる)
    //albedoColor.xyz *= lig;
    //-----------------------------------------//
    

    // TODO: add lighting. For example, start with ambient:
    //   float3 ambient = float3(0.3, 0.3, 0.3);
    //   albedoColor.xyz *= ambient;

    return finalColor;
}