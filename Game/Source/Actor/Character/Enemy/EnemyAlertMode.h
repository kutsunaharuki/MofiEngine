/**
 * @file EnemyAlertMode.h
 * @brief 敵の警戒モード(隠れ場所を見て回る頭)
 * @details 行き先を決めて、EnemyStateMachine::MoveTo で移動する。
 *          (目的モードと同じ)
 */
#pragma once
#include <cstdint>
#include <random>
#include <vector>
#include "Source/Stage/HidePoint.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class EnemyStateMachine;


		/**
		 * @brief 警戒モードの設定
		 */
		struct AlertModeSetting
		{
			/** 警戒が続く時間 */
			float alertDuration;
			/** 隠れ場所に着いてから見回している時間(秒) */
			float lookAroundTime;
			/** 道が見つからなかった時に次を試すまでの時間(秒) */
			float retryInterval;


			AlertModeSetting()
				: alertDuration(20.0f)
				, lookAroundTime(2.0f)
				, retryInterval(1.0f)
			{}
		};




		/********************************************/


		/**
		 * @brief 警戒モード
		 */
		class EnemyAlertMode
		{
		public:
			/**
			 * @brief コンストラクタ
			 * @param stateMachine 敵のステートマシン
			 */
			explicit EnemyAlertMode(EnemyStateMachine* stateMachine);
			~EnemyAlertMode();


			/**
			 * @brief 更新処理
			 * @param deltaTime 経過時間(秒)
			 */
			void Update(const float deltaTime);


		public:
			/**
			 * @brief 警戒を開始
			 */
			void StartAlert();
			/**
			 * @brief 警戒を終了
			 */
			void EndAlert();
			/**
			 * @brief 隠れ場所の一覧を設定
			 * @param hidePointList 隠れ場所の一覧(今いる階の分)
			 */
			void SetHidePointList(const std::vector<stage::HidePoint>& hidePointList);
			/**
			 * @brief 設定を警戒モードに変える
			 * @param setting 警戒モードの設定
			 */
			void SetSetting(const AlertModeSetting& setting) { m_setting = setting; }

			/**
			 * @brief 今の段階の名前を取得(デバッグで表示する)
			 * @return 今の段階の文字列を取得
			 */
			const char* GetPhaseName() const;
			/**
			 * @brief 今向かっている隠れ場所の番号を取得(デバッグで表示する)
			 * @return 今向かっている隠れ場所
			 */
			int GetPointIndex() const { return m_pointIndex; }
			/**
			 * @brief 警戒が終わるまでの残り時間を取得(デバッグで表示する)
			 * @return 警戒が終わるまでの残り時間
			 */
			float GetRemainTime() const;


		private:
			/** 警戒モードの段階 */
			enum class Phase : uint8_t
			{
				NoPoint,		// 隠れ場所が決まっていない
				MoveToPoint,	// 隠れ場所へ向かっている
				LookAround		// 隠れ場所で見回している
			};


		private:
			/**
			 * @brief 段階を変える(Phase の経過時間を 0 に戻す)
			 * @param phase 段階
			 */
			void ChangePhase(const Phase phase);
			/**
			 * @brief 次に見に行く隠れ場所を乱数で選ぶ
			 */
			void ChooseNextPoint();
			/**
			 * @brief 今の隠れ場所へ向かい始める
			 */
			void StartMoveToPoint();


		private:
			/** 命令をだすステートマシン */
			EnemyStateMachine* m_stateMachine;
			/** 警告モードの設定 */
			AlertModeSetting m_setting;
			/** 隠れ場所の一覧 */
			std::vector<stage::HidePoint> m_hidePointList;
			/** 重みの一覧 */
			std::vector<float> m_weightList;
			/** 今向かっている隠れ場所の番号 */
			int m_pointIndex;
			/** 今の段階 */
			Phase m_phase;
			/** 今の段階になってからの経過時間(秒) */
			float m_phaseTimer;
			/** 警戒してからの経過時間(秒) */
			float m_alertTimer;
			/** 乱数 */
			std::mt19937 m_random;
		};
	}
}

