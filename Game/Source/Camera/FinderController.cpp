/**
 * @file FinderController.cpp
 * @brief ファインダー(写真撮影)の操作
 */
#include "stdafx.h"
#include "FinderController.h"
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
			/** 円周率 */
			constexpr float PI = 3.1415926f;
		}


		FinderController::FinderController()
			: m_photoCapture(nullptr)
			, m_focusSubject(nullptr)
			, m_coolTimer(0.0f)
		{}


		FinderController::~FinderController()
		{}


		void FinderController::Initialize(PhotoCapture* photoCapture)
		{
			m_photoCapture = photoCapture;
			m_parameter.Initialize(FINDER_PARAMETER_PATH);
			m_coolTimer = 0.0f;
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
				return false;
			}

			// 調整パラメーターを取得
			const FinderParameter& parameter = m_parameter.Get();

			// オートフォーカス
			AutoFocus();

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


		const IPhotoSubject* FinderController::FinderFocusSubject()
		{
			// 調整用パラメーターを取得
			const  FinderParameter& parameter = m_parameter.Get();
			// 被写体を選ぶための変数を初期化
			const IPhotoSubject* bestSubject = nullptr;

			// 写真を撮る媒体からファインダーのカメラを取得
			const Camera& camera = m_photoCapture->GetFinderCamera();
			// ファインダーのカメラから位置を取得
			const Vector3 eye = camera.GetPosition();
			// ファインダーのカメラから注視点と位置を引いたベクトルを計算
			Vector3 forward = camera.GetTarget() - eye;
			// 計算結果を正規化して長さを1にする
			forward.Normalize();

			// 真ん中をみなす角度をcosで計算する(度 → ラジアン → cos)
			const float cosLimit = std::cos(parameter.autoFocusAngle * PI / 180.0f);

			// 計算した値を保持する
			float bestCos = cosLimit;

			// 被写体のリストを全部見て計算する
			auto it = m_subjectList.begin();
			for (auto end = m_subjectList.end(); it != end; ++it)
			{
				// 被写体のポインタを取得
				const IPhotoSubject* subject = *it;
				// 被写体の位置を取得
				const Vector3 subjectPos = subject->GetFocusPosition();
				// ファインダーのカメラから被写体までのベクトルを計算
				Vector3 toSubject = subjectPos - eye;

				// 被写体までの距離(正規化する前に測る)
				const float distance = toSubject.Length();
				// 遠すぎる被写体と、カメラと同じ位置にいる被写体は選ばない
				// ※ 長さ 0 のベクトルを正規化すると、計算が壊れる(NaN になる)
				if (distance > parameter.autoFocusMaxDistance || distance < 0.001f) continue;

				// 計算結果を正規化して長さを1にする
				toSubject.Normalize();
				// ファインダーの中心に近いかどうかを計算する
				const float cosAngle = forward.Dot(toSubject);
				
				// 今までで一番中心に近ければ覚えておく
				if (cosAngle > bestCos)
				{
					bestCos = cosAngle;
					bestSubject = subject;
				}
			}
			return bestSubject;
		}


		void FinderController::AutoFocus()
		{
			// ファインダーのカメラの位置を取得
			const Vector3 eyePos = m_photoCapture->GetFinderCamera().GetPosition();
			// 現在のピント距離を取得
			const float current = m_photoCapture->GetFocusDistance();
			// 目標の距離
			float target = current;
			// パラメーターを取得
			const FinderParameter& parameter = m_parameter.Get();
			// 1フレームの経過時間を取得
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			// ファインダーの中心に近い被写体を探す
			const IPhotoSubject* subject = FinderFocusSubject();


			// 被写体が見つかったら
			if (subject != nullptr)
			{
				Vector3 focusPosition = subject->GetFocusPosition() - eyePos;
				target = focusPosition.Length();
			}

			// 選んだ被写体を覚えておく(InGameSceneが撮影の判定に使う)
			m_focusSubject = subject;

			// 今のピントを目標に少しずつ近づける
			// ※ マイナスを付けないと t がマイナスになり、ピントが目標から離れていく
			const float t = 1.0f - std::exp(-parameter.autoFocusSpeed * deltaTime);
			// 今のピントと目標のピントを線形補間して、ピントを設定する
			m_photoCapture->SetFocusDistance(current + (target - current) * t);
		}
	}
}
