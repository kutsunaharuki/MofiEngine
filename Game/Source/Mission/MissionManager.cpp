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
		namespace
		{
			/**
			 * @brief 許容値を読む
			 * @param parent "tolerance" を持っている JSON
			 * @param base 初期値(共通の許容値)
			 */
			MatchTolerance ReadTolerance(const nlohmann::json& parent, const MatchTolerance& base)
			{
				MatchTolerance tolerance = base;
				// JSON にこの名前がないなら初期値
				if (!parent.contains("tolerance")) return tolerance;

				const nlohmann::json& item = parent["tolerance"];
				tolerance.faceAngleMax = util::JsonConverter::ToFloat(item, "faceAngleMax", base.faceAngleMax);
				tolerance.screenMargin = util::JsonConverter::ToFloat(item, "screenMargin", base.screenMargin);
				tolerance.yawDeg = util::JsonConverter::ToFloat(item, "yawDeg", base.yawDeg);
				tolerance.pitchDeg = util::JsonConverter::ToFloat(item, "pitchDeg", base.pitchDeg);
				tolerance.size = util::JsonConverter::ToFloat(item, "size", base.size);
				tolerance.position = util::JsonConverter::ToFloat(item, "position", base.position);
				return tolerance;
			}
		}


		MissionManager::MissionManager()
			: m_hasLastMatchResult(false)
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
			m_shotRecordList.clear();

			// 全部のミッションに共通の許容値
			m_commonTolerance = ReadTolerance(json, MatchTolerance());

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

				// 許容値: 共通の値を初期値にして、ミッションに書いた項目だけ上書き
				// ※ json(ファイル全体)ではなく item(このミッション)を渡す。json だと共通の値がそのまま入るだけになる
				mission.tolerance = ReadTolerance(item, m_commonTolerance);

				if (item.contains("sample"))
				{
					// ※ キーの名前は PhotoMatchDebugWindow の「Copy as sample」と同じにする(違うとエラーにならず 0 になる)
					const nlohmann::json& sample = item["sample"];
					mission.hasSample = true;
					mission.sample.yawDeg = util::JsonConverter::ToFloat(sample, "yawDeg", 0.0f);
					mission.sample.pitchDeg = util::JsonConverter::ToFloat(sample, "pitchDeg", 0.0f);
					mission.sample.size = util::JsonConverter::ToFloat(sample, "size", 0.0f);
					mission.sample.position = util::JsonConverter::ToVector2(sample, "position", Vector2(0.5f, 0.5f));
					mission.sample.poseTime = util::JsonConverter::ToFloat(sample, "poseTime", mission.sample.poseTime);
				}

				// ミッションの一覧にミッションの構造を入れる
				m_missionList.push_back(mission);
			}
		}


		bool MissionManager::OnShot(const ShotInfo& shot)
		{
			// 照合の結果は撮るたびに作り直す
			m_hasLastMatchResult = false;

			ShotRecord record;
			record.photoIndex = shot.photoIndex;
			const bool isCounted = JudgeShot(shot, record);

			// 数えなかった写真も、リザルトで「なぜダメだったか」を出すために残す
			m_shotRecordList.push_back(record);
			return isCounted;
		}


		bool MissionManager::JudgeShot(const ShotInfo& shot, ShotRecord& record)
		{
			// 敵が写っていなければ数えない
			if (shot.subject == nullptr)
			{
				record.status = ShotStatus::NoSubject;
				return false;
			}
			record.subjectType = shot.subject->GetEnemyType();

			// この盗みは、もう数えた(クールタイムの後に同じ盗みをもう一度撮っても、増やさない)
			auto it = m_countedStealList.find(shot.subject);
			if (it != m_countedStealList.end() && it->second == shot.stealCount)
			{
				record.status = ShotStatus::AlreadyCounted;
				return false;
			}

			bool isCounted = false;
			// 進行中のミッションが無ければ、照合しない
			record.status = ShotStatus::NoMission;

			// 結果を覚えたミッションの番号(ジャッジの UI で見本の絵を出すのに使う)
			int lastMissionIndex = -1;

			for (int missionIndex = 0; missionIndex < static_cast<int>(m_missionList.size()); ++missionIndex)
			{
				MissionData& mission = m_missionList[missionIndex];

				// 終わったミッションはもう数えない
				if (mission.state != MissionState::Progress) continue;

				// 見本と撮った写真を照合
				const MatchResult result = MissionJudge::Judge(shot, mission);
				// 狙いの悪党のミッションの結果を覚える
				const bool isTargetMission = result.isSuccess[static_cast<size_t>(MatchItem::Target)];
				// 悪党のミッション結果がある または 最後に撮った照合結果がないなら
				if (isTargetMission || !m_hasLastMatchResult)
				{
					m_lastMatchResult = result;
					m_hasLastMatchResult = true;
					lastMissionIndex = missionIndex;
				}

				// 照合した(数えるかは、この後で決まる)
				if (!isCounted)
				{
					record.status = ShotStatus::Failed;
					record.result = m_lastMatchResult;
					record.missionIndex = lastMissionIndex;
				}

				// 全ての項目が成功(イメージはチェックマーク)していないなら飛ばす
				if (!result.IsSuccess()) continue;

				// 数えたミッションの結果を残す
				record.status = ShotStatus::Counted;
				record.result = result;
				record.missionIndex = missionIndex;

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


		bool MissionManager::DebugAddCount(const int index)
		{
			if (index < 0 || index >= static_cast<int>(m_missionList.size())) return false;

			MissionData& mission = m_missionList[index];
			if (mission.state != MissionState::Progress) return false;

			// 撮って数えた時と同じ: 1 枚増やして、必要な枚数になったら達成
			++mission.currentCount;
			if (mission.currentCount >= mission.requiredCount)
			{
				mission.state = MissionState::Achievement;
			}
			return true;
		}


		void MissionManager::DebugAchieveAll()
		{
			for (MissionData& mission : m_missionList)
			{
				if (mission.state != MissionState::Progress) continue;

				mission.currentCount = mission.requiredCount;
				mission.state = MissionState::Achievement;
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