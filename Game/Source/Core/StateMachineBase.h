/**
 * @file ActorStateMachineBase.h
 * @brief アクターのステートマシンの基底クラス群
 */
#pragma once
#include "Source/Util/CRC32.h"

/**
 * @brief 数値に変換するマクロ
 * @param name ステート名
 */
#define appState(name)\
public:\
static constexpr uint32_t ID() { return Hash32(#name);}


namespace app
{
	namespace core
	{
		/** 前方宣言 */
		class StateMachineBase;


		/**
		 * @brief ステートの基底クラス
		 */
		class IState
		{
		public:
			IState() = default;
			virtual ~IState() = default;


		public:
			/** ステートに入った時の処理 */
			virtual void Enter() = 0;
			/** ステートの更新処理 */
			virtual void Update() = 0;
			/** ステートから出るときの処理 */
			virtual void Exit() = 0;
		};





		/***************************************************/


		/**
		 * @brief アクターのステートマシンの基底クラス
		 */
		class StateMachineBase
		{
		public:
			/**
			 * @brief ステートマシンを更新
			 * @details 持ち主が毎フレーム呼び出す必要がある
			 */
			void Update();
			/** 
			 * @brief ステートを変更
			 */
			void ChangeState();
			/**
			 * @brief 現在のステートと指定したIDが同じかどうか
			 * @param stateID ステートID
			 * @return 同じかどうか
			 */
			bool IsEqualState(const uint32_t stateID) const;
			/**
			 * @brief 現在のステートの Enter() を再度呼び出す
			 * @details モデルロード完了後にアニメーションを再適用するために使用する
			 */
			void ReEnterCurrentState();


		protected:
			template<typename TState, typename TStateMachine>
			void AddState(TStateMachine& stateMachine)
			{
				// 指定したIDを取得
				auto it = m_stateMap.find(TState::ID());

				// ここに入ったら警告
				if (it != m_stateMap.end())
				{
					// 既存のものを削除した後に警告を出す
					delete it->second.get();
					K2_ASSERT(false, "重複しています。");
				}

				// ステートを追加
				/**
				 * @details std::forward は完璧転送を行うために使用される。
				 *  受け取った引数の型(左辺値または右辺値)を保持して、それを他の関数に受け渡す際にその特性を失わせないもの。
				 */
				m_stateMap[TState::ID()] = std::make_unique<TState>(std::forward<TStateMachine>(stateMachine));
			}

			/**
			 * @brief ステートの変更先を取得
			 * @return 変更先のステート
			 */
			virtual IState* GetChangeState() = 0;
			/**
			 * @brief ステートを取得
			 * @param stateID ステートID
			 * @return ステートのポインタ
			 */
			IState* FindState(const uint32_t stateID);


		public:
			StateMachineBase();
			virtual ~StateMachineBase() = default;


		protected:
			/** ステートのマップ */
			std::unordered_map<uint32_t, std::unique_ptr<IState>> m_stateMap;
			/** 現在のステート */
			IState* m_currentState;
			/** 次のステート */
			IState* m_nextState;
		};
	}
}

