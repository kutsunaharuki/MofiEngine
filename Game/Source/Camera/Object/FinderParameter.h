/**
 * @file FinderParameter.h
 * @brief ファインダーのパラメーターを管理する
 */
#pragma once
#include "Source/Util/ParameterManager.h"


namespace app
{
	namespace camera
	{
		/**
		 * @brief ファインダーのパラメーター
		 */
		struct FinderParameter
		{
			/** ズームの速さ(1.0 がエンジンの標準の速さ) */
			float zoomSpeed = 1.0f;
			/** ピントを動かす速さ(1.0 がエンジンの標準の速さ) */
			float focusSpeed = 1.0f;
			/** 撮ってから、次に撮れるようになるまでの時間(秒) */
			float shotCoolTime = 1.0f;
			/** ピントが合う速さ */
			float autoFocusSpeed = 1.0f;
			/** 画面の真ん中とみなす角度 */
			float autoFocusAngle = 1.0f;
			/** これより遠い被写体は無視する距離 */
			float autoFocusMaxDistance = 100.0f;


			/**
			 * @brief JSON から読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
			{
				zoomSpeed = file.GetFloat("zoomSpeed", zoomSpeed);
				focusSpeed = file.GetFloat("focusSpeed", focusSpeed);
				shotCoolTime = file.GetFloat("shotCoolTime", shotCoolTime);
				autoFocusSpeed = file.GetFloat("autoFocusSpeed", autoFocusSpeed);
				autoFocusAngle = file.GetFloat("autoFocusAngle", autoFocusAngle);
				autoFocusMaxDistance = file.GetFloat("autoFocusMaxDistance", autoFocusMaxDistance);
			}
		};
	}
}
