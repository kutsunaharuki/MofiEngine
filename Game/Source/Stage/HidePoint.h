/**
 * @file HidePoint.h
 * @brief 隠れ場所の目印
 * @details ステージエディタで置いて、ステージのJSONに保存する。
 *          警戒した敵が、この中から行き先を選ぶ
 */
#pragma once


namespace app
{
	namespace stage
	{
		/**
		 * @brief 隠れ場所の目印
		 */
		struct HidePoint
		{
			/** 位置 */
			Vector3 position;
			/** 選ばれやすさ(値が大きいほど選ばれる確率が高い) */
			float weight;

			// コンストラクタ
			HidePoint()
				: position(Vector3::Zero)
				, weight(1.0f)
			{}
		};
	}
}

