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
