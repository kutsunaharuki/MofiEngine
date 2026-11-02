/**
 * @file UIAnimationTypes.h
 * @brief UIで扱うアニメーションキーの定義場所
 */
#pragma once
#include "Source/Util/CRC32.h"
#include <cstdint>


namespace app
{
	namespace ui
	{
		namespace animKey
		{
			/** 開く */
			constexpr uint32_t OPEN = Hash32("Open");
			/** 閉じる */
			constexpr uint32_t CLOSE = Hash32("Close");
			/** 選択中 */
			constexpr uint32_t SELECT = Hash32("Select");
			/** 決定 */
			constexpr uint32_t DECIDE = Hash32("Decide");
		}
	}
}
