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
			/** 頭のボーンの名前 */
			std::string headBone;
			/** 頭のボーンの中で「顔の前」の向き */
			Vector3 headForwardLocal;
			/** 左手のボーンの名前(見本の絵で、絵画を持たせる所) */
			std::string leftHandBone;
			/** 右手のボーンの名前 */
			std::string rightHandBone;

			// 主人公を見つけた時
			/** 見つけてから走り出すまでの時間(秒。「！」を出して、びっくりして止まる) */
			float foundReactTime;
			/** 主人公の方へ走る速さ */
			float foundRunSpeed;
			/** 走る時のアニメーションの速さ(倍。走るアニメーションが無いので、歩くアニメーションを速く回す) */
			float foundRunAnimationSpeed;
			/** 主人公にこれより近づいたら止まる(つかまえた) */
			float foundCatchDistance;
			/** 主人公がこの時間(秒)見えなかったら見失う(最後に見た所を調べに行く) */
			float loseSightTime;
			/** 追いかけている間に、主人公までの道順を求め直す間かく(秒。短いほどよく追うが重い) */
			float foundRepathInterval;
			/** 前に見失った所からこの距離の中で、また見失ったら、まわりの隠れ場所を探し回る */
			float sameLostDistance;

			// シャッター音
			/** シャッター音が聞こえる距離(撮った所から、これより遠いと聞こえない) */
			float hearDistance;
			/** 調べに来ている間(音がした所・見失った所)と、気を付けている間の、気づく時間の倍率(小さいほど早く気づく) */
			float investigateNoticeRate;

			// 柱からのぞいて撮られた時
			/** のぞいて撮られた柱から、この距離の中にいる悪党は気を付けるようになる */
			float pillarWaryDistance;
			/** 気を付けている時間(秒) */
			float waryTime;


			EnemyParameter()
				: moveSpeed(150.0f)
				, alertSpeed(200.0f)
				, focusHeight(80.0f)
				, initialPosition(Vector3::Zero)
				, initialScale(Vector3::One)
				, destinationMode(DestinationModeSetting())
				, alertMode(AlertModeSetting())
				, vision(ai::VisionSetting())
				, headForwardLocal(Vector3(0.0f,0.0f,1.0f))
				, foundReactTime(0.8f)
				, foundRunSpeed(450.0f)
				, foundRunAnimationSpeed(1.8f)
				, foundCatchDistance(120.0f)
				, loseSightTime(1.0f)
				, foundRepathInterval(0.5f)
				, sameLostDistance(300.0f)
				, hearDistance(800.0f)
				, investigateNoticeRate(0.4f)
				, pillarWaryDistance(600.0f)
				, waryTime(30.0f)
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
				destinationMode.stealGrabTime = file.GetFloat("stealGrabTime", destinationMode.stealGrabTime);
				destinationMode.stealTurnTime = file.GetFloat("stealTurnTime", destinationMode.stealTurnTime);
				destinationMode.stealShowTime = file.GetFloat("stealShowTime", destinationMode.stealShowTime);
				destinationMode.lookAroundTime = file.GetFloat("lookAroundTime", destinationMode.lookAroundTime);
				destinationMode.detourCheckInterval = file.GetFloat("detourCheckInterval", destinationMode.detourCheckInterval);
				destinationMode.detourChance = file.GetFloat("detourChance", destinationMode.detourChance);
				destinationMode.detourRadiusCell = file.GetInt("detourRadiusCell", destinationMode.detourRadiusCell);
				destinationMode.maxDetourPerTarget = file.GetInt("maxDetourPerTarget", destinationMode.maxDetourPerTarget);
				destinationMode.retryInterval = file.GetFloat("retryInterval", destinationMode.retryInterval);

				// 警戒モード
				alertMode.lookAroundTime = file.GetFloat("alertLookAroundTime", alertMode.lookAroundTime);
				alertMode.retryInterval = file.GetFloat("alertRetryInterval", alertMode.retryInterval);
				alertMode.searchRadius = file.GetFloat("alertSearchRadius", alertMode.searchRadius);
				alertMode.searchPointCount = static_cast<int>(file.GetFloat("alertSearchPointCount", static_cast<float>(alertMode.searchPointCount)));


				// 敵ビジョン(視線)
				vision.viewDistance = file.GetFloat("visionViewDistance", vision.viewDistance);
				vision.frontAngle = file.GetFloat("visionFrontAngle", vision.frontAngle);
				vision.sideAngle = file.GetFloat("visionSideAngle", vision.sideAngle);
				vision.sideRate = file.GetFloat("visionSideRate", vision.sideRate);
				vision.ringCount = static_cast<int>(file.GetFloat("visionRingCount", static_cast<float>(vision.ringCount)));
				vision.farRate = file.GetFloat("visionFarRate", vision.farRate);
				vision.eyeHeight = file.GetFloat("visionEyeHeight", vision.eyeHeight);
				vision.targetHeight = file.GetFloat("visionTargetHeight", vision.targetHeight);
				vision.noticeTime = file.GetFloat("visionNoticeTime", vision.noticeTime);

				// 頭(顔)のボーンの名前と向き
				headBone = file.GetString("headBone", headBone);
				headForwardLocal = file.GetVector3("headForwardLocal", headForwardLocal);

				// 手のボーンの名前(見本の絵で、絵画を持たせる)
				leftHandBone = file.GetString("leftHandBone", leftHandBone);
				rightHandBone = file.GetString("rightHandBone", rightHandBone);

				// 主人公を見つけた時
				foundReactTime = file.GetFloat("foundReactTime", foundReactTime);
				foundRunSpeed = file.GetFloat("foundRunSpeed", foundRunSpeed);
				foundRunAnimationSpeed = file.GetFloat("foundRunAnimationSpeed", foundRunAnimationSpeed);
				foundCatchDistance = file.GetFloat("foundCatchDistance", foundCatchDistance);
				loseSightTime = file.GetFloat("loseSightTime", loseSightTime);
				foundRepathInterval = file.GetFloat("foundRepathInterval", foundRepathInterval);
				sameLostDistance = file.GetFloat("sameLostDistance", sameLostDistance);

				// シャッター音
				hearDistance = file.GetFloat("hearDistance", hearDistance);
				investigateNoticeRate = file.GetFloat("investigateNoticeRate", investigateNoticeRate);

				// 柱からのぞいて撮られた時
				pillarWaryDistance = file.GetFloat("pillarWaryDistance", pillarWaryDistance);
				waryTime = file.GetFloat("waryTime", waryTime);
			}
		};
	}
}
