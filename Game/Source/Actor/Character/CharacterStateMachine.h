/**
 * @file CharacterStateMachine.h
 * @brief キャラクターステートマシン
 */
#pragma once
#include "CharacterBase.h"
#include "Source/Actor/ActorStateMachine.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief キャラクターステートマシン
		 */
		class CharacterStateMachine : public ActorStateMachine
		{
		public:
			CharacterStateMachine(CharacterBase* owner);
			virtual ~CharacterStateMachine() override = default;

			/**
			 * @brief キャラクターが地面に着いているかどうかを判定
			 * @return キャラクターがついてたらtrue
			 */
			inline bool IsOnGround() const { return m_characterOwner->IsOnGround(); }

			/**
			 * @brief アニメーションをクリップ番号で再生する
			 * @param clipIndex クリップ番号
			 * @param interpolationTime 補間時間(単位/秒)
			 */
			inline void PlayAnimation(int clipIndex, float interpolationTime)
			{
				m_characterOwner->GetModelRender()->PlayAnimation(clipIndex, interpolationTime);
			}


		protected:
			/** キャラクターのオーナー */
			CharacterBase* m_characterOwner;
		};
	}
}



