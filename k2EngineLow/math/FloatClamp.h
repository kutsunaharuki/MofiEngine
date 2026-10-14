/**
 * @file FloatClamp.h
 * @brief float型専用のClampクラス
 */
#pragma once

namespace nsK2EngineLow
{
	class FloatClamp
	{
	public:
		/**
		 * @brief float型の値を指定された範囲内にクランプする
		 * @param value クランプする値
		 * @param low 最小値
		 * @param high 最大値
		 * @return クランプされた値
		 */
		constexpr static const float Clamp(const float value, const float low, const float high)
		{
			if (value < low) return low;
			if (value > high) return high;
			return value;
		}
	};
}