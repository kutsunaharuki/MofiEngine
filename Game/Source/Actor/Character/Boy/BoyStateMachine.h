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
			 * @brief 入力の方向に移動して、その方向を向く
			 * @param speed 移動速度
			 * @details 縦(y)の速度は変えない
			 */
			void Move(const float speed);
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
			 * @brief 重力をかける
			 * @details 地面に着いているときは、キャラクターコントローラーが縦の速度を 0 に戻す
			 */
			void ApplyGravity();
			/**
			 * @brief ジャンプの速度を与える
			 */
			void StartJump();
			/**
			 * @brief ジャンプが終わったことを伝える
			 * @details ジャンプステートが着地したときに呼ぶ
			 */
			void EndJump() { m_isJumping = false; }
			/**
			 * @brief カメラの向いている方向を向く
			 */
			void FaceCameraForward();


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
			 * @brief 指定した方向へ、なめらかに向きを変える
			 * @param direction 向きたい方向(XZ平面)
			 */
			void RotateToward(const Vector3& direction);


		private:
			/** 主人公 */
			Boy* m_boy;
			/** 今の向き */
			Quaternion m_rotation;
			/** ジャンプ中か(着地するまで true) */
			bool m_isJumping;
		};
	}
}
