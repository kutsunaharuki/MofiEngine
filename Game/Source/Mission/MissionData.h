/**
 * @file MissionData.h
 * @brief ミッションに関するデータの構造体
 */
#pragma once
#include "Source/Actor/Character/Enemy/EnemyBase.h"


namespace app
{
	namespace mission
	{
		/**
		 * @brief 見本の写り方
		 */
		struct PhotoSample
		{
			/** 顔の正面から見てカメラが左右に何度ずれているか */
			float yawDeg;
			/** カメラがどれだけ上から見ているか */
			float pitchDeg;
			/** 悪党の大きさ(画面の高さの割合) */
			float size;
			/** 悪党の中心の画面の位置(左上:0,0、右下:1,1) */
			Vector2 position;
			/** 見本の絵で、盗むアニメーションを何秒目で止めるか */
			float poseTime;


			PhotoSample()
				: yawDeg(0.0f)
				, pitchDeg(0.0f)
				, size(0.0f)
				, position(Vector2(0.5f,0.5f))
				, poseTime(1.0f)
			{}
		};


		/**
		 * @brief 照合の許容値(比較してみた時に大体この範囲なら OK)
		 */
		struct MatchTolerance
		{
			/** 顔が見えているとみなす顔の向きの最大 */
			float faceAngleMax;
			/** 顔が枠の内側とみなす画面の端の余白(ピクセル。1920x1080) */
			float screenMargin;
			/** 左右のずれ(度数) */
			float yawDeg;
			/** 上下のずれ(度数) */
			float pitchDeg;
			/** 大きさのずれ(画面の高さの割合) */
			float size;
			/** 位置のずれ(0 ~ 1 の画面の中の距離) */
			float position;


			MatchTolerance()
				: faceAngleMax(70.0f)
				, screenMargin(40.0f)
				, yawDeg(20.0f)
				, pitchDeg(15.0f)
				, size(0.1f)
				, position(0.15f)
			{}
		};


		/** ミッションの進捗状態 */
		enum class MissionState : uint8_t
		{
			Progress,      // 進行中
			Achievement,   // 達成
			Failed         // 失敗
		};


		/**
		 * @brief ミッションデータ
		 */
		struct MissionData
		{
			// UI に出す文(UIText がwchar_t型だから)
			std::wstring text;
			// 誰を撮るか
			actor::EnemyBase::EnemyType target = actor::EnemyBase::EnemyType::Max;
			// 何枚撮るか
			int requiredCount = 0;
			// 今何枚か
			int currentCount = 0;
			// ミッションの進捗状態
			MissionState state = MissionState::Progress;
			/** 見本があるか(無いミッションは、角度・大きさ・位置を見ない) */
			bool hasSample = false;
			/** 見本の写り方 */
			PhotoSample sample;
			/** 照合の許容値 */
			MatchTolerance tolerance;
		};
	}
}