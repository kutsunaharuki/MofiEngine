/**
 * @file DebugScene.cpp
 * @brief 開発用のシーン
 */
#include "stdafx.h"
#include "DebugScene.h"
#include "TitleScene.h"
#include "Source/Camera/GameCamera.h"
#include "imgui.h"
#include "Source/Actor/Character/Boy/Boy.h"
#include "Source/Actor/Character/Boy/BoyState.h"
#include "Source/Util/ParameterManager.h"
#include "InGameScene.h"


namespace app
{
	namespace
	{
		/** 主人公の最初の位置(落ちたときも、ここに戻す) */
		const Vector3 BOY_START_POSITION = Vector3(0.0f, 0.0f, 0.0f);
		/** これより下に落ちたら、最初の位置に戻す */
		constexpr float FALL_LIMIT_Y = -1000.0f;
		/** 地面のモデル */
		constexpr const char* GROUND_MODEL_PATH = "Assets/modelData/ground.tkm";


		/**
		 * @brief 主人公の今のステートの名前を取得
		 * @param boy 主人公
		 * @return ステートの名前
		 */
		const char* GetBoyStateName(const actor::Boy& boy)
		{
			if (boy.IsState(actor::BoyIdleState::ID())) return "Idle";
			if (boy.IsState(actor::BoyWalkState::ID())) return "Walk";
			if (boy.IsState(actor::BoyRunState::ID())) return "Run";
			if (boy.IsState(actor::BoyJumpState::ID())) return "Jump";
			if (boy.IsState(actor::BoyPhotoState::ID())) return "Photo";
			return "(none)";
		}
	}


	DebugScene::DebugScene()
		: m_shotCount(0)
	{}


	DebugScene::~DebugScene()
	{
		// PhotoCapture の登録を外す
		RenderingEngine::GetInstance().SetPhotoCapture(nullptr);
	}


	bool DebugScene::Start()
	{
		// 地面と、その当たり判定
		m_groundModelRender.Init(GROUND_MODEL_PATH, nullptr, enModelUpAxisZ, 0, false, true);
		m_groundModelRender.SetTRS(Vector3::Zero, Quaternion::Identity, Vector3::One);
		m_groundModelRender.Update();
		Model& groundModel = m_groundModelRender.GetModel();
		m_groundPhysicsObject.CreateFromModel(groundModel, groundModel.GetWorldMatrix());

		// 主人公(位置を決めてから Start を呼ぶ)
		m_boy = std::make_unique<actor::Boy>();
		m_boy->SetPosition(BOY_START_POSITION);
		m_boy->Start();

		// 写真撮影
		m_photoCapture.Init();
		RenderingEngine::GetInstance().SetPhotoCapture(&m_photoCapture);
		m_finderController.Initialize(&m_photoCapture);

		// カメラ(主人公を追いかけて、主人公はカメラの向きを基準に動く)
		m_gameCamera = std::make_unique<camera::GameCamera>();
		m_gameCamera->Start();
		m_gameCamera->SetTarget(&m_boy->GetTransform());
		m_boy->SetGameCamera(m_gameCamera.get());
		m_gameCamera->Update();
		return true;
	}


	void DebugScene::Update()
	{
		// ファインダーの操作(覗く・ピント・ズーム・撮影)
		const bool isShot = m_finderController.Update();
		if (isShot) m_shotCount++;

		// 主人公 → カメラの順に更新する(カメラが主人公の新しい位置を追いかける)
		m_boy->SetIsFinderActive(m_photoCapture.IsFinderActive());
		m_boy->Update();

		// 落ちてしまったら、最初の位置に戻す
		if (m_boy->GetTransform().GetWorldPosition().y < FALL_LIMIT_Y)
		{
			m_boy->Warp(BOY_START_POSITION);
		}

		m_gameCamera->SetFirstPeerson(m_photoCapture.IsFinderActive());
		m_gameCamera->Update();

		// ファインダーのカメラを、ゲームカメラに合わせる
		Camera& finderCamera = m_photoCapture.GetFinderCamera();
		finderCamera.SetPosition(g_camera3D->GetPosition());
		finderCamera.SetTarget(g_camera3D->GetTarget());
		finderCamera.Update();

		// 調整用パラメーターの編集画面と、今の状態の表示
		util::ParameterManager::GetInstance()->DrawImGui();
		DrawDebugWindow(isShot);
	}


	void DebugScene::Render(RenderContext& rc)
	{
		// ファインダーを覗いている間は、主人公を描画しない(1人称なので)
		if (!m_photoCapture.IsFinderActive())
		{
			m_boy->Render(rc);
		}
		m_groundModelRender.Draw(rc);
	}


	bool DebugScene::RequestScene(uint32_t& id, float& waitTime)
	{
		// Select でタイトルに戻る
		if (g_pad[0]->IsTrigger(enButtonSelect))
		{
			id = InGameScene::ID();
			waitTime = 0.5f;
			return true;
		}
		return false;
	}


	void DebugScene::DrawDebugWindow(const bool isShot)
	{
		ImGui::Begin("Debug");
		ImGui::TextDisabled("Select (Space) : back to title");

		if (ImGui::CollapsingHeader("Boy", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const Vector3& position = m_boy->GetTransform().GetWorldPosition();
			const Vector3& moveSpeed = m_boy->GetMoveSpeed();
			ImGui::Text("State     : %s", GetBoyStateName(*m_boy));
			ImGui::Text("Position  : %.1f, %.1f, %.1f", position.x, position.y, position.z);
			ImGui::Text("MoveSpeed : %.1f, %.1f, %.1f", moveSpeed.x, moveSpeed.y, moveSpeed.z);
			ImGui::Text("OnGround  : %s", m_boy->IsOnGround() ? "Yes" : "No");
			if (ImGui::Button("Reset Position"))
			{
				m_boy->Warp(BOY_START_POSITION);
			}
		}

		if (ImGui::CollapsingHeader("Finder", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Active    : %s", m_photoCapture.IsFinderActive() ? "Yes" : "No");
			ImGui::Text("CoolTime  : %.2f sec %s", m_finderController.GetCoolTimeRemaining(), m_finderController.CanShot() ? "(can shot)" : "");
			ImGui::Text("Shots     : %d%s", m_shotCount, isShot ? "  <- shot!" : "");
			ImGui::Text("Focus     : %.1f", m_photoCapture.GetFocusDistance());
		}

		ImGui::End();
	}
}
