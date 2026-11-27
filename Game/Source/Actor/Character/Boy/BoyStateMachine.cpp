/**
 * @file BoyStateMachine.cpp
 * @brief 主人公のステートマシン
 */
#include "stdafx.h"
#include "BoyStateMachine.h"
#include "BoyState.h"
#include "Source/Camera/GameCamera.h"
#include <cmath>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** これより小さいスティックの入力は、入力なしとみなす */
			constexpr float MOVE_INPUT_THRESHOLD = 0.01f;
			/** 撮影歩きの向きを切り替えるのは、もう一方の軸がこの倍率より大きくなった時 */
			constexpr float PHOTO_WALK_SWITCH_RATIO = 1.3f;
		}


		BoyStateMachine::BoyStateMachine(Boy* owner)
			: CharacterStateMachine(owner)
			, m_boy(owner)
			, m_rotation(Quaternion::Identity)
			, m_isRunning(false)
			, m_isCrouching(false)
		{
			// ステートの追加
			AddState<BoyIdleState>(*this);
			AddState<BoyWalkState>(*this);
			AddState<BoyRunState>(*this);
			AddState<BoyPhotoIdleState>(*this);
			AddState<BoyPhotoWalkForwardState>(*this);
			AddState<BoyPhotoWalkBackState>(*this);
			AddState<BoyPhotoWalkLeftState>(*this);
			AddState<BoyPhotoWalkRightState>(*this);
			AddState<BoyCrouchIdleState>(*this);
			AddState<BoyCrouchWalkState>(*this);
			AddState<BoyCrouchCameraState>(*this);
			AddState<BoyCrouchPhotoWalkState>(*this);
			AddState<BoyLeanLeftState>(*this);
			AddState<BoyLeanRightState>(*this);
		}


		core::IState* BoyStateMachine::GetChangeState()
		{
			// ファインダー状態かを見る
			const bool isFinderActive = m_boy->IsFinderActive();
			// ステートを選ぶ前に、走るか・しゃがむかを決める
			UpdateRunToggle(isFinderActive);
			UpdateCrouchToggle();

			// 移動入力があれば歩く
			if (HasMoveInput())
			{
				if (isFinderActive)
				{
					// しゃがんだまま構えて歩くなら、しゃがんだまま歩く(立たない)
					if (m_isCrouching) return FindState(BoyCrouchPhotoWalkState::ID());
					// ファインダーをのぞきながら歩くなら、スティックの向きの撮影歩行ステート
					return FindPhotoWalkState();
				}
				// しゃがんでいれば、しゃがみ歩き
				if (m_isCrouching) return FindState(BoyCrouchWalkState::ID());
				// 移動入力 + 走れるなら 走るステート
				if (m_isRunning) return FindState(BoyRunState::ID());
				return FindState(BoyWalkState::ID());
			}

			// ファインダーを覗いて止まっている時
			if (isFinderActive)
			{
				// 柱の近くなら、柱からのぞく(しゃがんでいれば、しゃがんだままのぞく)
				const EnLeanSide leanSide = m_boy->GetLeanSide();
				if (leanSide == EnLeanSide::Left) return FindState(BoyLeanLeftState::ID());
				if (leanSide == EnLeanSide::Right) return FindState(BoyLeanRightState::ID());
				// しゃがんでいれば、しゃがんで構える
				if (m_isCrouching) return FindState(BoyCrouchCameraState::ID());
				return FindState(BoyPhotoIdleState::ID());
			}

			// しゃがんでいれば、しゃがんだまま止まる
			if (m_isCrouching) return FindState(BoyCrouchIdleState::ID());

			// 何もしていなければ待機
			return FindState(BoyIdleState::ID());
		}


		bool BoyStateMachine::HasMoveInput() const
		{
			return CalcMoveDirection().LengthSq() > 0.0f;
		}


		void BoyStateMachine::Move(const float speed, const bool isRotate)
		{
			const Vector3 direction = CalcMoveDirection();

			Vector3 moveSpeed = m_boy->GetMoveSpeed();
			moveSpeed.x = direction.x * speed;
			moveSpeed.z = direction.z * speed;
			m_boy->SetMoveSpeed(moveSpeed);

			if (isRotate && direction.LengthSq() > 0.0f)
			{
				RotateToward(direction);
			}
		}


		float BoyStateMachine::GetHorizontalSpeed() const
		{
			const Vector3& moveSpeed = m_boy->GetMoveSpeed();
			return std::sqrt(moveSpeed.x * moveSpeed.x + moveSpeed.z * moveSpeed.z);
		}


		void BoyStateMachine::StopMove()
		{
			Vector3 moveSpeed = m_boy->GetMoveSpeed();
			moveSpeed.x = 0.0f;
			moveSpeed.z = 0.0f;
			m_boy->SetMoveSpeed(moveSpeed);
		}


		void BoyStateMachine::FaceCameraForward()
		{
			const camera::GameCamera* gameCamera = m_boy->GetGameCamera();
			if (gameCamera == nullptr) return;

			RotateToward(gameCamera->GetForwardXZ());
		}


		void BoyStateMachine::SlowMove()
		{
			SlowMove(GetParameter().photoWalkSpeed);
		}


		void BoyStateMachine::SlowMove(const float maxSpeed)
		{
			// 撮影中は横や後ろにも歩くので、移動する方向は向かずにカメラの方を向く
			// 少し倒せばゆっくり、全部倒せば maxSpeed(狙いながら少しだけ動けるように)
			Move(maxSpeed * CalcMoveInputStrength(), false);
			FaceCameraForward();
		}


		core::IState* BoyStateMachine::FindPhotoWalkState()
		{
			const float stickX = g_pad[0]->GetLStickXF();
			const float stickY = g_pad[0]->GetLStickYF();

			const float absX = std::fabs(stickX);
			const float absY = std::fabs(stickY);

			// 今、縦(前後)と横(左右)のどちらで歩いているか
			const bool isWalkingVertical = IsEqualState(BoyPhotoWalkForwardState::ID()) || IsEqualState(BoyPhotoWalkBackState::ID());
			const bool isWalkingHorizontal = IsEqualState(BoyPhotoWalkLeftState::ID()) || IsEqualState(BoyPhotoWalkRightState::ID());

			// 縦と横で、大きく倒している方を優先する
			// ただし今の向きは、もう一方がはっきり大きくなるまで続ける(斜めでガタガタ入れ替わらないように)
			bool isVertical = absY >= absX;
			if (isWalkingVertical) isVertical = absX < absY * PHOTO_WALK_SWITCH_RATIO;
			if (isWalkingHorizontal) isVertical = absY >= absX * PHOTO_WALK_SWITCH_RATIO;

			if (isVertical)
			{
				if (stickY >= 0.0f) return FindState(BoyPhotoWalkForwardState::ID());
				return FindState(BoyPhotoWalkBackState::ID());
			}

			if (stickX >= 0.0f) return FindState(BoyPhotoWalkRightState::ID());
			return FindState(BoyPhotoWalkLeftState::ID());
		}


		void BoyStateMachine::UpdateRunToggle(const bool isFinderActive)
		{
			// 止まった時・ファインダーをのぞいた時は、走るのをやめる
			if (!HasMoveInput() || isFinderActive)
			{
				m_isRunning = false;
				return;
			}

			// 押し込むたびに 走ると歩くを切り替える
			if (g_pad[0]->IsTrigger(enButtonLB3))
			{
				m_isRunning = !m_isRunning;
				// しゃがんでいる時に走り出したら、立つ
				if (m_isRunning) m_isCrouching = false;
			}
		}


		void BoyStateMachine::UpdateCrouchToggle()
		{
			// 押し込むたびに しゃがむと立つを切り替える(1 回押したら、もう 1 回押すまでしゃがんだまま)
			if (g_pad[0]->IsTrigger(enButtonRB3))
			{
				m_isCrouching = !m_isCrouching;
				// しゃがんだら、走るのはやめる
				if (m_isCrouching) m_isRunning = false;
			}
		}


		float BoyStateMachine::CalcMoveInputStrength() const
		{
			const float stickX = g_pad[0]->GetLStickXF();
			const float stickY = g_pad[0]->GetLStickYF();

			// 斜めに倒すと 1 を少し超えることがあるので、1 までにする
			const float strength = std::sqrt(stickX * stickX + stickY * stickY);
			return TClamp<float>::Clamp(strength, 0.0f, 1.0f);
		}


		Vector3 BoyStateMachine::CalcMoveDirection() const
		{
			const float stickX = g_pad[0]->GetLStickXF();
			const float stickY = g_pad[0]->GetLStickYF();
			if (stickX * stickX + stickY * stickY < MOVE_INPUT_THRESHOLD * MOVE_INPUT_THRESHOLD)
			{
				return Vector3::Zero;
			}

			// カメラが無ければ、ワールドの軸を使う
			Vector3 forward(0.0f, 0.0f, 1.0f);
			Vector3 right(1.0f, 0.0f, 0.0f);
			if (const camera::GameCamera* gameCamera = m_boy->GetGameCamera())
			{
				forward = gameCamera->GetForwardXZ();
				right = gameCamera->GetRightXZ();
			}

			// スティックの上 = カメラの前、右 = カメラの右
			forward.Scale(stickY);
			right.Scale(stickX);
			Vector3 direction = forward;
			direction.Add(right);
			direction.y = 0.0f;

			// 入力がないときは向きは変えない
			if (direction.LengthSq() <= 0.0f) return Vector3::Zero;
			
			direction.Normalize();
			return direction;
		}


		void BoyStateMachine::RotateToward(const Vector3& direction)
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			Quaternion target;
			target.SetRotationYFromDirectionXZ(direction);

			// フレームレートが変わっても、同じ速さで向くようにする
			const float t = TClamp<float>::Clamp(GetParameter().rotateSpeed * deltaTime, 0.0f, 1.0f);
			m_rotation.Slerp(t, m_rotation, target);
			SetRotation(m_rotation);
		}
	}
}
