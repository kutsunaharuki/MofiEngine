/**
 * @file LightController.h
 * @brief ステージの明かり(ライトの設定と、影を作る範囲)
 */
#pragma once
#include "Source/Stage/Object/LightParameter.h"
#include "Source/Util/ParameterManager.h"
#include <vector>


namespace app
{
	namespace stage
	{
		class StageLayout;


		/**
		 * @brief ステージの明かり
		 * @details Light.json の色・環境光・向きを SceneLight に渡す。
		 *          影は階ごとに作る(1 階 = 0 番の範囲、2 階 = 1 番の範囲)。主人公がどこにいても、遠くの敵にも影が出る
		 */
		class LightController : public Noncopyable
		{
		public:
			LightController();
			~LightController();

			/**
			 * @brief 初期化
			 * @param layout ステージの構造(階ごとの影の範囲を作る)
			 */
			void Initialize(const StageLayout* layout);

			/**
			 * @brief 更新(明かりの設定を SceneLight に渡す)
			 */
			void Update();


		private:
			/**
			 * @brief 影を作る範囲(箱)
			 */
			struct ShadowArea
			{
				/** 箱の小さい方の角 */
				Vector3 areaMin;
				/** 箱の大きい方の角 */
				Vector3 areaMax;
			};

			/**
			 * @brief 階ごとの影の範囲を作る
			 * @param layout ステージの構造
			 */
			void BuildShadowAreaList(const StageLayout& layout);


		private:
			/** 明かりの設定(Light.json) */
			util::Parameter<LightParameter> m_parameter;
			/** 階ごとの影の範囲(0 = 1 階) */
			std::vector<ShadowArea> m_shadowAreaList;
		};
	}
}
