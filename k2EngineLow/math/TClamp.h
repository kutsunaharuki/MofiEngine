/**
 * @file TClamp.h
 * @brief std::clampの代替クラス(汎用的Clamp)
 */
#pragma once


namespace nsK2EngineLow
{
	/**
	 * @brief Clampクラス(std::clampのヘルパー)
	 * @tparam T 値の型
	 * @details std::clampがうまく使えないときの為に代替として用意したクラス
	 */
	template<class T>
	class TClamp : public Noncopyable
	{
	public:
		/**
		 * @brief 値を指定された範囲内に制限
		 * @param value 制限する値
		 * @param low 範囲の下限値
		 * @param high 範囲の上限値
		 * @return 制限された値
		 */
		constexpr static const T& Clamp(const T& value, const T& low, const T& high)
		{
			if (value < low) return low;
			if (value > high) return high;
			return value;
		}
	};
}
