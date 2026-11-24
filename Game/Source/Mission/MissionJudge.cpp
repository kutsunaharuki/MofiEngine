/**
 * @file MissionJudge.cpp
 * @brief ミッションを何で判断するかを決める
 */
#include "stdafx.h"
#include "MissionJudge.h"
#include <cmath>


namespace app
{
	namespace mission
	{
		namespace
		{
			/** 半回転の角度 */
			constexpr float HALF_TURN_DEG = 180.0f;
			/** 1 回転の角度 */
			constexpr float FULL_TURN_DEG = 360.0f;
			/** 画面の半分 */
			constexpr float HALF = 0.5f;



			/** 
			 * @brief 角度の差を -180 ～ 180 に回す
			 * @param deg 角度の差(度数)
			 * @return -180 ～ 180 に回した角度
			 */
			float WrapDeg(float deg)
			{
				while (deg > HALF_TURN_DEG) deg -= FULL_TURN_DEG;
				while (deg <= -HALF_TURN_DEG) deg += FULL_TURN_DEG;
				return deg;
			}
		}


		MatchResult MissionJudge::Judge(const ShotInfo& shot, const MissionData& mission)
		{
			MatchResult result;

			// 項目の合否とずれを入れる
			auto set = [&result](const MatchItem item, const bool isSuccess, const float diff)
				{
					result.isSuccess[static_cast<size_t>(item)] = isSuccess;
					result.diff[static_cast<size_t>(item)] = diff;
				};

			const ShotMeasure& measure = shot.measure;
			const MatchTolerance& tolerance = mission.tolerance;

			// 誰を・盗んでいる・ピント(途中で return しない：ずれた項目を全部出すため)
			const bool isTarget = shot.subject != nullptr && shot.subject->GetEnemyType() == mission.target;
			set(MatchItem::Target, isTarget, 0.0f);
			set(MatchItem::Stealing, shot.isStealing, 0.0f);
			set(MatchItem::Focus, shot.isInFocus, 0.0f);

			// 顔が見えている(見本の有無に関わらず必ず見る)
			// 顔がこちらを向いているか(ずれ = 許容値を何度超えたか。マイナスなら余裕がある)
			const bool isFaceToward = measure.isMeasured && measure.faceAngleDeg <= tolerance.faceAngleMax;
			set(MatchItem::FaceAngle, isFaceToward, measure.faceAngleDeg - tolerance.faceAngleMax);
			// 顔が隠れていないか
			set(MatchItem::FaceBlocked, measure.isMeasured && !measure.isFaceBlocked, 0.0f);

			// 顔が枠の中か(画面の端から、余白より内側。UI の座標は真ん中が原点なので、端までは幅・高さの半分)
			// ※ UI_SPACE_WIDTH は UINT(符号なし)なので、float にしてから計算する
			const float frameHalfWidth = static_cast<float>(UI_SPACE_WIDTH) * HALF - tolerance.screenMargin;
			const float frameHalfHeight = static_cast<float>(UI_SPACE_HEIGHT) * HALF - tolerance.screenMargin;
			
			const bool isInFrame = measure.isMeasured && measure.isFaceInFront
				&& std::fabs(measure.faceScreenPosition.x) <= frameHalfWidth
				&& std::fabs(measure.faceScreenPosition.y) <= frameHalfHeight;

			set(MatchItem::FaceInFrame, isInFrame, 0.0f);

			// 見本が無いミッション(と、構図を見ない時)は、角度・大きさ・位置を見ない(成功にしておく)
			// ※ JSON に見本を書き足しながら、ミッションを 1 つずつ進められるように
			if (!mission.hasSample || !IS_CHECK_COMPOSITION)
			{
				set(MatchItem::Yaw, true, 0.0f);
				set(MatchItem::Pitch, true, 0.0f);
				set(MatchItem::Size, true, 0.0f);
				set(MatchItem::Position, true, 0.0f);
				return result;
			}

			const PhotoSample& sample = mission.sample;

			// 左右(回り込んで 180° をまたぐので、差を -180 ～ 180 に回してから比べる)
			const float yawDiff = WrapDeg(measure.yawDeg - sample.yawDeg);
			set(MatchItem::Yaw, std::fabs(yawDiff) <= tolerance.yawDeg, yawDiff);

			// 上下
			const float pitchDiff = measure.pitchDeg - sample.pitchDeg;
			set(MatchItem::Pitch, std::fabs(pitchDiff) <= tolerance.pitchDeg, pitchDiff);

			// 大きさ(画面の高さの割合)
			const float sizeDiff = measure.size - sample.size;
			set(MatchItem::Size, std::fabs(sizeDiff) <= tolerance.size, sizeDiff);

			// 位置(見本の位置からの距離。画面の中の 0 ～ 1)
			const float positionDiffX = measure.position.x - sample.position.x;
			const float positionDiffY = measure.position.y - sample.position.y;
			const float positionDiff = std::sqrt(positionDiffX * positionDiffX + positionDiffY * positionDiffY);
			set(MatchItem::Position, positionDiff <= tolerance.position, positionDiff);

			return result;
		}


		bool MatchResult::IsSuccess() const
		{
			// 1 つでも失敗があれば、照合は NG
			for (const bool isItemSuccess : isSuccess)
			{
				if (!isItemSuccess) return false;
			}
			return true;
		}




		/*******************************************************************************/


		std::vector<MatchItem> MatchResult::GetFailedItems() const
		{
			// 失敗した項目を、判定の順番(MatchItem の順番)で並べる(4 の UI はこの順に出す)
			std::vector<MatchItem> failedList;
			for (size_t i = 0; i < isSuccess.size(); ++i)
			{
				if (!isSuccess[i]) failedList.push_back(static_cast<MatchItem>(i));
			}
			return failedList;
		}


		float MatchResult::GetSuccessRate() const
		{
			// 狙いと違う悪党を撮った時は、ほかの項目が合っていても意味が無いので 0
			if (!isSuccess[static_cast<size_t>(MatchItem::Target)]) return 0.0f;

			// 構図を見ない時は、構図の項目を数えない(いつも成功なので、一致度が高く出すぎる)
			int successCount = 0;
			int itemCount = 0;
			for (size_t i = 0; i < isSuccess.size(); ++i)
			{
				if (!IS_CHECK_COMPOSITION && IsCompositionItem(static_cast<MatchItem>(i))) continue;
				++itemCount;
				if (isSuccess[i]) ++successCount;
			}
			if (itemCount == 0) return 0.0f;
			return static_cast<float>(successCount) / static_cast<float>(itemCount);
		}
	}
}