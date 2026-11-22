/**
 * @file VisitorParameter.h
 * @brief 訪問者(一般客)の調整パラメーター
 */
#pragma once
#include "Source/Util/ParameterManager.h"


namespace app
{
	namespace actor
	{
		/** 訪問者(一般客)の 1グループのパラメーターの JSON パス */
		constexpr const char* VISITOR_GROUP_PARAMETER_JSON_PATH = "Assets/parameter/Actor/Character/Visitor/VisitorGroup.json";

		/**
		 * @brief 訪問者(一般客)の 1グループの調整パラメーター
		 */
		struct VisitorGroupParameter
		{
			/** 1グループの人数 */
			int memberCount;
			/** グループを出す位置(試し用) */
			Vector3 groupPosition;
			/** 親からメンバーまでの最大の距離(円の半径) */
			float groupRadius;
			/** 散らばり具合(親からの距離のばらつき。大きいほど、近い人と遠い人にばらける) */
			float scatter;
			/** メンバーの最小間隔 */
			float minSpacing;
			/** 美術品の前で立ち止まって見る時間の最小(秒) */
			float lookTimeMin;
			/** 美術品の前で立ち止まって見る時間の最大(秒) */
			float lookTimeMax;
			/** 誰が話すかを選び直す間隔(秒) */
			float talkSwitchTime;
			/** 1人ひとりが話す確率(0 〜 1) */
			float talkRate;
			/** ベビーカーを、親の前にどれだけ離して置くか */
			float strollerDistance;
			/** ベビーカーの大きさ */
			Vector3 strollerScale;
			/** ベビーカーの向きを直す角度(度。モデルが横や後ろを向いている時に使う) */
			float strollerYawDeg;
			/** 赤ちゃんを、ベビーカーからどれだけずらして置くか(x:右, y:上, z:前。親の向きで回す) */
			Vector3 babyOffset;
			/** 赤ちゃんの大きさ */
			Vector3 babyScale;
			/** 赤ちゃんの向きを直す角度(度。モデルが横や後ろを向いている時に使う) */
			float babyYawDeg;


			VisitorGroupParameter()
				: memberCount(8)
				, groupPosition(Vector3::Zero)
				, groupRadius(100.0f)
				, scatter(20.0f)
				, minSpacing(40.0f)
				, lookTimeMin(4.0f)
				, lookTimeMax(8.0f)
				, talkSwitchTime(5.0f)
				, talkRate(0.4f)
				, strollerDistance(50.0f)
				, strollerScale(Vector3::One)
				, strollerYawDeg(0.0f)
				, babyOffset(Vector3::Zero)
				, babyScale(Vector3::One)
				, babyYawDeg(0.0f)
			{}


			/**
			 * @brief JSON からパラメーターを読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
			{
				memberCount = file.GetInt("memberCount", memberCount);
				groupPosition = file.GetVector3("groupPosition", groupPosition);
				groupRadius = file.GetFloat("groupRadius", groupRadius);
				scatter = file.GetFloat("scatter", scatter);
				minSpacing = file.GetFloat("minSpacing", minSpacing);
				lookTimeMin = file.GetFloat("lookTimeMin", lookTimeMin);
				lookTimeMax = file.GetFloat("lookTimeMax", lookTimeMax);
				talkSwitchTime = file.GetFloat("talkSwitchTime", talkSwitchTime);
				talkRate = file.GetFloat("talkRate", talkRate);
				strollerDistance = file.GetFloat("strollerDistance", strollerDistance);
				strollerScale = file.GetVector3("strollerScale", strollerScale);
				strollerYawDeg = file.GetFloat("strollerYawDeg", strollerYawDeg);
				babyOffset = file.GetVector3("babyOffset", babyOffset);
				babyScale = file.GetVector3("babyScale", babyScale);
				babyYawDeg = file.GetFloat("babyYawDeg", babyYawDeg);
			}
		};


		/**
		 * @brief 訪問者(一般客)の基本的パラメーター
		 */
		struct VisitorParameter
		{
			/** 歩く速度 */
			float walkSpeed;
			/** 歩きながら話す時の速度(歩幅が違うと足が滑るので、歩きと分ける) */
			float walkTalkSpeed;
			/** スケール */
			Vector3 scale;


			VisitorParameter()
				: walkSpeed(100.0f)
				, walkTalkSpeed(80.0f)
				, scale(Vector3::One)
			{}


			/**
			 * @brief JSON からパラメーターを読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
			{
				walkSpeed = file.GetFloat("walkSpeed", walkSpeed);
				walkTalkSpeed = file.GetFloat("walkTalkSpeed", walkTalkSpeed);
				scale = file.GetVector3("scale", scale);
			}
		};
	}
}
