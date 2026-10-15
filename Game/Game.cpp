#include "stdafx.h"
#include "Game.h"
#include "GameCamera.h"
#include "imgui.h"
#include "Enemy.h"


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

	/** 子の座標 */
	const Vector3 CHILD_POSITION = { 10.0f,0.0f,0.0f};
	/** 拡大の比率 */
	constexpr float ZOOM_IN_RATE = 1.0f;
	/** 縮小の比率 */
	constexpr float ZOOM_OUT_RATE = -1.0f;

	/** フォーカスの最大値 */
	constexpr float FOCUS_MAX = 1.0f;
	/** フォーカスの最小値 */
	constexpr float FOCUS_MIN = -1.0f;
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
	, m_isLastShotInFocus(false)
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

	// PhotoCaptureの登録を外す
	RenderingEngine::GetInstance().SetPhotoCapture(nullptr);
}


bool Game::Start()
{	
	// Step1-4完成
	m_modelRender = new ModelRender;
	m_animClips[0].Load("Assets/animData/idle.tka");
	m_animClips[0].SetLoopFlag(true);
	m_animClips[1].Load("Assets/animData/run.tka");
	m_animClips[1].SetLoopFlag(true);

	m_modelRender->Init("Assets/modelData/unityChan.tkm", m_animClips, enModelUpAxisZ, 2);
	m_modelRender->SetTRS(m_position, m_rotation, m_scale);
	m_modelRender->PlayAnimation(0);
	m_modelRender->Update();
	
	// 地面のモデルレンダーを初期化
	m_groundModelRender = new ModelRender;
	m_groundModelRender->Init("Assets/modelData/ground.tkm", nullptr, enModelUpAxisZ, 0, false, true);
	m_groundModelRender->SetTRS(m_gPosition, m_gRotation, m_gScale);
	m_groundModelRender->Update();

	// 写真撮影クラスの初期化
	m_photoCapture.Init();
	// RenderingEngineにPhotoCaptureを登録
	RenderingEngine::GetInstance().SetPhotoCapture(&m_photoCapture);

	m_enemy = std::make_unique<app::Enemy>();
	m_enemy->Init();
	m_enemy->Update();

	// 後々に響きそうなのでゲームカメラを作成しておく。
	m_gameCamera = new app::camera::GameCamera;
	m_gameCamera->Start();
	m_transform.SetLocalPosition(m_position);
	m_transform.UpdateTransform();
	m_gameCamera->SetTarget(&m_transform);
	m_gameCamera->Update();
	return true;
}


void Game::Update()
{
	if (g_pad[0]->IsPress(enButtonA))
	{
		m_modelRender->PlayAnimation(1, 0.3f);
	}
	else
	{
		m_modelRender->PlayAnimation(0, 0.3f);
	}

	m_modelRender->Update();

	// ファインダーのON/OFFを切り替える
	if (g_pad[0]->IsTrigger(enButtonY))
	{
		m_photoCapture.ToggleFinderActive();
	}

	// ファインダーを覗いている間だけ、1人称にする
	m_gameCamera->SetFirstPeerson(m_photoCapture.IsFinderActive());
	m_gameCamera->Update();

	// ファインダー視点のカメラをゲームカメラの位置と向きに合わせる
	Camera& finderCamera = m_photoCapture.GetFinderCamera();
	finderCamera.SetPosition(g_camera3D->GetPosition());
	finderCamera.SetTarget(g_camera3D->GetTarget());
	finderCamera.Update();

	// 敵の位置更新
	Quaternion enemyRot;
	enemyRot.SetRotationDegY(180.0f);
	m_enemy->SetRotation(enemyRot);
	m_enemy->Update();

	// 敵の位置からファインダー視点のカメラまでの距離
	Vector3 toEnemy = m_enemy->GetPosition() - m_photoCapture.GetFinderCamera().GetPosition();
	float enemyDistance = toEnemy.Length();

	// 今のピントが敵に合っているかどうか
	bool isInFocus = m_photoCapture.IsInFocus(enemyDistance);
	
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

	if (ImGui::BeginTabBar("DoF"))
	{
		// DoF(ピントのぼけ)の調整
		if (ImGui::BeginTabItem("DoF"))
		{
			// ファインダーを覗いているときだけ、効果が見える
			ImGui::Text("Finder Active: %s", m_photoCapture.IsFinderActive() ? "Yes" : "No");

			// ピント距離
			float distance = m_photoCapture.GetFocusDistance();
			if (ImGui::SliderFloat("Distance", &distance, 0.0f, 1000.0f))
			{
				m_photoCapture.SetFocusDistance(distance);
			}

			// ボケ量が最大になるまでの幅
			float blurRange = m_photoCapture.GetFocusBlurRange();
			if (ImGui::SliderFloat("Blur Range", &blurRange, 1.0f, 1000.0f))
			{
				m_photoCapture.SetFocusBlurRange(blurRange);
			}

			// ピントが合っているとみなす許容値
			float tolerance = m_photoCapture.GetFocusTolerance();
			if (ImGui::SliderFloat("Tolerance", &tolerance, 1.0f, 200.0f))
			{
				m_photoCapture.SetFocusTolerance(tolerance);
			}

			// 敵の距離とピントの距離の差を表示
			ImGui::Text("Enemy Distance: %.1f", enemyDistance);
			// ファインダーのピント距離を表示
			ImGui::Text("Focus Distance: %.1f", m_photoCapture.GetFocusDistance());
			// ピント距離と敵の距離の差を表示
			ImGui::Text("Difference: %.1f", fabsf(m_photoCapture.GetFocusDistance() - enemyDistance));
			// ピントが合っていたらYes, 合っていないならNoを表示
			ImGui::Text("In Focus: %s", isInFocus ? "Yes" : "No");

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();


	// ピントを合わせる
	if (m_photoCapture.IsFinderActive() && g_pad[0]->IsPress(enButtonUp))
	{
		m_photoCapture.FinderFocus(FOCUS_MAX);
	}
	// ピントを外す
	if (m_photoCapture.IsFinderActive() && g_pad[0]->IsPress(enButtonDown))
	{
		m_photoCapture.FinderFocus(FOCUS_MIN);
	}

	// ファインダーが有効の時だけ写真を撮る
	if (m_photoCapture.IsFinderActive() && g_pad[0]->IsTrigger(enButtonB))
	{
		m_photoCapture.Shot();
		// この瞬間のピントの判定を記録
		m_isLastShotInFocus = isInFocus;
	}

	// ファインダー拡大
	if (m_photoCapture.IsFinderActive() && g_pad[0]->IsPress(enButtonRB1))
	{
		m_photoCapture.FinderZoom(ZOOM_IN_RATE);
	}
	// ファインダー縮小
	if (m_photoCapture.IsFinderActive() && g_pad[0]->IsPress(enButtonLB1))
	{
		m_photoCapture.FinderZoom(ZOOM_OUT_RATE);
	}

	// ファインダー用のカメラの参照を取得
	Camera& camera = m_photoCapture.GetFinderCamera();

	camera.SetPosition(g_camera3D->GetPosition());
	camera.SetTarget(g_camera3D->GetTarget());
	camera.Update();
}

void Game::Render(RenderContext& rc)
{
	// ファインダーを覗いていない時だけ描画する
	if (!m_photoCapture.IsFinderActive())
	{
		m_modelRender->Draw(rc);
	}

	// モデル描画(地面)
	m_groundModelRender->Draw(rc);

	// 敵の描画
	m_enemy->Render(rc);
}