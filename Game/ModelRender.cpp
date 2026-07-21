#include "stdafx.h"
#include "ModelRender.h"


namespace nsK2Engine
{
	ModelRender::ModelRender()
		: m_modelInitData()
		, m_model()
		, m_position(Vector3::Zero)
		, m_rotation(Quaternion::Identity)
		, m_scale(Vector3::One)
	{}


	ModelRender::~ModelRender()
	{}


	void ModelRender::Init(const char* tkmFilePath, const char* fxFilePath, const bool isShadow, const bool reShadow)
	{
		// tkmファイルパスを設定
		m_modelInitData.m_tkmFilePath = tkmFilePath;
		// fxファイルパスを設定
		m_modelInitData.m_fxFilePath = fxFilePath;
		// 影を落とすかどうかを設定
		m_isShadow = isShadow;
		// 影を受けるかどうかを設定
		m_isReceiveShadow = reShadow;

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
		
		// Step1-10
		// ユーザ拡張の定数バッファにライトの定数バッファを設定
		m_modelInitData.m_expandConstantBuffer = &SceneLight::GetInstance().GetLightCB();
		// ユーザー拡張の定数バッファのサイズを設定
		m_modelInitData.m_expandConstantBufferSize = sizeof(SceneLight::LightCB);
		// モデルを初期化
		m_model.Init(m_modelInitData);
	}


	void ModelRender::Update()
	{
		// ワールド行列の更新
		m_model.UpdateWorldMatrix(m_position, m_rotation, m_scale);
	}


	void ModelRender::Draw(RenderContext& rc)
	{
		// 描画
		//m_model.Draw(rc);

		// モデルを直接描画からモデルを登録に変更
		nsK2EngineLow::RenderingEngine::GetInstance().AddModel(m_model);
	}
}