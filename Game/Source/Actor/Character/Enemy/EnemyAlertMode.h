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
			/** 隠れ場所に着いてから見回している時間(秒) */
			float lookAroundTime;
			/** 道が見つからなかった時に次を試すまでの時間(秒) */
			float retryInterval;
			/** 探し回る時に、見失った所からこの距離の中の隠れ場所を探す */
			float searchRadius;
			/** 探し回る時に見に行く隠れ場所の数(近い順) */
			int searchPointCount;


			AlertModeSetting()
				: lookAroundTime(2.0f)
				, retryInterval(1.0f)
				, searchRadius(600.0f)
				, searchPointCount(3)
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
			 * @brief ある場所を調べに行く(シャッター音がした所・主人公を見失った所)
			 * @param position 調べに行く場所
			 * @details 歩く速さで向かい、着いたら少し見回して、警戒をやめる(いつもの動きに戻る)。
			 *          盗んでいる途中でもやめて見に行く
			 */
			void StartInvestigate(const Vector3& position);
			/**
			 * @brief ある場所のまわりを探し回る(同じ所で、また主人公を見失った時)
			 * @param position 見失った所
			 * @details 見失った所から searchRadius の中の隠れ場所を、近い順に searchPointCount 個まわって見回す。
			 *          全部まわったら、警戒をやめる。近くに隠れ場所が無ければ、見失った所を調べに行くだけにする
			 */
			void StartSearchAround(const Vector3& position);
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


		private:
			/** 警戒モードの段階 */
			enum class Phase : uint8_t
			{
				NoPoint,		// 隠れ場所が決まっていない
				MoveToPoint,	// 隠れ場所へ向かっている
				LookAround,		// 隠れ場所で見回している
				MoveToInvestigate,		// 調べに行く場所へ向かっている(音がした所・見失った所)
				LookAroundInvestigate,	// 調べに行った場所で見回している(終わったら警戒をやめる)
				MoveToSearch,			// 探し回っている途中で、次の隠れ場所へ向かっている
				LookAroundSearch		// 探し回っている途中で、隠れ場所で見回している(終わったら次へ)
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
			/**
			 * @brief 探し回っている時の、次の隠れ場所へ向かい始める(全部まわったら警戒をやめる)
			 */
			void StartMoveToSearchPoint();
			/**
			 * @brief 調べに行っている・探し回っている途中か
			 * @return 調べに行く場所・探す隠れ場所へ向かっている・見回しているなら true
			 */
			bool IsInvestigating() const
			{
				return m_phase == Phase::MoveToInvestigate || m_phase == Phase::LookAroundInvestigate
					|| m_phase == Phase::MoveToSearch || m_phase == Phase::LookAroundSearch;
			}


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
			/** 探し回る隠れ場所の位置の一覧(近い順。位置で持つので、階を移って隠れ場所の一覧が変わっても使える) */
			std::vector<Vector3> m_searchPositionList;
			/** 探し回っている時に、今向かっている隠れ場所の番号(m_searchPositionList の中) */
			int m_searchIndex;
			/** 乱数 */
			std::mt19937 m_random;
		};
	}
}

