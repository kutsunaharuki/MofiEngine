/**
 * @file MissionJudge.h
 * @brief ミッションを何で判断するかを決める
 * @details 判定のルールだけを持つ。数を数えたり、状態を変えたりするのは MissionManager の役目
 */
#pragma once
#include "Source/Mission/MissionData.h"
#include "Source/Mission/ShotMeasure.h"
#include <array>
#include <vector>


namespace app
{
	namespace mission
	{
		/**
		 * @brief 照合の項目(判定の順番)
		 */
		enum class MatchItem : uint8_t
		{
			Target,			// 狙いの悪党か
			Stealing,		// 盗んでいるか
			Focus,			// ピントが合っているか
			FaceAngle,		// 顔がこちらを向いているか
			FaceBlocked,	// 顔が隠れていないか
			FaceInFrame,	// 顔が枠の中か
			Yaw,			// 左右の角度
			Pitch,			// 上下の角度
			Size,			// 大きさ
			Position,		// 位置
			Max
		};

		
		/**
		 * @brief 照合の結果(項目ごとの合否と見本のずれ)
		 */
		struct MatchResult
		{
			/** 項目ごとの合否 */
			std::array<bool, static_cast<size_t>(MatchItem::Max)> isSuccess = {};
			/** 項目ごとのずれ(UI で「あと右へ 20°」と出すときに使用) */
			std::array<float, static_cast<size_t>(MatchItem::Max)> diff	   = {};

			/** 全部の項目が成功しているか */
			bool IsSuccess() const;
			/** NG の項目 */
			std::vector<MatchItem> GetFailedItems() const;
		};



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
			// アルバムに残した写真の番号(残していなければ -1)
			int photoIndex = -1;
			/** 写り方を測った値 */
			ShotMeasure measure;
		};


		/**
		 * @brief ミッションを何で判断するかを決める
		 */
		class MissionJudge
		{
		public:
			/**
			 * @brief 撮った写真をミッションの見本と照合する
			 * @param shot 撮った瞬間の情報
			 * @param mission ミッション
			 * @return 項目ごとの合否のずれ
			 */
			static MatchResult Judge(const ShotInfo& shot, const MissionData& mission);
		};
	}
}
