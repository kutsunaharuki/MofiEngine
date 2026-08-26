#include "stdafx.h"
#include "Game.h"
#include "GameCamera.h"
#include "imgui.h"


namespace
{
	/** 移動速度 */
	constexpr float MOVE_SPEED = 1.0f;
	/** 回転速度 */
	constexpr float ROTATE_SPEED = 2.0f;
	/** 拡縮速度 */
	constexpr float SCALE_SPEED = 0.01f;
	/** 初期位置 */
	const Vector3 INITIALIZE_POSITION = { 0.0f,0.0f,0.0f };
	/** 初期スケール */
	const Vector3 INITIALIZE_SCALE = { 2.0f,2.0f,2.0f };
}


Game::Game()
	: m_spriteRender(nullptr)
	, m_modelRender(nullptr)
	, m_position(INITIALIZE_POSITION)
	, m_rotation(Quaternion::Identity)
	, m_scale(Vector3::One)
	, m_gPosition(Vector3::Zero)
	, m_gRotation(Quaternion::Identity)
	, m_gScale(Vector3::One)
	, m_gameCamera(nullptr)
	, m_groundModelRender(nullptr)
{}


Game::~Game()
{
	if (m_spriteRender != nullptr)
	{
		delete m_spriteRender;
		m_spriteRender = nullptr;
	}
	if (m_modelRender != nullptr)
	{
		delete m_modelRender;
		m_modelRender = nullptr;
	}
	if (m_groundModelRender != nullptr)
	{
		delete m_groundModelRender;
		m_groundModelRender = nullptr;
	}
}


bool Game::Start()
{	
	// Step1-4完成
	m_modelRender = new ModelRender;
	m_animClips[0].Load("Assets/animData/idle.tka");
	m_animClips[0].SetLoopFlag(true);
	m_animClips[1].Load("Assets/animData/run.tka");
	m_animClips[1].SetLoopFlag(true);

	m_modelRender->Init("Assets/modelData/unityChan.tkm", m_animClips, enModelUpAxisY, 2);
	m_modelRender->SetTRS(m_position, m_rotation, m_scale);
	m_modelRender->PlayAnimation(0);
	m_modelRender->Update();
	
	// 地面のモデルレンダーを初期化
	m_groundModelRender = new ModelRender;
	m_groundModelRender->Init("Assets/modelData/ground.tkm", nullptr, enModelUpAxisZ, 0, false, true);
	m_groundModelRender->SetTRS(m_gPosition, m_gRotation, m_gScale);
	m_groundModelRender->Update();

	// 後々に響きそうなのでゲームカメラを作成しておく。
	m_gameCamera = new app::camera::GameCamera;
	m_gameCamera->Start();
	m_gameCamera->SetTarget(m_modelRender);
	m_gameCamera->Update();

	return true;
}


void Game::Update()
{
	// ライトの定数バッファの参照を取得
	auto& light = SceneLight::GetInstance().GetLightCB();
	// ライト関連のImGuiウィンドウ(結果もここ)
	ImGui::Begin("Lighting Settings");
	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::SliderFloat3("Direction", &light.directionLight.direction.x, -1.0f, 1.0f);
		ImGui::ColorEdit3("Color", &light.directionLight.color.x);
		ImGui::ColorEdit3("Ambient", &light.ambientLight.ambient.x);
		ImGui::SliderFloat("Spec Power", &light.light.specPower, 1.0f, 200.0f);
	}
	// 影の調整(調整しなくていい気がする)
	if (ImGui::CollapsingHeader("Shadow"))
	{
		ImGui::SliderFloat("Shadow Bias", &light.shadowParam.shadowBias, 0.0f, 1.0f);
		ImGui::SliderFloat("Shadow Bias Min", &light.shadowParam.shadowBiasMin, 0.0f, 1.0f);
	}
	// ポイントライトの調整
	if (ImGui::CollapsingHeader("PointLight"))
	{
		ImGui::SliderInt("Count", &light.numPointLights, 0, 4);
		for (int i = 0; i < light.numPointLights; i++)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode("", "Light %d", i))
			{
				ImGui::DragFloat3("Position", &light.ptLights[i].ptPosition.x, 5.0f);
				ImGui::ColorEdit3("Color", &light.ptLights[i].ptColor.x);
				ImGui::DragFloat("Range", &light.ptLights[i].ptRange, 5.0f, 0.0f, 2000.0f);
				ImGui::DragFloat3("Direction", &light.ptLights[i].ptDirection.x, 0.01f, -1.0f, 1.0f);
				ImGui::DragFloat("Angle", &light.ptLights[i].ptAngle, 1.0f, 0.0f, 180.0f);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
	// ガウシアンブラーの調整
	if (ImGui::BeginTabBar("PostEffectTabs"))
	{
		if (ImGui::BeginTabItem("GaussianBlur"))
		{
			ImGui::DragFloat("Blur Power", &RenderingEngine::GetInstance().GetScreenBlurPower(), 0.1f, 0.0f, 1.0f);
			ImGui::EndTabItem();
		}

		// ブルームの調整
		auto& re = RenderingEngine::GetInstance();
		if (ImGui::BeginTabItem("Bloom"))
		{
			ImGui::Checkbox("Bloom Enable", &re.IsEnableBloom());

			float threshold = re.GetBloomCB().threshold;
			// ブルームの閾値
			if (ImGui::SliderFloat("Threshold", &threshold, 0.0f, 3.0f))
			{
				re.SetBloomThreshold(threshold);
			}

			// ブルームの強さ
			float intensity = re.GetBloomIntensity();
			if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 3.0f))
			{
				re.SetBloomIntensity(intensity);
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();

	m_rotation.SetRotationDegY(180.0f);
	m_modelRender->SetRotation(m_rotation);


	if (g_pad[0]->IsPress(enButtonA))
	{
		m_modelRender->PlayAnimation(1, 0.3f);
	}
	else
	{
		m_modelRender->PlayAnimation(0, 0.3f);
	}

	m_modelRender->Update();
	m_gameCamera->Update();
}

void Game::Render(RenderContext& rc)
{
	// モデル描画(ユニティちゃん)
	m_modelRender->Draw(rc);
	// モデル描画(地面)
	m_groundModelRender->Draw(rc);
}