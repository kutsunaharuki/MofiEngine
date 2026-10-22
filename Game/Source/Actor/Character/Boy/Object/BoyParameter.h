/**
 * @file BoyParameter.h
 * @brief 主人公の調整パラメータ
 */
#pragma once
#include "Source/Util/ParameterManager.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 主人公の調整パラメーター
		 */
		struct BoyParameter
		{
			/** 歩く速さ */
			float walkSpeed = 150.0f;
			/** 走る速さ */
			float runSpeed = 400.0f;
			/** 向きを変える速さ(大きいほど早く向く) */
			float rotateSpeed = 15.0f;
			/** 重力 */
			float gravity = 1500.0f;
			/** ファインダーを覗いている時の歩行速度 */
			float photoWalkSpeed = 100.0f;
			/** 初期位置 */
			Vector3 initialPosition = Vector3::Zero;
			/** 初期スケール */
			Vector3 initialScale = Vector3::One;

			/**
			 * @brief JSON から読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
			{
				walkSpeed = file.GetFloat("walkSpeed", walkSpeed);
				runSpeed = file.GetFloat("runSpeed", runSpeed);
				rotateSpeed = file.GetFloat("rotateSpeed", rotateSpeed);
				gravity = file.GetFloat("gravity", gravity);
				photoWalkSpeed = file.GetFloat("photoWalkSpeed", photoWalkSpeed);
				initialPosition = file.GetVector3("initialPosition", initialPosition);
				initialScale = file.GetVector3("initialScale", initialScale);
			}
		};
	}
}