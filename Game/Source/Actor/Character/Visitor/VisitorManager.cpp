/**
 * @file VisitorManager.cpp
 * @brief 一般客の管理場所
 */
#include "stdafx.h"
#include "VisitorManager.h"
#include "VisitorGroup.h"
#include "Source/AI/Navigation/NavGrid.h"
#include "Source/Util/JsonConverter.h"
#include <algorithm>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 組どうしの最低の間隔の初期値 */
			constexpr float SPAWN_MIN_DISTANCE = 300.0f;
			/** 通れるマスを探す範囲の初期値(マスの数) */
			constexpr int SPAWN_SEARCH_CELL = 10;
			/** 出す場所が決められない時の場所の初期値 */
			const Vector3 FALLBACK_POSITION = { 0.0f,0.0f,-300.0f };
			/** 出す場所が決められない時に、組ごとにずらす間隔 */
			constexpr float FALLBACK_OFFSET_X = 200.0f;
			/** 出す場所を選び直す最大の回数(ほかの組と近すぎた時) */
			constexpr int SPAWN_RETRY_MAX = 20;

			/** 組の種類の設定の JSONパス */
			constexpr const char* GROUP_TYPE_JSON_PATH = "Assets/parameter/Actor/Character/Visitor/VisitorGroupType.json";
			/** 組の種類の名前(GroupType と同じ順番) */
			const char* GROUP_TYPE_NAME_LIST[] = { "Solo","Student","Couple","Family" };

			// コンパイル時にチェック
			static_assert(sizeof(GROUP_TYPE_NAME_LIST) / sizeof(GROUP_TYPE_NAME_LIST[0]) == static_cast<size_t>(GroupType::Max),
				"GROUP_TYPE_NAME_LIST と GroupType の数があっていません。");


			/**
			 * @brief "Couple" 等の名前を組の種類にする
			 * @param JSON の中の名前
			 * @param outList 足す先
			 * @return 名前が見つかれば true
			 */
			bool ToGroupType(const std::string& name, GroupType& outType)
			{
				for (int i = 0; i < static_cast<int>(GroupType::Max); ++i)
				{
					// 名前が違うならスキップ
					if (name != GROUP_TYPE_NAME_LIST[i]) continue;
					outType = static_cast<GroupType>(i);
					return true;
				}
				return false;
			}


			/**
			 * @brief 見た目の名前の配列を読む
			 * @param item JSON の配列
			 * @param key 名前
			 * @param outList 足す先
			 */
			void ReadTypeList(const nlohmann::json& item, const char* key, std::vector<Visitor::VisitorType>& outList)
			{
				// JSON の配列が無いならここで終了
				if (!item.contains(key) || !item[key].is_array()) return;

				for (const nlohmann::json& name : item[key])
				{
					if (!name.is_string()) continue;

					Visitor::VisitorType type;
					if (Visitor::ToVisitorType(name.get<std::string>(), type)) outList.push_back(type);
				}
			}
		}


		VisitorManager::VisitorManager()
			: m_pathfinder(nullptr)
			, m_navGrid(nullptr)
			, m_spawnMinDistance(SPAWN_MIN_DISTANCE)
			, m_spawnSearchCell(SPAWN_SEARCH_CELL)
			, m_fallbackPosition(FALLBACK_POSITION)
			, m_random(std::random_device{}())
		{}


		// ※ VisitorGroup を前方宣言しているので、unique_ptr を消すデストラクタは .cpp に書く
		VisitorManager::~VisitorManager()
		{}


		void VisitorManager::Initialize(
			const char* spawnFilePath,
			const ai::Pathfinder* pathfinder,
			const ai::NavGrid* navGrid,
			const std::vector<Vector3>& targetList)
		{
			m_pathfinder = pathfinder;
			m_navGrid = navGrid;
			m_targetList = targetList;

			// 2 回呼ばれても、二重に作らない
			m_soloList.clear();
			m_groupList.clear();
			m_spawnedPositionList.clear();

			// 組の種類ごとの設定を読む(何人で来るか・どの見た目か)
			LoadGroupTypeSetting(GROUP_TYPE_JSON_PATH);

			// 読めなければ、誰も出さない
			nlohmann::json json;
			if (!util::JsonConverter::LoadFile(spawnFilePath, json)) return;

			m_spawnMinDistance = util::JsonConverter::ToFloat(json, "spawnMinDistance", SPAWN_MIN_DISTANCE);
			m_spawnSearchCell = (std::max)(0, util::JsonConverter::ToInt(json, "spawnSearchCell", SPAWN_SEARCH_CELL));
			m_fallbackPosition = util::JsonConverter::ToVector3(json, "fallbackPosition", FALLBACK_POSITION);

			if (!json.contains("spawn") || !json["spawn"].is_array()) return;

			for (const nlohmann::json& item : json["spawn"])
			{
				GroupType type;
				if (!ToGroupType(util::JsonConverter::ToString(item, "type"), type)) continue;

				const int count = (std::max)(0, util::JsonConverter::ToInt(item, "count", 0));
				// カウント回数分の組を作成
				for (int i = 0; i < count; ++i)
				{
					CreateGroup(type);
				}
			}
		}


		void VisitorManager::Update()
		{
			for (auto& solo : m_soloList)
			{
				solo->Update();
			}
			for (auto& group : m_groupList)
			{
				group->Update();
			}
		}


		void VisitorManager::Render(RenderContext& rc)
		{
			for (auto& solo : m_soloList)
			{
				solo->Render(rc);
			}
			for (auto& group : m_groupList)
			{
				group->Render(rc);
			}
		}


		void VisitorManager::LoadGroupTypeSetting(const char* filePath)
		{
			// 読み直したときに前の設定が残らないように空に
			for (GroupTypeSetting& setting : m_groupTypeSettingList)
			{
				setting = GroupTypeSetting();
			}


			nlohmann::json json;
			// JSON ファイル読み込み時に無いなら読まない
			if (!util::JsonConverter::LoadFile(filePath, json)) return;

			
			for (int i = 0; i < static_cast<int>(GroupType::Max); ++i)
			{
				const char* name = GROUP_TYPE_NAME_LIST[i];
				if (!json.contains(name)) continue;

				const nlohmann::json& item = json[name];
				GroupTypeSetting& setting = m_groupTypeSettingList[i];

				// 人数の候補
				if (item.contains("variations") && item["variations"].is_array())
				{
					for (const nlohmann::json& variationItem : item["variations"])
					{
						Variation variation;
						variation.memberCount = (std::max)(1, util::JsonConverter::ToInt(variationItem, "memberCount", 1));
						variation.hasStroller = util::JsonConverter::ToBool(variationItem, "stroller", false);
						variation.weight = util::JsonConverter::ToFloat(variationItem, "weight", 1.0f);
						setting.variationList.push_back(variation);
					}
				}

				// 見た目の候補
				ReadTypeList(item, "appearance", setting.appearanceList);
				ReadTypeList(item, "adult", setting.adultList);
				ReadTypeList(item, "child", setting.childList);
				setting.adultCount = util::JsonConverter::ToInt(item, "adultCount", 0);
				setting.isNoDuplicate = util::JsonConverter::ToBool(item, "noDuplicate", false);
			}
		}


		void VisitorManager::CreateGroup(const GroupType type)
		{
			// 組の一覧を取得(変換して持つ)
			const GroupTypeSetting& setting = m_groupTypeSettingList[static_cast<size_t>(type)];

			// 人数を決める(候補がないなら作らない)
			const Variation* variation = PickVariation(setting);
			if (!variation) return;

			// 誰が来るかを決める
			const GroupComposition composition = MakeComposition(type, setting, *variation);
			// リストが空なら止める
			if (composition.memberTypeList.empty()) return;

			// 組を作る(必要な機能を設定)
			auto group = std::make_unique<VisitorGroup>();
			group->SetNavigation(m_pathfinder, m_navGrid);
			group->SetTargetList(m_targetList);
			group->SetSpawn(FindSpawnPosition(), composition);
			group->Initialize();

			// 決まった人数で一覧を分ける(1人の学生も1人の客の一覧に入る)
			if (composition.memberTypeList.size() == 1)
			{
				m_soloList.push_back(std::move(group));
				return;
			}
			m_groupList.push_back(std::move(group));
		}


		const VisitorManager::Variation* VisitorManager::PickVariation(const GroupTypeSetting & setting)
		{
			// 人数の候補リスト
			const std::vector<Variation>& variationList = setting.variationList;
			
			// 空なら nullptr
			if (variationList.empty()) return nullptr;

			// 候補が 1つ(カップル・学生)ならくじは引かない
			if (variationList.size() == 1) return &variationList.front();


			// 選ばれやすさを並べる
			std::vector<float> weightList;
			float totalWeight = 0.0f;

			// 加えるごとに選ばれやすい
			for (const Variation& variation : variationList)
			{
				weightList.push_back(variation.weight);
				totalWeight += variation.weight;
			}

			// 全部 0 の時は同じ確率にする(discrete_distribution は合計が 0だと正しく選べない)
			if (totalWeight <= 0.0f)
			{
				std::uniform_int_distribution<int> indexRange(0, static_cast<int>(variationList.size()) - 1);
				return &variationList[indexRange(m_random)];
			}

			// weight が大きい候補ほど出やすい
			std::discrete_distribution<int> variationRange(weightList.begin(), weightList.end());
			return &variationList[variationRange(m_random)];
		}


		GroupComposition VisitorManager::MakeComposition(
			const GroupType type, 
			const GroupTypeSetting & setting,
			const Variation & variation)
		{
			GroupComposition composition;
			composition.type = type;
			composition.hasStroller = variation.hasStroller;

			// 家族(大人と子供を分ける種類): 先頭に大人、残りが子供
			if (!setting.adultList.empty())
			{
				// 親(先頭)は必ず大人にする(ベビーカーを押すのも先頭の人)
				const int adultCount = TClamp<int>::Clamp(setting.adultCount, 1, variation.memberCount);
				PickTypes(setting.adultList, adultCount, setting.isNoDuplicate, composition.memberTypeList);
				
				// 子供の候補がないときは大人から選ぶ
				const std::vector<Visitor::VisitorType>& childCandidateList = setting.childList.empty() ? setting.adultList : setting.childList;
				PickTypes(childCandidateList, variation.memberCount - adultCount, false, composition.memberTypeList);

				return composition;
			}

			// それ以外(1人・学生・カップル): 見た目の候補から人数分
			PickTypes(setting.appearanceList, variation.memberCount, setting.isNoDuplicate, composition.memberTypeList);
			return composition;
		}


		void actor::VisitorManager::PickTypes(
			const std::vector<Visitor::VisitorType>& candidateList, 
			const int count, 
			const bool isNoDuplicate,
			std::vector<Visitor::VisitorType>& outList)
		{
			// 選べる見た目が空 or 人が 0未満なら
			if (candidateList.empty() || count <= 0) return;

			// 重複なしの時: 候補を混ぜて前から順に取る(例:) トランプを切って上から配るのと同じ)
			std::vector<Visitor::VisitorType> shuffledList = candidateList;
			// std::shuffle で指定された範囲内の要素をランダムで配置
			std::shuffle(shuffledList.begin(), shuffledList.end(), m_random);

			std::uniform_int_distribution<int> candidateRange(0, static_cast<int>(candidateList.size()) - 1);

			for (int i = 0; i < count; ++i)
			{
				// 重複なしでまだ配れる候補が残っていれば混ぜた順に取る
				if (isNoDuplicate && i < static_cast<int>(shuffledList.size()))
				{
					outList.push_back(shuffledList[i]);
					continue;
				}
				
				// 重複してよい時・候補を配り切った時はランダムで選ぶ
				outList.push_back(candidateList[candidateRange(m_random)]);
			}
		}


		Vector3 VisitorManager::FindSpawnPosition()
		{
			// 美術品か地図が無ければ、決まった場所から横にずらして並べる
			if (m_targetList.empty() || m_navGrid == nullptr || !m_navGrid->IsBuilt())
			{
				Vector3 position = m_fallbackPosition;
				position.x += FALLBACK_OFFSET_X * static_cast<float>(m_spawnedPositionList.size());
				m_spawnedPositionList.push_back(position);
				return position;
			}

			// 美術品をランダムに選んで、その前の通れるマスに出す(ほかの組と近すぎたら選び直す)
			std::uniform_int_distribution<int> targetRange(0, static_cast<int>(m_targetList.size()) - 1);
			Vector3 firstFound = m_fallbackPosition;
			bool isFound = false;
			for (int retry = 0; retry < SPAWN_RETRY_MAX; ++retry)
			{
				Vector3 position;
				if (!FindOpenPositionNear(m_targetList[targetRange(m_random)], position)) continue;

				if (IsFarFromSpawned(position))
				{
					m_spawnedPositionList.push_back(position);
					return position;
				}

				// 離れた場所が無い時のために、最初に見つけた場所を覚えておく
				if (!isFound)
				{
					firstFound = position;
					isFound = true;
				}
			}

			// 組が多くて離れた場所が無い時は、近くても出す
			m_spawnedPositionList.push_back(firstFound);
			return firstFound;
		}


		bool VisitorManager::FindOpenPositionNear(const Vector3& position, Vector3& outPosition) const
		{
			int centerX = 0;
			int centerZ = 0;
			if (!m_navGrid->WorldToCell(position, centerX, centerZ)) return false;

			// 近い輪から順に、通れるマスを探す(壁に掛かった絵は通れないマスにあるので、その前のマスが見つかる)
			for (int radius = 0; radius <= m_spawnSearchCell; ++radius)
			{
				for (int dz = -radius; dz <= radius; ++dz)
				{
					for (int dx = -radius; dx <= radius; ++dx)
					{
						// 輪の外周だけ見る(内側はもう見た)
						if ((std::max)(std::abs(dx), std::abs(dz)) != radius) continue;

						const int x = centerX + dx;
						const int z = centerZ + dz;
						if (m_navGrid->IsBlocked(x, z)) continue;

						outPosition = m_navGrid->CellToWorld(x, z);
						return true;
					}
				}
			}
			return false;
		}


		bool VisitorManager::IsFarFromSpawned(const Vector3& position) const
		{
			const float minDistanceSq = m_spawnMinDistance * m_spawnMinDistance;
			for (const Vector3& spawned : m_spawnedPositionList)
			{
				const Vector3 diff = position - spawned;
				if (diff.LengthSq() < minDistanceSq) return false;
			}
			return true;
		}
	}
}
