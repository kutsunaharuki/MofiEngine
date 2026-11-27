/**
 * @file BoyStateMachine.h
 * @brief 主人公のステートマシン
 */
#pragma once
#include "Source/Actor/Character/CharacterStateMachine.h"
#include "Boy.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 主人公のステートマシン
		 * @details どのステートにするかは GetChangeState で決める(優先順位の高い順に判定する)。
		 *          各ステートが共通で使う処理(移動・重力・向き)も、ここにまとめている
		 */
		class BoyStateMachine : public CharacterStateMachine
		{
		public:
			explicit BoyStateMachine(Boy* owner);
			~BoyStateMachine() override = default;


		public:
			/**
			 * @brief 調整用パラメーターを取得
			 * @return パラメーター(JSON を保存すると、ゲームを動かしたまま変わる)
			 */
			const BoyParameter& GetParameter() { return m_boy->GetParameter(); }

			/**
			 * @brief アニメーションを再生する
			 * @param animation アニメーション
			 * @param interpolateTime 前のアニメーションからの補間時間(秒)
			 */
			void PlayAnimation(const EnBoyAnimation animation, const float interpolateTime = 0.2f)
			{
				CharacterStateMachine::PlayAnimation(static_cast<int>(animation), interpolateTime);
			}

			/**
			 * @brief 移動の入力があるか
			 * @return 入力があれば true
			 */
			bool HasMoveInput() const;
			/**
			 * @brief 入力の方向に移動する
			 * @param speed 移動速度
			 * @param isRotate 移動する方向を向くか(撮影中はカメラの方を向くので false にする)
			 * @details 縦(y)の速度は変えない
			 */
			void Move(const float speed, const bool isRotate = true);
			/**
			 * @brief 今の横(XZ平面)の速さを取得
			 * @return 横の速さ
			 */
			float GetHorizontalSpeed() const;
			/**
			 * @brief 横の移動を止める
			 * @details 縦(y)の速度は変えない(重力は続く)
			 */
			void StopMove();
			/**
			 * @brief カメラの向いている方向を向く
			 */
			void FaceCameraForward();
			/**
			 * @brief ファインダーを覗いている時の移動
			 * @details 覗いていない時より遅く歩いて、向きはカメラの向いている方向のままにする
			 */
			void SlowMove();
			/**
			 * @brief ファインダーを覗いている時の移動(速さを決めて動く)
			 * @param maxSpeed スティックを全部倒した時の速さ
			 * @details しゃがんで構えたまま歩く時に使う
			 */
			void SlowMove(const float maxSpeed);
			/**
			 * @brief 指定した方向へ、なめらかに向きを変える
			 * @param direction 向きたい方向(XZ平面)
			 */
			void RotateToward(const Vector3& direction);
			/**
			 * @brief しゃがんでいるか
			 * @return しゃがんでいれば true(R3 で切りかわる)
			 */
			bool IsCrouching() const { return m_isCrouching; }


		protected:
			/**
			 * @brief ステートの変更先を取得
			 * @return 変更先のステート
			 */
			core::IState* GetChangeState() override;


		private:
			/**
			 * @brief スティックの入力を、カメラから見た移動方向(XZ平面)にする
			 * @return 移動方向(長さ1。入力が無ければ 0)
			 */
			Vector3 CalcMoveDirection() const;
			/**
			 * @brief スティックの向きから、撮影中の歩きステート(前後左右)を選ぶ
			 * @return 歩きステート
			 */
			core::IState* FindPhotoWalkState();
			/**
			 * @brief 走るかどうかを切り替える(L スティックの押し込み)
			 * @param isFinderActive ファインダーを覗いているか
			 */
			void UpdateRunToggle(const bool isFinderActive);
			/**
			 * @brief しゃがむかどうかを切り替える(R スティックの押し込み。1 回押したら、もう 1 回押すまでしゃがんだまま)
			 * @details ファインダーを覗いていても、歩いても、しゃがんだまま(立つのは R3 か、L3 で走った時だけ)
			 */
			void UpdateCrouchToggle();
			/**
			 * @brief スティックをどれだけ倒したか
			 * @return 0(倒していない)〜 1(全部倒した)
			 */
			float CalcMoveInputStrength() const;


		private:
			/** 主人公 */
			Boy* m_boy;
			/** 今の向き */
			Quaternion m_rotation;
			/** 走っているか */
			bool m_isRunning;
			/** しゃがんでいるか */
			bool m_isCrouching;
		};
	}
}
