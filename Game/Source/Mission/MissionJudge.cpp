/**
 * @file MissionJudge.cpp
 * @brief ミッションを何で判断するかを決める
 */
#include "stdafx.h"
#include "MissionJudge.h"


namespace app
{
	namespace mission
	{
		bool MissionJudge::IsDecisiveMoment(const ShotInfo& shot, const MissionData& mission)
		{
			// 終わったミッションは、もう数えない
			if (mission.state != MissionState::Progress) return false;
			// 敵が写っていなければ、決定的瞬間ではない
			if (shot.subject == nullptr) return false;
			// ピンぼけの写真は、証拠にならない
			if (!shot.isInFocus) return false;
			// 盗んでいる瞬間でなければ、決定的瞬間ではない
			if (!shot.isStealing) return false;

			// 狙いの敵かどうか
			return shot.subject->GetEnemyType() == mission.target;
		}
	}
}
