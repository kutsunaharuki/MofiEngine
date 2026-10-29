/**
 * @file EnemyStateMachine.h
 * @brief 敵のステートマシン
 */
#pragma once
#include "Source/Actor/Character/CharacterStateMachine.h"
#include "Source/Actor/Character/Enemy/EnemyBase.h"


namespace app
{
	namespace ai
	{
		/** 前方宣言 */
		class Pathfinder;
		class NavGrid;
		class PathFollower;
		
		// うろつきの設定
		struct WanderSetting;
	}


	namespace actor
	{
		/**
		 * @brief 敵のステートマシン
		 */
		class EnemyStateMachine : public CharacterStateMachine
		{
		public:
			explicit EnemyStateMachine(EnemyBase* enemyOwner);
			~EnemyStateMachine() override;


		public:
			/**
			 * @brief アニメーションを再生
			 * @param animation 再生するアニメーション
			 * @param interpolateTime 補間時間(/s(秒))
			 */
			void PlayAnimation(EnemyBase::EnemyAnimation animation, const float interpolateTime = 0.2f)
			{
				CharacterStateMachine::PlayAnimation(static_cast<int>(animation), interpolateTime);
			}

			/**
			 * @brief 移動方向を設定
			 * @param direction 移動方向
			 */
			inline void SetMoveDirection(const Vector3& direction) { m_moveDirection = direction; }
			/**
			 * @brief 警戒状態かどうかを設定
			 * @param isAlert 警戒状態かどうか
			 */
			inline void SetAlert(bool isAlert) { m_isAlert = isAlert; }
			/**
			 * @brief 移動量を設定
			 * @param amount 移動量
			 */
			inline void SetMoveAmount(const float amount) { m_moveAmount = amount; }

			/**
			 * @brief 移動制御
			 */
			void Move(const float speed);
			/**
			 * @brief 横の移動を止める
			 */
			void StopMove();
			/**
			 * @brief 指定した方向に回転する
			 * @param direction 回転する方向
			 */
			void RotateToward(const Vector3& direction);


		public:
			/**
			 * @brief 経路探索に使うものを設定
			 * @param pathfinder 道順を求めるもの
			 * @param navGrid 地図(ふらつきで壁を調べるのに使用。nullptrなら調べない)
			 */
			void SetNavigation(const ai::Pathfinder* pathfinder, const ai::NavGrid* navGrid);
			/**
			 * @brief 指定した位置まで向かう
			 * @param goal 行き先
			 * @return 道順が見つかれば true
			 * @details A* を使うので、行き先が変わる時だけ呼ぶ。(毎フレーム呼ばない)
			 */
			bool MoveTo(const Vector3& goal);
			/**
			 * @brief 道順を辿るのをやめる
			 * @details やめた後は、外から SetMoveDirection / SetMoveAmount で動かす必要がある
			 */
			void StopPathFollow();
			/**
			 * @brief 道順を辿って移動方向と移動量を決める
			 * @details 毎フレーム、ステートを呼ぶより先に呼ぶ必要がある
			 *          先に移動量を決めないと待機から歩行に移行しない
			 */
			void UpdatePathFollow();
			/**
			 * @brief うろつきの設定
			 * @param setting うろつきの設定(目的地モードでは大きめ、警戒モードは 0)
			 */
			void SetWanderSetting(const ai::WanderSetting& setting);


		public:
			/**
			 * @brief 敵の調整パラメーターを取得
			 * @return 調整パラメーター(種類ごとの JSON の値)
			 */
			const EnemyParameter& GetParameter() { return m_enemyOwner->GetParameter(); }

			/**
			 * @brief 道順を辿っているか
			 * @return 辿っているなら true
			 */
			bool IsFollowingPath() const;
			/**
			 * @brief 行き先に着いたか
			 * @return 着いたら true
			 */
			bool IsArrived() const;
			/**
			 * @brief 道順を辿る係を取得(デバッグ表示用)
			 * @return 道順を辿る係
			 */
			const ai::PathFollower& GetPathFollower() { return *m_pathFollower; }


		protected:
			/**
			 * @biref ステートの変更先を取得
			 * @return 変更先のステート
			 */
			core::IState* GetChangeState() override;


		private:
			/** 敵のオーナー */
			EnemyBase* m_enemyOwner;
			/** 移動量 */
			float m_moveAmount;
			/** 移動方向 */
			Vector3 m_moveDirection;
			/** 警戒状態かどうか */
			bool m_isAlert;
			/** 回転 */
			Quaternion m_rotation;


			/** 道順を求める */
			const ai::Pathfinder* m_pathfinder;
			/** 道順を辿る */
			std::unique_ptr<ai::PathFollower> m_pathFollower;
			/** 今の行き先(詰まった時に求め直す為) */
			Vector3 m_goal;
		};
	}
}

