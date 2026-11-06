/**
 * @file MissionJudge.h
 * @brief ミッションを何で判断するかを決める
 * @details 判定のルールだけを持つ。数を数えたり、状態を変えたりするのは MissionManager の役目
 */
#pragma once
#include "Source/Mission/MissionData.h"


namespace app
{
	namespace mission
	{
		/**
		 * @brief 撮った瞬間の情報
		 * @details InGameScene が撮影したフレームに作って、MissionManager に渡す
		 */
		struct ShotInfo
		{
			// 写っている敵(ピントの被写体が敵でなければ nullptr)
			const actor::EnemyBase* subject = nullptr;
			// ピントが合っているか
			bool isInFocus = false;
			// 撮った瞬間に盗んでいるか
			bool isStealing = false;
			// 撮った時の盗みの番号(EnemyStateMachine::GetStealCount)
			int stealCount = 0;
		};


		/**
		 * @brief ミッションを何で判断するかを決める
		 */
		class MissionJudge
		{
		public:
			/**
			 * @brief 撮った写真が、ミッションの「決定的瞬間」かどうか
			 * @param shot 撮った瞬間の情報
			 * @param mission ミッション
			 * @return 決定的瞬間なら true
			 * @details 進行中のミッションで、狙いの敵が盗んでいるところに、ピントが合っていれば true
			 */
			static bool IsDecisiveMoment(const ShotInfo& shot, const MissionData& mission);
		};
	}
}
