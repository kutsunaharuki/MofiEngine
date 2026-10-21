/**
 * @file FinderController.cpp
 * @brief ファインダー(写真撮影)の操作
 */
#include "stdafx.h"
#include "FinderController.h"
#include "Source/UI/FinderMenu/FinderMenu.h"
#include <algorithm>
#include <cmath>


namespace app
{
	namespace camera
	{
		namespace
		{
			/** 調整用パラメーターの JSON */
			constexpr const char* FINDER_PARAMETER_PATH = "Assets/parameter/Camera/Finder.json";
			/** ファインダーの UI のレイアウトの JSON */
			constexpr const char* FINDER_LAYOUT_PATH = "Assets/parameter/UI/Finder/Finder.json";


			/**
			 * @brief 3D の位置を、画面の位置に変える
			 * @param camera カメラ
			 * @param worldPosition 3D の位置
			 * @param out 画面の位置(画面の真ん中が原点、上が +Y。UI と同じ座標)
			 * @return カメラの前にあれば true(後ろにある物は、画面に写らない)
			 * @details Camera::CalcScreenPositionFromWorldPosition と同じ計算に、カメラの後ろかの判定を足したもの
			 */
			bool CalcScreenPosition(Camera& camera, const Vector3& worldPosition, Vector2& out)
			{
				Vector4 position(worldPosition.x, worldPosition.y, worldPosition.z, 1.0f);
				camera.GetViewProjectionMatrix().Apply(position);

				// w がマイナス(または 0)なら、カメラの後ろ
				if (position.w <= 0.0001f) return false;

				// -1 ~ 1 の範囲を、画面のピクセルにする(ファインダーの画面は画面全体に出ている)
				out.x = (position.x / position.w) * UI_SPACE_WIDTH * 0.5f;
				out.y = (position.y / position.w) * UI_SPACE_HEIGHT * 0.5f;
				return true;
			}
		}


		FinderController::FinderController()
			: m_photoCapture(nullptr)
			, m_finderMenu(nullptr)
			, m_focusSubject(nullptr)
			, m_isSubjectInRange(false)
			, m_coolTimer(0.0f)
		{}


		FinderController::~FinderController()
		{}


		void FinderController::Initialize(PhotoCapture* photoCapture)
		{
			m_photoCapture = photoCapture;
			m_parameter.Initialize(FINDER_PARAMETER_PATH);
			m_coolTimer = 0.0f;

			// ファインダーの UI を作る(最初は閉じている)
			m_finderLayout.Initialize<ui::FinderMenu>(FINDER_LAYOUT_PATH);
			m_finderMenu = m_finderLayout.GetMenu<ui::FinderMenu>();
		}


		bool FinderController::Update()
		{
			if (m_photoCapture == nullptr) return false;

			// クールタイムは、覗いていなくても進める
			m_coolTimer -= g_gameTime->GetFrameDeltaTime();
			if (m_coolTimer < 0.0f) m_coolTimer = 0.0f;

			// 覗く / やめる
			if (g_pad[0]->IsTrigger(enButtonY))
			{
				m_photoCapture->ToggleFinderActive();
			}
			if (!m_photoCapture->IsFinderActive())
			{
				// 覗いていないときは、ピントを合わせている被写体はいない
				m_focusSubject = nullptr;
				m_isSubjectInRange = false;
				return false;
			}

			// 調整パラメーターを取得
			const FinderParameter& parameter = m_parameter.Get();

			// ※ オートフォーカスは、ファインダーのカメラを合わせた後の LateUpdate で行う

			// ズーム(RB1 で寄る、LB1 で引く)
			if (g_pad[0]->IsPress(enButtonRB1))
			{
				m_photoCapture->FinderZoom(parameter.zoomSpeed);
			}
			if (g_pad[0]->IsPress(enButtonLB1))
			{
				m_photoCapture->FinderZoom(-parameter.zoomSpeed);
			}

			// 撮影(クールタイム中は撮れない)
			if (g_pad[0]->IsTrigger(enButtonB) && CanShot())
			{
				m_photoCapture->Shot();
				m_coolTimer = parameter.shotCoolTime;
				return true;
			}
			return false;
		}


