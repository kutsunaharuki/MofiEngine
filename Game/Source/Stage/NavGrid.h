/**
 * @file NavGrid.h
 * @brief 経路探索用の地図クラス。(通れる / 通れないマス)
 * @details ステージを一定の大きさのマスに分割し、置物があるマスを通れないマスとして持つ。
 *          Stage を読み込んだ後に Build で作る。敵(犯人)は読むだけ
 */
#pragma once


namespace app
{
	namespace stage
	{
		class Stage;
	}
}


namespace nsk2EngineLow
{
	namespace nsAI
	{
		/**
		 * @brief 経路探索用の地図
		 */
		class NavGrid
		{
		public:
			explicit NavGrid();
			~NavGrid() = default;

			bool Build(const app::stage::Stage& stage);
		};
	}
}

