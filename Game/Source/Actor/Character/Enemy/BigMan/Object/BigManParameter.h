/**
 * @file BigManParameter.h
 * @brief 大男の調整パラメーター
 */
#pragma once
#include "Source/Util/ParameterManager.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 大男の調整パラメーター
		 */
		struct BigManParameter
		{
			/** 移動速度 */
			float moveSpeed;
			/** 警戒時の速度 */
			float alertSpeed;
			/** ピントを合わせる位置(Y座標) */
			Vector3 focusPositionY;
			/** 初期位置 */
			Vector3 initialPosition;
			/** 初期スケール */
			Vector3 initialScale;


			BigManParameter()
				: moveSpeed(1.0f)
				, alertSpeed(1.0f)
				, focusPositionY(Vector3::Zero)
				, initialPosition(Vector3::Zero)
				, initialScale(Vector3::One)
			{}


			/**
			 * @brief JSONからパラメーターを読む
			 * @param file パラメーターファイル
			 */
			void Load(const util::ParameterFile& file)
			{
				moveSpeed = file.GetFloat("moveSpeed", moveSpeed);
				alertSpeed = file.GetFloat("alertSpeed", alertSpeed);
				focusPositionY = file.GetVector3("focusPositionY", focusPositionY);
				initialPosition = file.GetVector3("initialPosition", initialPosition);
				initialScale = file.GetVector3("initialScale", initialScale);
			}
		};
	}
}

