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
			 * @brief 指定した方向へ、なめらかに向きを変える
			 * @param direction 向きたい方向(XZ平面)
			 */
			void RotateToward(const Vector3& direction);
			/**
			 * @brief 自分から、隠れている物への方向を取得
			 * @return 方向(XZ平面、長さ1)
			 * @details 隠れるステートで、向きを決めるのに使う
			 */
			Vector3 GetDirectionToHideObject() const;


		protected:
			/**
			 * @brief ステートの変更先を取得
			 * @return 変更先のステート
			 */
			core::IState* GetChangeState() override;


		private:
			/** 近くにある、隠れられる物の種類 */
			enum class EnHideType : uint8_t
			{
				None,	// 何もない
				Low,	// 小さい物(屈んで隠れる)
				High,	// 壁や高い物(壁を背にして隠れる)
			};

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
			 * @brief 近くに隠れられる物があるかを調べる
			 * @return 隠れられる物の種類
			 * @details 膝と頭の高さでレイを飛ばして、膝だけ当たれば小さい物、両方当たれば壁とする。
			 *          当たった位置は m_hideObjectPosition に入れる
			 */
			EnHideType CheckHideType();


		private:
			/** 主人公 */
			Boy* m_boy;
			/** 今の向き */
			Quaternion m_rotation;
			/** 隠れている物の位置(レイが当たった位置) */
			Vector3 m_hideObjectPosition;
		};
	}
}
