/**
 * @file EnemyDestinationMode.h
 * @brief 敵の目的地モード(美術品を盗みに行く頭)
 * @details 行き先を決めて、EnemyStateMachine::MoveTo に渡すだけ。
 *          道順を求める(Pathfinder)・たどる(PathFollower)・実際に動かす(Move)は、ここではやらない
 */
#pragma once
#include <cstdint>
#include <random>
#include <vector>


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class EnemyStateMachine;


		/**
		 * @brief 目的地モードの設定
		 * @details あとで EnemyParameter(JSON)に移すと、大男とねずみ男で変えられて、ホットリロードで調整できる
		 */
		struct DestinationModeSetting
		{
			/** ふらつく角度の最大(度数) */
			float wanderAmplitudeDeg;
			/** ふらつく速さ(1秒に何回くらい揺れるか) */
			float wanderFrequency;
			/** 美術品の前で盗んでいる時間(秒) */
			float stealTime;
			/** 寄り道した先で見回している時間(秒) */
			float lookAroundTime;
			/** 寄り道するかを決める間隔(秒) */
			float detourCheckInterval;
			/** 1回の判定で寄り道する確率(0 ~ 1) */
			float detourChance;
			/** 寄り道する場所を、今の位置から何マス以内で探すか */
			int detourRadiusCell;
			/** 1つの美術品へ向かう間に、何回まで寄り道するか */
			int maxDetourPerTarget;
			/** 道が見つからなかった時に、次を試すまでの時間(秒) */
			float retryInterval;


			DestinationModeSetting()
				: wanderAmplitudeDeg(25.0f)
				, wanderFrequency(0.5f)
				, stealTime(3.0f)
				, lookAroundTime(2.0f)
				, detourCheckInterval(3.0f)
				, detourChance(0.3f)
				, detourRadiusCell(4)
				, maxDetourPerTarget(2)
				, retryInterval(1.0f)
			{}
		};




		/****************************************************/


		/**
		 * @brief 敵の目的地モード
		 * @details 「美術品を選ぶ → 向かう(途中で寄り道) → 盗む → 次の美術品」をくり返す。
		 *          警戒中は何もしない(警戒モードに任せる)
		 */
		class EnemyDestinationMode
		{
		public:
			/**
			 * @brief コンストラクタ
			 * @param stateMachine 命令を出すステートマシン(持ち主の敵のもの。借りるだけ)
			 */
			explicit EnemyDestinationMode(EnemyStateMachine* stateMachine);
			~EnemyDestinationMode();

			/**
			 * @brief 更新処理(行き先を決めて、命令を出す)
			 * @param deltaTime 1フレームの経過時間(秒)
			 * @details EnemyStateMachine::UpdatePathFollow より先に呼ぶ
			 */
			void Update(const float deltaTime);


		public:
			/**
			 * @brief 盗みに行く美術品の位置の一覧を設定
			 * @param targetList 美術品の位置の一覧
			 * @details 設定し直すと、最初から(次の美術品を選ぶところから)やり直す
			 */
			void SetTargetList(const std::vector<Vector3>& targetList);
			/**
			 * @brief 設定を変える
			 * @param setting 目的地モードの設定
			 */
			void SetSetting(const DestinationModeSetting& setting) { m_setting = setting; }

			/**
			 * @brief 今の段階の名前を取得(デバッグ表示用)
			 * @return 段階の名前
			 */
			const char* GetPhaseName() const;
			/**
			 * @brief 今向かっている美術品の番号を取得(デバッグ表示用)
			 * @return 美術品の番号(決まっていなければ -1)
			 */
			int GetTargetIndex() const { return m_targetIndex; }


		private:
			/** 目的地モードの段階 */
			enum class Phase : uint8_t
			{
				NoTarget,      // 美術品が無い(待つ)
				MoveToTarget,  // 美術品へ向かっている
				Detour,        // 寄り道している
				LookAround,    // 寄り道した先で見回している
				Steal,         // 美術品を盗んでいる
			};


		private:
			/**
			 * @brief 段階を変える(段階の経過時間を 0 に戻す)
			 * @param phase 次の段階
			 */
			void ChangePhase(const Phase phase);
			/**
			 * @brief 次に盗みに行く美術品を選ぶ
			 * @details 美術品が2つ以上あれば、今と違うものをランダムに選ぶ。寄り道の回数も0に戻す
			 */
			void ChooseNextTarget();
			/**
			 * @brief 今の美術品へ向かい始める
			 * @details 道が見つからなくても MoveToTarget のまま。少し待ってから次の美術品を試す
			 */
			void StartMoveToTarget();
			/**
			 * @brief 寄り道を始める
			 * @return 寄り道を始められたら true
			 * @details 始められなければ、美術品へ向かうのを続ける
			 */
			bool StartDetour();
			/**
			 * @brief 寄り道する場所を探す(今の位置の近くの、通れるマス)
			 * @param outPosition 寄り道する場所
			 * @return 見つかれば true
			 */
			bool FindDetourPoint(Vector3& outPosition);


		private:
			/** 命令を出すステートマシン(借りるだけ) */
			EnemyStateMachine* m_stateMachine;
			/** 設定 */
			DestinationModeSetting m_setting;
			/** 美術品の位置の一覧 */
			std::vector<Vector3> m_targetList;
			/** 今向かっている美術品の番号(決まっていなければ -1) */
			int m_targetIndex;

			/** 今の段階 */
			Phase m_phase;
			/** 今の段階になってからの経過時間(秒) */
			float m_phaseTimer;
			/** 寄り道するかを決めるまでの経過時間(秒) */
			float m_detourTimer;
			/** 今の美術品へ向かう間に寄り道した回数 */
			int m_detourCount;

			/** 乱数(美術品の選び方と、寄り道) */
			std::mt19937 m_random;
		};
	}
}
