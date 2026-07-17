#include "stdafx.h"
#include "ModelRender.h"


namespace nsK2Engine
{
	ModelRender::ModelRender()
		: modelInitData_()
		, model_()
		, position_(Vector3::Zero)
		, rotation_(Quaternion::Identity)
		, scale_(Vector3::One)
	{}


	ModelRender::~ModelRender()
	{}


	void ModelRender::Init(const char* tkmFilePath, const char* fxFilePath)
	{
		// tkmファイルパスを設定
		modelInitData_.m_tkmFilePath = tkmFilePath;
		// fxファイルパスを設定
		modelInitData_.m_fxFilePath = fxFilePath;

		// Step1-5完成(環境光 ← ambient)
		// 定数バッファを設定
		//modelInitData_.m_expandConstantBuffer = &ambientLight_;
		// 定数バッファのサイズを設定
		//modelInitData_.m_expandConstantBufferSize = sizeof(ambientLight_);

		// Step1-6完成(拡散反射光 ← lambert)
		// 定数バッファを設定
		//modelInitData_.m_expandConstantBuffer = &directionLight_;
		// 定数バッファのサイズを設定
		//modelInitData_.m_expandConstantBufferSize = sizeof(directionLight_);

		// Step1-7(鏡面反射光 ← phong)
		// 定数バッファを設定
		//modelInitData_.m_expandConstantBuffer = &lightCB_;
		// 定数バッファのサイズを設定
		//modelInitData_.m_expandConstantBufferSize = sizeof(lightCB_);
		
		// Step1-10
		// ユーザ拡張の定数バッファにライトの定数バッファを設定
		modelInitData_.m_expandConstantBuffer = &SceneLight::GetInstance().GetLightCB();
		// ユーザー拡張の定数バッファのサイズを設定
		modelInitData_.m_expandConstantBufferSize = sizeof(SceneLight::LightCB);

		// モデルを初期化
		model_.Init(modelInitData_);
	}


	void ModelRender::Update()
	{
		// ワールド行列の更新
		model_.UpdateWorldMatrix(position_, rotation_, scale_);
	}


	void ModelRender::Draw(RenderContext& rc)
	{
		// 描画
		model_.Draw(rc);
	}
}