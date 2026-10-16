/**
 * @file TClamp.h
 * @brief std::clampの代替クラス(汎用的Clamp)
 */
#pragma once


namespace nsK2EngineLow
{
	/**
	 * @brief Clampクラス(std::clampのヘルパー)
	 * @tparam T 値の型(float / int など、< と > で比べられる小さい型)
	 * @details std::clampがうまく使えないときの為に代替として用意したクラス。
	 *          使用例 : TClamp<float>::Clamp(value, 0.0f, 1.0f)
	 */
	template<class T>
	class TClamp
	{
	public:
		/**
		 * @brief 値を指定された範囲内に制限
		 * @param value 制限する値
		 * @param low 範囲の下限値
		 * @param high 範囲の上限値
		 * @return 制限された値
		 * @details 値渡し・値返しにしている。
		 *          参照で返すと、一時的な値(x * 2.0f など)を渡したときに、消えた値を指してしまうため
		 */
		static constexpr T Clamp(const T value, const T low, const T high)
		{
			if (value < low) return low;
			if (value > high) return high;
			return value;
		}
	};
}
