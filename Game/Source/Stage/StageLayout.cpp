/**
 * @file StageLayout.cpp
 * @brief ステージの構造(階・区画・階段)
 */
#include "stdafx.h"
#include "StageLayout.h"
#include "Source/Util/JsonConverter.h"
#include <algorithm>


namespace app
{
	namespace stage
	{
		namespace
		{
			/**
			 * 階を決める時に、高さに足す余裕
			 * ※ 床の上に立つキャラクターの y は、床とぴったり同じではない(少し沈む・浮く)ので、
			 *    床より少し下でも、その階に入るようにする
			 */
			constexpr float FLOOR_TOLERANCE = 50.0f;
			/** 階が無い */
			constexpr int NO_FLOOR = -1;
		}


		StageLayout::StageLayout()
		{}


		StageLayout::~StageLayout()
		{}


		bool StageLayout::Load(const std::string& filePath)
		{
			m_floorList.clear();
			m_stairsList.clear();

			nlohmann::json json;
			if (!util::JsonConverter::LoadFile(filePath, json)) return false;

			if (json.contains("floors") && json["floors"].is_array())
			{
				for (const nlohmann::json& floorItem : json["floors"])
				{
					StageFloor floor;
					floor.name = util::JsonConverter::ToString(floorItem, "name");
					floor.height = util::JsonConverter::ToFloat(floorItem, "height");

					if (floorItem.contains("areas") && floorItem["areas"].is_array())
					{
						for (const nlohmann::json& areaItem : floorItem["areas"])
						{
							StageArea area;
							area.name = util::JsonConverter::ToString(areaItem, "name");
							area.min = util::JsonConverter::ToVector3(areaItem, "min");
							area.max = util::JsonConverter::ToVector3(areaItem, "max");
							floor.areaList.push_back(area);
						}
					}
					m_floorList.push_back(floor);
				}
			}

			if (json.contains("stairs") && json["stairs"].is_array())
			{
				for (const nlohmann::json& stairsItem : json["stairs"])
				{
					StageStairs stairs;
					stairs.name = util::JsonConverter::ToString(stairsItem, "name");
					stairs.from = util::JsonConverter::ToString(stairsItem, "from");
					stairs.to = util::JsonConverter::ToString(stairsItem, "to");
					stairs.min = util::JsonConverter::ToVector3(stairsItem, "min");
					stairs.max = util::JsonConverter::ToVector3(stairsItem, "max");
					m_stairsList.push_back(stairs);
				}
			}

			// 低い階から順に並べておく(FindFloorIndex が下から調べるため)
			std::sort(m_floorList.begin(), m_floorList.end(),
				[](const StageFloor& a, const StageFloor& b) { return a.height < b.height; });

			return true;
		}


		int StageLayout::FindFloorIndex(const float height) const
		{
			if (m_floorList.empty()) return NO_FLOOR;

			// 床が高さ以下の階のうち、一番上の階(一番下の階より低ければ、一番下の階)
			int floorIndex = 0;
			for (int i = 0; i < static_cast<int>(m_floorList.size()); ++i)
			{
				if (m_floorList[i].height <= height + FLOOR_TOLERANCE)
				{
					floorIndex = i;
				}
			}
			return floorIndex;
		}


		const StageArea* StageLayout::FindArea(const Vector3& position) const
		{
			// 今いる階の区画だけを調べる
			const int floorIndex = FindFloorIndex(position.y);
			if (floorIndex == NO_FLOOR) return nullptr;

			for (const StageArea& area : m_floorList[floorIndex].areaList)
			{
				if (area.Contains(position)) return &area;
			}
			return nullptr;
		}
	}
}
