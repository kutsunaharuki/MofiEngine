/**
 * @file MissionText.h
 * @brief ミッションの画面に出す名前(悪党の名前・照合の項目の名前)
 * @details リザルトとジャッジの UI の両方で使うので、1 か所にまとめる。
 *          文を作るのは Presenter(ResultScene・InGameScene)の役目で、View は渡された文を出すだけ
 */
#pragma once
#include "Source/Mission/MissionJudge.h"


namespace app
{
	namespace mission
	{
		/**
		 * @brief 悪党の名前
		 * @param type 悪党の種類
		 * @return 名前(知らない種類なら空)
		 */
		inline const wchar_t* GetEnemyName(const actor::EnemyBase::EnemyType type)
		{
			switch (type)
			{
			case actor::EnemyBase::EnemyType::BigMan: return L"大男";
			case actor::EnemyBase::EnemyType::RatMan: return L"ねずみ男";
			default:                                  return L"";
			}
		}


		/**
		 * @brief 照合の項目の名前(NG の時に出す)
		 * @param item 照合の項目
		 * @return 名前(知らない項目なら空)
		 */
		inline const wchar_t* GetMatchItemName(const MatchItem item)
		{
			switch (item)
			{
			case MatchItem::Target:      return L"狙いと違う悪党";
			case MatchItem::Stealing:    return L"盗んでいない";
			case MatchItem::Focus:       return L"ピント";
			case MatchItem::FaceAngle:   return L"顔の向き";
			case MatchItem::FaceBlocked: return L"顔が隠れている";
			case MatchItem::FaceInFrame: return L"顔が枠の外";
			case MatchItem::Yaw:         return L"左右の角度";
			case MatchItem::Pitch:       return L"上下の角度";
			case MatchItem::Size:        return L"大きさ";
			case MatchItem::Position:    return L"位置";
			default:                     return L"";
			}
		}


		/**
		 * @brief 照合の項目の名前(ジャッジのチェックリストに出す。合っている時の言い方)
		 * @param item 照合の項目
		 * @return 名前(知らない項目なら空)
		 */
		inline const wchar_t* GetMatchItemCheckName(const MatchItem item)
		{
			switch (item)
			{
			case MatchItem::Target:      return L"狙いの悪党";
			case MatchItem::Stealing:    return L"盗んでいる瞬間";
			case MatchItem::Focus:       return L"ピントが合っている";
			case MatchItem::FaceAngle:   return L"顔がこちらを向いている";
			case MatchItem::FaceBlocked: return L"顔が隠れていない";
			case MatchItem::FaceInFrame: return L"顔が画面の中";
			case MatchItem::Yaw:         return L"左右の角度";
			case MatchItem::Pitch:       return L"上下の角度";
			case MatchItem::Size:        return L"大きさ";
			case MatchItem::Position:    return L"位置";
			default:                     return L"";
			}
		}
	}
}
