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



		bool CalcHead(const ModelRender& modelRender, const int headBoneNo, const Vector3& headForwardLocal, Vector3& outPosition, Vector3& outForward)
		{
			// 見つからない時は処理止める
			if (headBoneNo < 0) return false;

			// 頭のボーンのワールド行列
			const Matrix& head = modelRender.GetBone(headBoneNo)->GetWorldMatrix();
			outPosition.Set(head.m[3][0], head.m[3][1], head.m[3][2]);
			outForward = headForwardLocal;
			head.Apply3x3(outForward);
			outForward.Normalize();

			return true;
		}


		ShotMeasure MeasureShape(const Vector3& headPosition, const Vector3& headForward, const Vector3& footPosition, Camera& camera)
		{
			ShotMeasure result;
			result.isMeasured = true;
			// 形だけを測るので「隠れていない」にしておく(隠れているかは MeasureShot がレイで上書きする)
			result.isFaceBlocked = false;

			Vector3 toCamera = camera.GetPosition() - headPosition;
			toCamera.Normalize();

			// 顔の向き: 長さ 1 の向き同士の内積 = cos。誤差で ±1 を超えると acos が NaN になるので Clampする
			const float faceDot = TClamp<float>::Clamp(headForward.Dot(toCamera), -1.0f, 1.0f);
			result.faceAngleDeg = Math::RadToDeg(std::acos(faceDot));


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
			util::CalcScreenPosition(camera, footPosition, footScreen);
			const float screenWidth = static_cast<float>(UI_SPACE_WIDTH);
			const float screenHeight = static_cast<float>(UI_SPACE_HEIGHT);
			result.size = (result.faceScreenPosition.y - footScreen.y) / screenHeight;

			const float centerX = (result.faceScreenPosition.x + footScreen.x) * HALF;
			const float centerY = (result.faceScreenPosition.y + footScreen.y) * HALF;
			result.position.x = centerX / screenWidth + HALF;
			result.position.y = HALF - centerY / screenHeight;
			return result;
		}


		ShotMeasure MeasureShot(actor::EnemyBase& enemy, Camera& camera, const btCollisionObject* photographer)
		{
			// 頭の位置と向き
			Vector3 headPosition;
			Vector3 headForward;
			if (!enemy.GetHead(headPosition, headForward)) return ShotMeasure();
			
			// 形(角度・大きさ・位置)
			ShotMeasure result = MeasureShape(headPosition, headForward, enemy.GetTransform().GetWorldPosition(), camera);

			// 隠れていないか(悪党だけ): カメラ → 顔にレイを当てる。撮影者の体は無視して悪党に当たったら隠れていない
			const util::RayHit hit = util::RayCastClosest(camera.GetPosition(), headPosition, photographer);
			result.isFaceBlocked = hit.isHit && hit.hitObject != enemy.GetSubjectCollisionObject();
			return result;
		}
	}
}