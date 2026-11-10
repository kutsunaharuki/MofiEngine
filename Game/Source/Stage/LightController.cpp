/**
 * @file LightController.cpp
 * @brief ステージの明かり(ライトの設定と、影を作る範囲)
 */
#include "stdafx.h"
#include "LightController.h"
#include "StageLayout.h"


namespace app
{
	namespace stage
	{
		namespace
		{
			/** 明かりのパラメーターのパス */
			constexpr const char* LIGHT_PARAMETER_PATH = "Assets/parameter/Light/Light.json";
			/**
			 * 影の範囲の上下の余白(床の厚みの分)
			 * ※ 下の階の範囲は、上の階の床より少し下で終わる(上の階のキャラクターの足が、下の階の影にならないように)
			 */
			constexpr float SHADOW_AREA_FLOOR_MARGIN = 10.0f;


			/** 箱 a を、箱 b も入るように広げる(XZ だけ) */
			void ExpandXZ(Vector3& areaMin, Vector3& areaMax, const Vector3& otherMin, const Vector3& otherMax)
			{
				if (otherMin.x < areaMin.x) areaMin.x = otherMin.x;
				if (otherMin.z < areaMin.z) areaMin.z = otherMin.z;
				if (otherMax.x > areaMax.x) areaMax.x = otherMax.x;
				if (otherMax.z > areaMax.z) areaMax.z = otherMax.z;
			}
		}


		LightController::LightController()
			: m_parameter()
			, m_shadowAreaList()
		{}


		LightController::~LightController()
		{}


		void LightController::Initialize(const StageLayout* layout)
		{
			m_parameter.Initialize(LIGHT_PARAMETER_PATH);
			if (layout == nullptr) return;

			BuildShadowAreaList(*layout);

			// 階ごとに影の範囲を置く(ステージの形は変わらないので、最初に 1 回だけ)
			RenderingEngine& renderingEngine = RenderingEngine::GetInstance();
			const int count = static_cast<int>(m_shadowAreaList.size());
			for (int i = 0; i < count && i < renderingEngine.GetShadowAreaCount(); i++)
			{
				renderingEngine.SetShadowArea(i, m_shadowAreaList[i].areaMin, m_shadowAreaList[i].areaMax);
			}
		}


		void LightController::Update()
		{
			// 明かりの色と向き(JSON を書き換えると、ホットリロードで値がすぐ反映する)
			const LightParameter& light = m_parameter.Get();
			SceneLight& sceneLight = SceneLight::GetInstance();
			sceneLight.SetColor(light.directionColor);
			sceneLight.SetAmbient(light.ambient);
			sceneLight.SetDirection(light.directionDir);
		}


		void LightController::BuildShadowAreaList(const StageLayout& layout)
		{
			m_shadowAreaList.clear();

			const int floorCount = layout.GetFloorCount();
			for (int i = 0; i < floorCount; i++)
			{
				const StageFloor& floor = layout.GetFloor(i);
				if (floor.areaList.empty()) continue;

				// XZ は、その階の区画を全部囲む箱
				ShadowArea area;
				area.areaMin = floor.areaList[0].min;
				area.areaMax = floor.areaList[0].max;
				float top = area.areaMax.y;
				for (const StageArea& stageArea : floor.areaList)
				{
					ExpandXZ(area.areaMin, area.areaMax, stageArea.min, stageArea.max);
					if (stageArea.max.y > top) top = stageArea.max.y;
				}
				// 階段の上のキャラクターにも影が出るように、階段も XZ に入れる(高さは階の分だけ)
				for (const StageStairs& stairs : layout.GetStairsList())
				{
					ExpandXZ(area.areaMin, area.areaMax, stairs.min, stairs.max);
				}

				// 高さは、床の少し下から、上の階の床の少し下まで(一番上の階は、区画の天井まで)
				area.areaMin.y = floor.height - SHADOW_AREA_FLOOR_MARGIN;
				if (i + 1 < floorCount)
				{
					top = layout.GetFloor(i + 1).height - SHADOW_AREA_FLOOR_MARGIN;
				}
				area.areaMax.y = top;

				m_shadowAreaList.push_back(area);
			}
		}
	}
}
