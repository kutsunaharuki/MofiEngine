/**
 * @file ModelRender.cpp
 * @brief モデルレンダークラスの実装
 */
#include "k2EngineLowPreCompile.h"
//#include "ModelRender.h"


namespace nsK2EngineLow
{
	ModelRender::ModelRender()
		: m_modelInitData()
		, m_shadowModelInitData()
		, m_model()
		, m_shadowModel()
		, m_position(Vector3::Zero)
		, m_rotation(Quaternion::Identity)
		, m_scale(Vector3::One)
		, m_isShadowCaster(false)
		, m_isReceiveShadow(false)
		, m_animationSpeed(1.0f)
		, m_isAnimted(false)
		, m_skeleton()
	{}


	ModelRender::~ModelRender()
	{}


	void ModelRender::Init(
		const char* tkmFilePath,
		AnimationClip* animationClips,
		EnModelUpAxis upAxis,
		int numAnimClips,
		const bool isShadow,
		const bool reShadow)
	{
		// Step3-1
		/** 1. スケルトンの初期化 */
		// 同じ階層のtksファイルを読むため
		std::string skeletonFilePath = tkmFilePath;
		// replaceでtksに置き換える
		skeletonFilePath.replace(skeletonFilePath.length() - 3, 3, "tks");
		// スケルトン初期化
		m_skeleton.Init(skeletonFilePath.c_str());

		/** 2. Modelにスケルトンを渡す(骨の行列がt3に送られる) */
		// tkmファイルパスを設定
		m_modelInitData.m_tkmFilePath = tkmFilePath;
		// モデルの上方向を設定
		m_modelInitData.m_modelUpAxis = upAxis;
		// fxファイルパスを設定
		m_modelInitData.m_fxFilePath = "Assets/shader/model.fx";
		// スケルトンを設定(骨のアドレス)
		m_modelInitData.m_skeleton = &m_skeleton;
		// エントリーポイントを設定(スキンあり頂点シェーダー)
		m_modelInitData.m_vsSkinEntryPointFunc = "VSMainSkin";
		// 影を落とすかどうかを設定
		m_isShadowCaster = isShadow;
		// 影を受けるかどうかを設定
		m_isReceiveShadow = reShadow;

		

		/** 4. アニメーション(クリップが渡された時のみ) */
		if (animationClips != nullptr)
		{
			m_animation.Init(m_skeleton, animationClips, numAnimClips);
			m_isAnimted = true;
		}

		// 影を落とす(シャドウキャスター)場合
		if (m_isShadowCaster == true)
		{
			m_shadowModelInitData.m_tkmFilePath = tkmFilePath;
			m_shadowModelInitData.m_fxFilePath = "Assets/shader/drawShadowMap.fx";
			/** 3. 影用モデルにもスケルトンのアドレスを渡す */
			// ※ これを渡さないと影だけTぽーすになってしまう
			m_shadowModelInitData.m_skeleton = &m_skeleton;
			m_shadowModelInitData.m_vsSkinEntryPointFunc = "VSMainSkin";
			// フォーマットが合っていないとD3D12 が実行時エラーを出す
			m_shadowModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;
			m_shadowModel.Init(m_shadowModelInitData);
		}

		// 影を受ける(シャドウレシーバー)場合
		if (m_isReceiveShadow == true)
		{
			// シャドウライトの数だけSRVを設定
			for (int i = 0; i < RenderingEngine::GetInstance().GetMaxShadowCount(); i++)
			{
				m_modelInitData.m_expandShaderResoruceView[i] 
					= &nsK2EngineLow::RenderingEngine::GetInstance().GetShadowMapTexture(i);
				m_model.Init(m_modelInitData);
			}
		}

		// ユーザ拡張の定数バッファにライトの定数バッファを設定
		m_modelInitData.m_expandConstantBuffer = &SceneLight::GetInstance().GetLightCB();
		// ユーザー拡張の定数バッファのサイズを設定
		m_modelInitData.m_expandConstantBufferSize = sizeof(SceneLight::LightCB);
		// モデルの初期化
		m_model.Init(m_modelInitData);
	}


	void ModelRender::Update()
	{
		// ワールド行列の更新
		m_model.UpdateWorldMatrix(m_position, m_rotation, m_scale);

		if (m_isShadowCaster)
		{
			m_shadowModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		}

		// 初期化済みかをチェック
		if (m_skeleton.IsInited())
		{
			m_skeleton.Update(m_model.GetWorldMatrix());
		}

		// アニメーションの時間を進める(1フレーム分)
		if (m_isAnimted)
		{
			m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);
		}
	}


	void ModelRender::Draw(RenderContext& rc)
	{
		// モデルを直接描画からモデルを登録に変更
		RenderingEngine::GetInstance().AddModel(m_model);

		if (m_isShadowCaster) {
			// シャドウキャスターを登録
			RenderingEngine::GetInstance().AddShadowCaster(m_shadowModel);
		}
	}
}