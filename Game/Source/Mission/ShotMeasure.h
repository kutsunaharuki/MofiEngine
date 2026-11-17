/**
 * @file ShotMeasure.h
 * @brief 撮った瞬間に悪党がどう写っているかを測る
 * @details 見本や許容値は知らない。判定は MissionJudge が行うようにする。
 */
#pragma once


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class EnemyBase;
	}

	
	namespace mission
	{
		/**
		 * @brief 悪党の写り方を測った値
		 */
		struct ShotMeasure
		{
			/** 頭のボーンが取れたか */
			bool isMeasured;
			/** 顔の正面と「顔 → カメラ」の向きの角度*/
			float faceAngleDeg;
			/** カメラと顔の間に物があるか */
			bool isFaceBlocked;
			/** 顔がカメラの前にあるか */
			bool isFaceInFront;
			/** 顔の画面の位置 */
			Vector2 faceScreenPosition;
			/** 顔の正面から見てカメラが左右に何度ずれているか */
			float yawDeg;
			/** カメラがどれだけ上から見ているか */
			float pitchDeg;
			/** 悪党の大きさ */
			float size;
			/** 悪党の中心の画面の位置(左上:0,0、右下:1,1) */
			Vector2 position;


			ShotMeasure()
				: isMeasured(false)
				, faceAngleDeg(180.0f)
				, isFaceBlocked(true)
				, isFaceInFront(false)
				, yawDeg(0.0f)
				, size(0.0f)
				, pitchDeg(0.0f)
			{}
		};


		/**
		 * @brief 悪党の写り方を測る
		 * @param enemy 悪党
		 * @param camera 撮るカメラ(今回はファインダー)
		 * @param photographer 撮る人の体(レイで無視する為)
		 * @return 測った値
		 */
		ShotMeasure MeasureShot(actor::EnemyBase& enemy, Camera& camera, const btCollisionObject* photographer);
	}
}

