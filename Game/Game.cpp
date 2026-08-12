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
	// ライト調整ウィンドウ
	ImGui::Begin("Light");
	auto& light = SceneLight::GetInstance().GetLightCB();
	// ウィンドウの幅と高さを設定
	ImGui::SetNextWindowSize(ImVec2(800,300), ImGuiCond_Once);
	ImGui::SliderFloat3("Direction", &light.directionLight.direction.x, -1.0f, 1.0f);
	ImGui::ColorEdit3("Color", &light.directionLight.color.x);
	ImGui::ColorEdit3("Ambient", &light.ambientLight.ambient.x);
	ImGui::SliderFloat("Spec Power", &light.light.specPower, 1.0f, 200.0f);
	ImGui::End();

	// 影調整ウィンドウ
	ImGui::Begin("Shadow");
	ImGui::SetNextWindowSize(ImVec2(800,300), ImGuiCond_Once);
	ImGui::SliderFloat("Shadow Bias", &light.shadowParam.shadowBias, 0.0f, 1.0f);
	ImGui::SliderFloat("Shadow Bias Min", &light.shadowParam.shadowBiasMin, 0.0f, 1.0f);
	ImGui::End();

	// ポイントライト調整ウィンドウ
	ImGui::Begin("PointLight");
	ImGui::SliderInt("Count", &light.numPointLights, 0, 4);
	for (int i = 0; i < light.numPointLights; i++)
	{
		ImGui::PushID(i);
		if (ImGui::TreeNode("", "Light %d", i))
		{
			ImGui::SetNextWindowSize(ImVec2(800, 300), ImGuiCond_Once);
			ImGui::DragFloat3("Position", &light.ptLights[i].ptPosition.x, 5.0f);
			ImGui::ColorEdit3("Color", &light.ptLights[i].ptColor.x);
			ImGui::DragFloat("Range", &light.ptLights[i].ptRange, 5.0f, 0.0f, 2000.0f);
			ImGui::DragFloat3("Direction", &light.ptLights[i].ptDirection.x, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat("Angle", &light.ptLights[i].ptAngle, 1.0f, 0.0f, 180.0f);
			ImGui::TreePop();
		}
		ImGui::PopID();
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