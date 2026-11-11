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
			float walkSpeed;
			/** 走る速さ */
			float runSpeed;
			/** 向きを変える速さ(大きいほど早く向く) */
			float rotateSpeed;
			/** 重力 */
			float gravity;
			/** ファインダーを覗いている時の歩行速度 */
			float photoWalkSpeed;
			/** 隠れられる物を探す距離(足元の中心から) */
			float hideCheckDistance;
			/** 小さい物を探すレイの高さ(膝くらい) */
			float hideLowRayHeight;
			/** 高い物を探すレイの高さ(頭くらい) */
			float hideHighRayHeight;
			/** 初期位置 */
			Vector3 initialPosition;
			/** 更新されるスケール */
			Vector3 updateScale;
			/** 走っている時の目立ちやすさの倍率 */
			float runNoticeScale;


			BoyParameter()
				: walkSpeed(150.0f)
				, runSpeed(400.0f)
				, rotateSpeed(15.0f)
				, gravity(1500.0f)
				, photoWalkSpeed(100.0f)
				, hideCheckDistance(60.0f)
				, hideLowRayHeight(30.0f)
				, hideHighRayHeight(110.0f)
				, initialPosition(Vector3::Zero)
				, updateScale(Vector3::One)
				, runNoticeScale(1.0f)
			{}


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
				hideCheckDistance = file.GetFloat("hideCheckDistance", hideCheckDistance);
				hideLowRayHeight = file.GetFloat("hideLowRayHeight", hideLowRayHeight);
				hideHighRayHeight = file.GetFloat("hideHighRayHeight", hideHighRayHeight);
				initialPosition = file.GetVector3("initialPosition", initialPosition);
				updateScale = file.GetVector3("updateScale", updateScale);
				runNoticeScale = file.GetFloat("runNoticeScale", runNoticeScale);
			}
		};
	}
}