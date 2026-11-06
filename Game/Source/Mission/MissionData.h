/**
 * @file MissionData.h
 * @brief ミッションデータ
 */
#pragma once
#include "Source/Actor/Character/Enemy/EnemyBase.h"


namespace app
{
	namespace mission
	{
		/** ミッションの進捗状態 */
		enum class MissionState : uint8_t
		{
			Progress,      // 進行中
			Achievement,   // 達成
			Failed         // 失敗
		};


		/**
		 * @brief ミッションデータ
		 */
		struct MissionData
		{
			// UI に出す文(UIText がwchar_t型だから)
			std::wstring text;
			// 誰を撮るか
			actor::EnemyBase::EnemyType target = actor::EnemyBase::EnemyType::Max;
			// 何枚撮るか
			int requiredCount = 0;
			// 今何枚か
			int currentCount = 0;
			// ミッションの進捗状態
			MissionState state = MissionState::Progress;
		};
	}
}