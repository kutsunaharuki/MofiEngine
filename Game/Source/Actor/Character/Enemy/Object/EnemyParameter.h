/**
 * @file EnemyParameter.h
 * @brief 敵の調整パラメーター
 */
#pragma once
#include "Source/Util/ParameterManager.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 敵の調整パラメーター
		 */
		struct EnemyParameter
		{
			/** 移動速度(3D) */
			Vector3 bigmanMoveSpeed;
			/** 警戒時の速度 */
			float alertSpeed;
			/** ピントを合わせる位置(Y座標) */
			Vector3 focusPositionY;
			/** 初期位置 */
			Vector3 initialPosition;
			/** 初期スケール */
			Vector3 initialScale;


			EnemyParameter()
				: bigmanMoveSpeed(Vector3::Zero)
				, alertSpeed(1.0f)
				, focusPositionY(Vector3::Zero)
				, initialPosition(Vector3::Zero)
				, initialScale(Vector3::One)
			{}


			/**
			 * @brief JSONからパラメーターを読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
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

