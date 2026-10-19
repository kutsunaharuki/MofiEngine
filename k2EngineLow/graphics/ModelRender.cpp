/**
 * @file ModelRender.cpp
 * @brief モデルレンダークラスの実装
 */
#include "k2EngineLowPreCompile.h"


namespace nsK2EngineLow
{
	ModelRender::ModelRender()
		: m_modelInitData()
		, m_shadowModelInitData()
		, m_forwardModel()
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
		const bool isSkeletonLoaded = m_skeleton.Init(skeletonFilePath.c_str());

		// アニメーションするのにスケルトンが無いと、ボーンが0本のまま進み、
		// 頂点シェーダーがボーン行列の範囲外を読んでしまう(敵の影が出ない・範囲外アクセスの原因になる)
		// ※ .tks は .tkm と同じフォルダーに、同じ名前で置く
		if (!isSkeletonLoaded && animationClips != nullptr)
		{
			// ※ K2_ASSERT のメッセージは 256 バイトまで(ファイル名と行番号を含む)なので、短くする
			std::string message = "tks not found : ";
			message += skeletonFilePath;
			message += "\n";
			K2_ASSERT(false, message.c_str());
		}


		ModelInitData deferredInitData;
		deferredInitData.m_tkmFilePath = tkmFilePath;
		deferredInitData.m_fxFilePath = "Assets/shader/renderToGBuffer.fx";
		deferredInitData.m_skeleton = &m_skeleton;
		// スキン(骨で頂点を動かす)用の頂点シェーダーを指定する
		// ※ 指定しないと既定の "VSMain" になり、アニメーションしても画面のモデルは T ポーズのまま動かない
		deferredInitData.m_vsSkinEntryPointFunc = "VSMainSkin";
		// モデルの上方向(フォワード用・ファインダー用と合わせる)
		deferredInitData.m_modelUpAxis = upAxis;

		// パイプラインは絵描き先のフォーマットを知っている必要がある
		// ※ RenderingEngine の G-Buffer(m_gAlbedoRT / m_gNormalRT / m_gDepthRT)のフォーマットと必ず合わせる
		deferredInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;		// アルベド(色なので 0~1 で十分)
		deferredInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;	// 法線 + スペキュラの強さ
		deferredInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// ワールド座標(影の比較に使うので精度を高くする)
		deferredInitData.m_expandConstantBuffer = &SceneLight::GetInstance().GetLightCB();
		deferredInitData.m_expandConstantBufferSize = sizeof(SceneLight::LightCB);
		m_deferredModel.Init(deferredInitData);

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

		// モデルの何軸を上方向として扱うかを設定
		m_modelInitData.m_modelUpAxis = upAxis;

		m_modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
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
			// モデルの上方向(本体と合わせないと、影だけ寝てしまう)
			m_shadowModelInitData.m_modelUpAxis = upAxis;
			// フォーマットが合っていないとD3D12 が実行時エラーを出す
			m_shadowModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;
			m_shadowModel.Init(m_shadowModelInitData);
		}

		// 影を受ける(シャドウレシーバー)場合
		if (m_isReceiveShadow == true)
		{
			int srvNo = 0;

			// 来た順に拡張SRVに詰めるような処理になる(これで枚数を知らずに入れることができるようになった)
			RenderingEngine::GetInstance().QueryShadowMapTexture([&](Texture& shadowMap)
			{
				m_modelInitData.m_expandShaderResoruceView[srvNo] = &shadowMap;
				srvNo++;
			});

			// 拡張定数バッファにライトの定数バッファを設定
			m_modelInitData.m_expandConstantBuffer = &SceneLight::GetInstance().GetLightCB();
			// 拡張定数バッファのサイズを設定
			m_modelInitData.m_expandConstantBufferSize = sizeof(SceneLight::LightCB);
			// ファインダー・写真用のモデルをフォワード用で初期化
			m_finderModel.Init(m_modelInitData);
		}
	}


	void ModelRender::Update()
	{
		// ディファードモデルのワールド行列を更新
		m_deferredModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		// ファインダーモデルのワールド行列を更新
		m_finderModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		
		if (m_isShadowCaster)
		{
			m_shadowModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		}

		// 初期化済みかをチェック
		if (m_skeleton.IsInited())
		{
			m_skeleton.Update(m_deferredModel.GetWorldMatrix());
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
		RenderingEngine::GetInstance().AddModel(m_deferredModel);

		// ファインダーモデルを登録
		RenderingEngine::GetInstance().AddFinderModel(m_finderModel);

		if (m_isShadowCaster) {
			// シャドウキャスターを登録
			RenderingEngine::GetInstance().AddShadowCaster(m_shadowModel);
		}
	}
}