		void FinderController::LateUpdate()
		{
			if (m_photoCapture == nullptr) return;

			// 構えている間だけ、ファインダーの UI を出す
			SyncMenuOpen();

			if (m_photoCapture->IsFinderActive())
			{
				// ピントを合わせる被写体を選んで、丸を出す
				AutoFocus();

				// つまみに、今のピントとズームの量を渡す
				if (m_finderMenu)
				{
					m_finderMenu->SetFocusRate(m_photoCapture->GetFocusDistanceRate());
					m_finderMenu->SetZoomRate(m_photoCapture->GetFinderZoomRate());
				}
			}

			// UI の更新(閉じている時は、メニューの中で何もしない。JSON のホットリロードもここで確認される)
			m_finderLayout.Update();
		}


		void FinderController::Render(RenderContext& rc)
		{
			// 閉じている時は、メニューの中で何も描画されない
			m_finderLayout.Render(rc);
		}


		void FinderController::SyncMenuOpen()
		{
			if (m_finderMenu == nullptr) return;

			const bool isFinderActive = m_photoCapture->IsFinderActive();
			if (isFinderActive && !m_finderMenu->IsOpen())
			{
				m_finderMenu->Open();
			}
			else if (!isFinderActive && m_finderMenu->IsOpen())
			{
				m_finderMenu->Close();
			}
		}


		void FinderController::RegisterSubject(const IPhotoSubject* subject)
		{
			// 被写体が nullptr なら登録しない
			if (subject == nullptr) return;

			// 同じ被写体は2回登録しない
			// 2回入っていると、被写体を解除した時に1つしか消えず、消えた敵のポインタが残ってしまうため
			auto it = std::find(m_subjectList.begin(), m_subjectList.end(), subject);

			// 2回目の登録なら登録しない
			if (it != m_subjectList.end()) return;

			m_subjectList.push_back(subject);
		}
		
		
		void FinderController::UnregisterSubject(const IPhotoSubject * subject)
		{
			// 解除する被写体がリストにあるか探す
			auto it = std::find(m_subjectList.begin(), m_subjectList.end(), subject);

			// 見つかったらリストから削除する
			if (it != m_subjectList.end())
			{
				m_subjectList.erase(it);
			}
		}


		bool FinderController::FindFocusSubject(SubjectOnScreen& out)
		{
			// 調整用パラメーターを取得
			const FinderParameter& parameter = m_parameter.Get();

			// 写真を撮る媒体からファインダーのカメラを取得
			Camera& camera = m_photoCapture->GetFinderCamera();
			// ファインダーのカメラから位置を取得
			const Vector3 eye = camera.GetPosition();

			// 真ん中のグリッドの大きさの半分(この中に顔がある被写体だけが候補)
			const Vector2 gridHalfSize = m_finderMenu ? m_finderMenu->GetGridHalfSize() : Vector2(0.0f, 0.0f);

			bool isFound = false;

			// 被写体のリストを全部見て計算する
			for (const IPhotoSubject* subject : m_subjectList)
			{
				// 被写体の顔の位置を取得
				const Vector3 facePosition = subject->GetFocusPosition();

				// 被写体までの距離
				const float distance = (facePosition - eye).Length();
				// 遠すぎる被写体と、カメラと同じ位置にいる被写体は選ばない
				if (distance > parameter.autoFocusMaxDistance || distance < 0.001f) continue;

				// 顔を画面の位置に変える(カメラの後ろにいる被写体は選ばない)
				Vector2 screenPosition;
				if (!CalcScreenPosition(camera, facePosition, screenPosition)) continue;

				// 顔が真ん中のグリッドの外なら選ばない
				if (std::fabs(screenPosition.x) > gridHalfSize.x || std::fabs(screenPosition.y) > gridHalfSize.y) continue;

				// 画面の真ん中(十字)から顔までの距離(ピクセル)
				const float screenDistance = std::sqrt(screenPosition.x * screenPosition.x + screenPosition.y * screenPosition.y);

				// 今までで一番真ん中に近ければ覚えておく
				if (isFound && screenDistance >= out.screenDistance) continue;

				// 顔のまわりの丸の半径 = 顔の半径(3D)を画面に写した大きさ
				// ※ 顔の位置と、そこから顔の半径だけ上の位置を画面に写して、その差を半径にする。遠い・ズームで自動で変わる
				float circleRadius = parameter.subjectCircleMinRadius;
				Vector2 faceTopScreenPosition;
				const Vector3 faceTop(facePosition.x, facePosition.y + parameter.subjectFaceRadius, facePosition.z);
				if (CalcScreenPosition(camera, faceTop, faceTopScreenPosition))
				{
					const float dx = faceTopScreenPosition.x - screenPosition.x;
					const float dy = faceTopScreenPosition.y - screenPosition.y;
					circleRadius = std::sqrt(dx * dx + dy * dy);
				}
				circleRadius = TClamp<float>::Clamp(circleRadius, parameter.subjectCircleMinRadius, parameter.subjectCircleMaxRadius);

				out.subject = subject;
				out.screenPosition = screenPosition;
				out.circleRadius = circleRadius;
				out.distance = distance;
				out.screenDistance = screenDistance;
				isFound = true;
			}
			return isFound;
		}


