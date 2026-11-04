/**
 * @file WeightedRandom.h
 * @brief 重み付きくじ引き
 */
#pragma once
#include <vector>
#include <random>


namespace app
{
	namespace ai
	{
		/**
		 * @brief 重みに比例した確率で番号を 1 つ選ぶ
		 * @param weightList 候補ごとの重み(0 以下の候補は選ばない)
		 * @param excludeIndex 選ばない番号(今いる場所など)。無いときは -1
		 * @param random 乱数
		 * @return 選んだ番号。選べる候補がないときは -1
		 */
		inline int PickWeighted(const std::vector<float>& weightList, int excludeIndex, std::mt19937& random)
		{
			// 選べる候補の重みを計算
			float total = 0.0f;
			const int count = static_cast<int>(weightList.size());
			for (int i = 0; i < count; i++)
			{
				// 除外する番号は数えない
				if (i == excludeIndex) continue;
				// 重みが 0 以下は選ばない
				if (weightList[i] <= 0.0f) continue;

				total += weightList[i];
			}

			// 選べる候補がないときは -1 を返す
			if (total <= 0.0f) return -1;

			// 0 ～ total(合計) の乱数を引く
			// ※std::uniform_int_distribution<> では、
			// 整数の範囲しか指定できないので、少数の重みには使えないため
			// sttd::uniform_real_distribution<> を使用する
			// range (0.0f, total) は0.0以上、 total未満の結果になる
			std::uniform_real_distribution<float> range(0.0f, total);
			float r = range(random);

			// 前から順に重みを引いていく。r が重みより小さくなった候補が採用される
			int last = -1;
			for (int i = 0; i < count; i++)
			{
				// 一番上と同じように候補を飛ばす
				if (i == excludeIndex) continue;
				if (weightList[i] <= 0.0f) continue;

				last = i;
				if (r < weightList[i]) return i;
				r -=weightList[i];
			}

			// 少数の誤差で最後まで当たらなかったときは、最後に見た候補を返す
			return last;
		}
	}
}