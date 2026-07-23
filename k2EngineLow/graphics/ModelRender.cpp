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
	{}


	ModelRender::~ModelRender()
	{}


	void ModelRender::Init(const char* tkmFilePath, const bool isShadow, const bool reShadow, const char* fxFilePath)
	{
		// tkmファイルパスを設定
		m_modelInitData.m_tkmFilePath = tkmFilePath;
		// fxファイルパスを設定
		m_modelInitData.m_fxFilePath = fxFilePath;
		// 影を落とすかどうかを設定
		m_isShadowCaster = isShadow;
		// 影を受けるかどうかを設定
		m_isReceiveShadow = reShadow;
		
		// 影を落とす(シャドウキャスター)場合
		if (m_isShadowCaster == true)
		{
			m_shadowModelInitData.m_tkmFilePath = tkmFilePath;
			m_shadowModelInitData.m_fxFilePath = "Assets/shader/drawShadowMap.fx";
			// フォーマットが合っていないとD3D12 が実行時エラーを出す
			m_shadowModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;
			m_shadowModel.Init(m_shadowModelInitData);
		}

		//m_modelInitData.m_expandShaderResoruceView[0] =
		//	&nsK2EngineLow::RenderingEngine::GetInstance().GetShadowMapTexture();

		// 影を受ける(シャドウレシーバー)場合
		if (m_isReceiveShadow == true)
		{
			// シャドウマップのテクスチャを設定(t10に届く)
			m_modelInitData.m_expandShaderResoruceView[0] =
				&nsK2EngineLow::RenderingEngine::GetInstance().GetShadowMapTexture();

			// モデルを初期化
			//m_model.Init(m_modelInitData);
		}

		// Step1-5完成(環境光 ← ambient)
		// 定数バッファを設定
		//m_modelInitData.m_expandConstantBuffer = &ambientLight_;
		// 定数バッファのサイズを設定
		//m_modelInitData.m_expandConstantBufferSize = sizeof(ambientLight_);

		// Step1-6完成(拡散反射光 ← lambert)
		// 定数バッファを設定
		//m_modelInitData.m_expandConstantBuffer = &directionLight_;
		// 定数バッファのサイズを設定
		//m_modelInitData.m_expandConstantBufferSize = sizeof(directionLight_);

		// Step1-7(鏡面反射光 ← phong)
		// 定数バッファを設定
		//m_modelInitData.m_expandConstantBuffer = &m_lightCB;
		// 定数バッファのサイズを設定
		//m_modelInitData.m_expandConstantBufferSize = sizeof(m_lightCB);
		
		// Step1-10完成
		// ユーザ拡張の定数バッファにライトの定数バッファを設定
		m_modelInitData.m_expandConstantBuffer = &SceneLight::GetInstance().GetLightCB();
		// ユーザー拡張の定数バッファのサイズを設定
		m_modelInitData.m_expandConstantBufferSize = sizeof(SceneLight::LightCB);
		
		//SceneLight::GetInstance().SetAmbient(Vector3(1.0f, -0.5f, 1.0f));
		//SceneLight::GetInstance().SetColor(Vector3(1.0f,1.0f,1.0f));
		//SceneLight::GetInstance().SetDirection(Vector3(-1.0f,-1.0f,-1.0f));

		// モデルを初期化
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
	}


	void ModelRender::Draw(RenderContext& rc)
	{
		// 描画
		//m_model.Draw(rc);

		// モデルを直接描画からモデルを登録に変更
		RenderingEngine::GetInstance().AddModel(m_model);

		if (m_isShadowCaster) {
			// シャドウキャスターを登録
			RenderingEngine::GetInstance().AddShadowCaster(m_shadowModel);
		}
	}
}