		void FinderController::AutoFocus()
		{
			// 現在のピント距離を取得
			const float current = m_photoCapture->GetFocusDistance();
			// 目標の距離
			float target = current;
			// パラメーターを取得
			const FinderParameter& parameter = m_parameter.Get();
			// 1フレームの経過時間を取得
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			// 顔が真ん中のグリッドの中にある、一番真ん中に近い被写体を探す
			SubjectOnScreen focus;
			const bool isFound = FindFocusSubject(focus);

			if (isFound)
			{
				// ピントの目標は、その被写体の顔までの距離(エンジンの範囲の外には出さない)
				target = TClamp<float>::Clamp(focus.distance, m_photoCapture->GetFocusDistanceMin(), m_photoCapture->GetFocusDistanceMax());

				// 真ん中の十字が、顔のまわりの丸の中に入っていれば「範囲内」
				m_isSubjectInRange = focus.screenDistance <= focus.circleRadius;

				// 顔のまわりに丸を出す(範囲内なら細い丸、範囲外なら太い丸)
				if (m_finderMenu) m_finderMenu->ShowSubjectCircle(focus.screenPosition, focus.circleRadius, m_isSubjectInRange);
			}
			else
			{
				// 候補の被写体がいなければ、丸を出さない
				m_isSubjectInRange = false;
				if (m_finderMenu) m_finderMenu->HideSubjectCircle();

				// 被写体を見失ったら、すぐにピンボケにする(少しずつではなく、その瞬間に外す)
				// ※ 今のままにすると、一度合ったピントが、構えている間ずっと残ってしまう
				// ※ 外す先は、覗いた瞬間と同じ「一番近い距離」(PhotoCapture::ToggleFinderActive と同じ)
				//    次に被写体が見つかったら、そこから少しずつ合っていく
				m_focusSubject = nullptr;
				m_photoCapture->SetFocusDistance(m_photoCapture->GetFocusDistanceMin());
				// ※ この後の「少しずつ近づける」処理を通ると補間されて、すぐに外れなくなるので、ここで終わる
				return;
			}

			// 選んだ被写体を覚えておく(InGameSceneが撮影の判定に使う)
			m_focusSubject = focus.subject;

			// 今のピントを目標に少しずつ近づける
			// ※ マイナスを付けないと t がマイナスになり、ピントが目標から離れていく
			const float t = 1.0f - std::exp(-parameter.autoFocusSpeed * deltaTime);
			// 今のピントと目標のピントを線形補間して、ピントを設定する
			m_photoCapture->SetFocusDistance(current + (target - current) * t);
		}
	}
}
