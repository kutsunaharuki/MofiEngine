/**
 * @file MissionManager.cpp
 * @brief ミッションの管理元
 */
#include "stdafx.h"
#include "MissionManager.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace mission
	{
		MissionManager::MissionManager()
		{}
		
		
		MissionManager::~MissionManager()
		{}


		void MissionManager::Initialize(const char* filePath)
		{
			nlohmann::json json;
			// 失敗したら読まない
			if (!util::JsonConverter::LoadFile(filePath, json)) return;
			
			// Initialize が2回以上呼ばれた時に、ミッションが二重に入らないようにする
			m_missionList.clear();
			m_countedStealList.clear();

			// ないなら読まない
			if (!json.contains("missions") || !json["missions"].is_array()) return;

			// Json のmissions の配列の中を読む
			for (const nlohmann::json& item : json["missions"])
			{
				MissionData mission;

				// これがミッションの文になる
				mission.text = util::JsonConverter::ToWString(item, "text");
				// EnemyType は std::string で直接には変換できないため ToEnemyType で変換
				mission.target = ToEnemyType(util::JsonConverter::ToString(item, "target"));
				// これは何枚撮ればいいか
				mission.requiredCount = util::JsonConverter::ToInt(item, "count", 0);
				// ミッションの一覧にミッションの構造を入れる
				m_missionList.push_back(mission);
			}

		}


		bool MissionManager::OnShot(const ShotInfo& shot)
		{
			// 敵が写っていなければ数えない
			if (shot.subject == nullptr) return false;

			// この盗みは、もう数えた(クールタイムの後に同じ盗みをもう一度撮っても、増やさない)
			auto it = m_countedStealList.find(shot.subject);
			if (it != m_countedStealList.end() && it->second == shot.stealCount) return false;

			bool isCounted = false;
			for (MissionData& mission : m_missionList)
			{
				if (!MissionJudge::IsDecisiveMoment(shot, mission)) continue;

				++mission.currentCount;
				// 必要な枚数を撮ったら達成
				if (mission.currentCount >= mission.requiredCount)
				{
					mission.state = MissionState::Achievement;
				}
				isCounted = true;
			}

			// 数えた時だけ、盗みの番号を覚える(ピンぼけで撮り直した時は、まだ数えられるようにする)
			if (isCounted)
			{
				m_countedStealList[shot.subject] = shot.stealCount;
			}
			return isCounted;
		}


		void MissionManager::Finish()
		{
			for (MissionData& mission : m_missionList)
			{
				// 達成していないミッションは失敗
				if (mission.state == MissionState::Progress)
				{
					mission.state = MissionState::Failed;
				}
			}
		}


		std::vector<MissionData> MissionManager::GetMissionList(const MissionState state) const
		{
			std::vector<MissionData> result;
			for (const MissionData& mission : m_missionList)
			{
				if (mission.state == state) result.push_back(mission);
			}
			return result;
		}


		bool MissionManager::IsAllAchieved() const
		{
			if (m_missionList.empty()) return false;

			for (const MissionData& mission : m_missionList)
			{
				if (mission.state != MissionState::Achievement) return false;
			}
			return true;
		}


		actor::EnemyBase::EnemyType MissionManager::ToEnemyType(const std::string& name)
		{
			// name が BigMan なら BigMan を返す
			if (name == "BigMan") return actor::EnemyBase::EnemyType::BigMan;
			// name が RatMan なら RatMan を返す
			if (name == "RatMan") return actor::EnemyBase::EnemyType::RatMan;

			// 返すのは最大
			return actor::EnemyBase::EnemyType::Max;
		}
	}
}