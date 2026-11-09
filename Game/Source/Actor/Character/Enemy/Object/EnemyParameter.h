/**
 * @file EnemyParameter.h
 * @brief 敵の調整パラメーター
 * @details 項目は大男もねずみ男も同じ。値は種類ごとの JSON に書く
 *          (BigMan/BigManParameter.json, RatMan/RatManParameter.json)
 */
#pragma once
#include "Source/Util/ParameterManager.h"
#include "Source/Actor/Character/Enemy/EnemyDestinationMode.h"
#include "Source/Actor/Character/Enemy/EnemyAlertMode.h"
#include "Source/AI/Vision.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 敵の調整パラメーター
		 */
		struct EnemyParameter
		{
			/** 歩く速さ */
			float moveSpeed;
			/** 警戒時の速さ */
			float alertSpeed;
			/** ピントを合わせる高さ(足元から) */
			float focusHeight;
			/** 初期位置 */
			Vector3 initialPosition;
			/** 初期スケール */
			Vector3 initialScale;
			/** 目的地モードの設定 */
			DestinationModeSetting destinationMode;
			/** 警戒モードの設定 */
			AlertModeSetting alertMode;
			/** ビジョン */
			ai::VisionSetting vision;


			EnemyParameter()
				: moveSpeed(150.0f)
				, alertSpeed(200.0f)
				, focusHeight(80.0f)
				, initialPosition(Vector3::Zero)
				, initialScale(Vector3::One)
				, destinationMode(DestinationModeSetting())
				, alertMode(AlertModeSetting())
				, vision(ai::VisionSetting())
			{}


			/**
			 * @brief JSONからパラメーターを読む
			 * @param file パラメーターファイル
			 * @details JSON に書いていない項目は、今の値(初期値)のまま
			 */
			void Read(const util::ParameterFile& file)
			{
				// 基本パラメーター
				moveSpeed = file.GetFloat("moveSpeed", moveSpeed);
				alertSpeed = file.GetFloat("alertSpeed", alertSpeed);
				focusHeight = file.GetFloat("focusHeight", focusHeight);
				initialPosition = file.GetVector3("initialPosition", initialPosition);
				initialScale = file.GetVector3("initialScale", initialScale);
				
				// 目的地モード
				destinationMode.wanderAmplitudeDeg = file.GetFloat("wanderAmplitudeDeg", destinationMode.wanderAmplitudeDeg);
				destinationMode.wanderFrequency = file.GetFloat("wanderFrequency", destinationMode.wanderFrequency);
				destinationMode.stealTime = file.GetFloat("stealTime", destinationMode.stealTime);
				destinationMode.lookAroundTime = file.GetFloat("lookAroundTime", destinationMode.lookAroundTime);
				destinationMode.detourCheckInterval = file.GetFloat("detourCheckInterval", destinationMode.detourCheckInterval);
				destinationMode.detourChance = file.GetFloat("detourChance", destinationMode.detourChance);
				destinationMode.detourRadiusCell = file.GetInt("detourRadiusCell", destinationMode.detourRadiusCell);
				destinationMode.maxDetourPerTarget = file.GetInt("maxDetourPerTarget", destinationMode.maxDetourPerTarget);
				destinationMode.retryInterval = file.GetFloat("retryInterval", destinationMode.retryInterval);

				// 警戒モード
				alertMode.alertDuration = file.GetFloat("alertDuration", alertMode.alertDuration);
				alertMode.lookAroundTime = file.GetFloat("alertLookAroundTime", alertMode.lookAroundTime);
				alertMode.retryInterval = file.GetFloat("alertRetryInterval", alertMode.retryInterval);


				// 敵ビジョン(視線)
				vision.viewDistance = file.GetFloat("visionViewDistance", vision.viewDistance);
				vision.viewAngle = file.GetFloat("visionViewAngle", vision.viewAngle);
				vision.eyeHeight = file.GetFloat("visionEyeHeight", vision.eyeHeight);
				vision.targetHeight = file.GetFloat("visionTargetHeight", vision.targetHeight);
				vision.noticeTime = file.GetFloat("visionNoticeTime", vision.noticeTime);
			}
		};
	}
}
