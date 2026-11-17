/**
 * @file ShotMeasure.cpp
 * @brief 撮った瞬間に悪党がどう写っているかを測る
 */
#include "stdafx.h"
#include "ShotMeasure.h"
#include "Source/Actor/Character/Enemy/EnemyBase.h"
#include "Source/Util/RayCast.h"
#include "Source/Util/ScreenPosition.h"
#include <cmath>


namespace app
{
	namespace mission
	{
		namespace
		{
			/** 半分 */
			constexpr float HALF = 0.5f;
		}



		ShotMeasure MeasureShot(actor::EnemyBase& enemy, Camera& camera, const btCollisionObject* photographer)
		{
			ShotMeasure result;

			// 頭の位置と向き
			Vector3 headPosition;
			Vector3 headForward;

			if (!enemy.GetHead(headPosition, headForward)) return result;
			result.isMeasured = true;

			const Vector3& eye = camera.GetPosition();
			Vector3 toCamera = eye - headPosition;
			toCamera.Normalize();

			// 顔の向き: 長さ 1 の向き同士の内積 = cos。誤差で ±1 を超えると acos が NaN になるので Clampする
			const float faceDot = TClamp<float>::Clamp(headForward.Dot(toCamera), -1.0f, 1.0f);
			result.faceAngleDeg = Math::RadToDeg(std::acos(faceDot));


			// 隠れていないか: カメラ → 顔にレイを当てる。撮影者の体は無視して悪党に当たったら隠れていない
			const util::RayHit hit = util::RayCastClosest(eye, headPosition, photographer);
			result.isFaceBlocked = hit.isHit && hit.hitObject != enemy.GetSubjectCollisionObject();


			// 顔の画面の位置(枠の中かはミッションごとの余白で MissionJudge で決める)
			result.isFaceInFront = util::CalcScreenPosition(camera, headPosition, result.faceScreenPosition);


			// 左右: 水平にした「顔の正面」と「顔 → カメラ」の符号付の角度
			// ※ 内積だけでは左右が分からないので、外積の Y で符号を出して atan2 で -180 ～ 180 にする
			Vector3 forwardFlat(headForward.x, 0.0f, headForward.z);
			Vector3 toCameraFlat(toCamera.x, 0.0f, toCamera.z);
			forwardFlat.Normalize();
			toCameraFlat.Normalize();

			const float flatCross = forwardFlat.z * toCameraFlat.x - forwardFlat.x * toCameraFlat.z;
			result.yawDeg = Math::RadToDeg(std::atan2(flatCross, forwardFlat.Dot(toCameraFlat)));


			// 上下: 「顔 → カメラ」の高さの角度
			result.pitchDeg = Math::RadToDeg(std::asin(TClamp<float>::Clamp(toCamera.y, -1.0f, 1.0f)));


			// 大きさと位置
			Vector2 footScreen;
			util::CalcScreenPosition(camera, enemy.GetTransform().GetWorldPosition(), footScreen);
			const float screenWidth = static_cast<float>(UI_SPACE_WIDTH);
			const float screenHeight = static_cast<float>(UI_SPACE_HEIGHT);
			result.size = (result.faceScreenPosition.y - footScreen.y) / screenHeight;

			const float centerX = (result.faceScreenPosition.x + footScreen.x) * HALF;
			const float centerY = (result.faceScreenPosition.y + footScreen.y) * HALF;
			result.position.x = centerX / screenWidth + HALF;
			result.position.y = HALF - centerY / screenHeight;
			return result;
		}
	}
}