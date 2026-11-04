/**
 * @file InstancedModelRender.cpp
 * @brief インスタンス化モデルの描画
 */
#include "k2EngineLowPreCompile.h"


namespace nsK2EngineLow
{
	namespace
	{
		/**
		 * 行列リストを入れる拡張SRVの番号
		 * ※ 拡張SRVは t10 から始まるので、4番 = t14(ModelVSCommon.hlsli の g_worldMatrixArray と必ず合わせる)
		 *    0~3番(t10~t13)はシャドウマップが使う
		 */
		constexpr int INSTANCE_MATRIX_SRV_NO = 4;
	}


	InstancedModelRender::InstancedModelRender()
		: m_numInstance(0)
		, m_maxInstance(0)
	{}


	InstancedModelRender::~InstancedModelRender()
	{}


	void InstancedModelRender::Init(const char* tkmFilePath, int maxInstance, EnModelUpAxis upAxis, D3D12_CULL_MODE cullMode)
	{
		m_maxInstance = maxInstance;
		m_numInstance = 0;
		m_worldMatrices.assign(m_maxInstance, Matrix::Identity);

		/** 1. 行列リストを作る(Model より先に作る。Model を作る時にディスクリプタヒープに登録されるため) */
		m_worldMatrixSB.Init(sizeof(Matrix), m_maxInstance, m_worldMatrices.data());

		// 拡張SRV:0~3番にシャドウマップ、4番に行列リスト
		std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV> expandSRV = {};
		int srvNo = 0;
		RenderingEngine::GetInstance().QueryShadowMapTexture([&](Texture& shadowMap)
		{
			expandSRV[srvNo] = &shadowMap;
			srvNo++;
		});
		// シャドウマップが増えると行列リストの番号と重なるので止める
		K2_ASSERT(srvNo <= INSTANCE_MATRIX_SRV_NO, "shadow maps overlap instance matrix SRV(t14)");
		expandSRV[INSTANCE_MATRIX_SRV_NO] = &m_worldMatrixSB;


		/** 2. ディファード用のモデル(G-Buffer に描く。設定は ModelRender と同じ) */
		ModelInitData deferredInitData;
		deferredInitData.m_tkmFilePath = tkmFilePath;
		deferredInitData.m_fxFilePath = "Assets/shader/renderToGBuffer.fx";
		// インスタンシング用の頂点シェーダー(SV_InstanceID 番目の行列を使う)
		deferredInitData.m_vsEntryPointFunc = "VSMainInstancing";
		deferredInitData.m_vsSkinEntryPointFunc = "VSMainSkinInstancing";
		deferredInitData.m_modelUpAxis = upAxis;
		deferredInitData.m_cullMode = cullMode;
		// ※ RenderingEngine の G-Buffer のフォーマットと必ず合わせる
		deferredInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		deferredInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		deferredInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		deferredInitData.m_expandConstantBuffer = &SceneLight::GetInstance().GetLightCB();
		deferredInitData.m_expandConstantBufferSize = sizeof(SceneLight::LightCB);
		// renderToGBuffer.fx はシャドウマップを読まないが、0~3番を空けると未初期化のディスクリプタが残るので埋めておく
		deferredInitData.m_expandShaderResoruceView = expandSRV;
		m_deferredModel = std::make_unique<Model>();
		m_deferredModel->Init(deferredInitData);


		/** 3. ファインダー用のモデル(写真・ファインダーに描く。設定は ModelRender と同じ) */
		ModelInitData finderInitData;
		finderInitData.m_tkmFilePath = tkmFilePath;
		finderInitData.m_fxFilePath = "Assets/shader/model.fx";
		finderInitData.m_vsEntryPointFunc = "VSMainInstancing";
		finderInitData.m_vsSkinEntryPointFunc = "VSMainSkinInstancing";
		finderInitData.m_modelUpAxis = upAxis;
		finderInitData.m_cullMode = cullMode;
		finderInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		finderInitData.m_expandConstantBuffer = &SceneLight::GetInstance().GetLightCB();
		finderInitData.m_expandConstantBufferSize = sizeof(SceneLight::LightCB);
		finderInitData.m_expandShaderResoruceView = expandSRV;
		m_finderModel = std::make_unique<Model>();
		m_finderModel->Init(finderInitData);
	}


	void InstancedModelRender::ClearInstance()
	{
		m_numInstance = 0;
	}


	void InstancedModelRender::AddInstance(const Matrix& worldMatrix)
	{
		// 最大数を超えた分は描かない(行列リストの外に書いてしまうため)
		if (m_numInstance >= m_maxInstance) return;

		m_worldMatrices[m_numInstance] = worldMatrix;
		m_numInstance++;
	}


	void InstancedModelRender::Draw(RenderContext& rc)
	{
		// 1つも映っていなければ、登録しない
		if (m_numInstance == 0) return;

		// 行列リストを GPU に送る(今のバックバッファの方に書く)
		m_worldMatrixSB.Update(m_worldMatrices.data());

		// 何個まとめて描くかを Model に覚えさせる(RenderingEngine が Draw に渡す)
		m_deferredModel->SetNumInstance(m_numInstance);
		m_finderModel->SetNumInstance(m_numInstance);

		// 描画は RenderingEngine がまとめて行う(ModelRender::Draw と同じ)
		RenderingEngine::GetInstance().AddModel(*m_deferredModel);
		RenderingEngine::GetInstance().AddFinderModel(*m_finderModel);
	}
}
