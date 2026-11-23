/**
 * @file ResultScene.cpp
 * @brief リザルトシーン
 */
#include "stdafx.h"
#include "ResultScene.h"
#include "TitleScene.h"
#include "InGameScene.h"
#include "Source/Manager/GameResult.h"
#include "Source/UI/ResultMenu/ResultMenu.h"
#include "Source/Mission/MissionText.h"
#include <algorithm>


namespace app
{
	namespace
	{
		/** リザルトの UI のレイアウトのパス */
		constexpr const char* RESULT_LAYOUT_PATH = "Assets/parameter/UI/Result/Result.json";

		/** タイトルの文字(どんな終わり方でも、いつも「リザルト」) */
		constexpr const wchar_t* TITLE_TEXT = L"リザルト";
		/** ミッションの行のマーク(● ○ はフォントに無いので 〇 を使う) */
		constexpr const wchar_t* MISSION_MARK = L"〇";
		/** 出す写真の枚数(新しい順) */
		constexpr int PHOTO_SHOW_COUNT = 8;
		/** 1 行に出す NG の項目の数(多いと写真の幅からはみ出す) */
		constexpr int FAILED_ITEM_PER_LINE = 2;
		/** NG の項目を出す最大の数(残りは「ほか」にする) */
		constexpr int FAILED_ITEM_MAX = 4;


		// 悪党の名前・照合の項目の名前は MissionText.h(ジャッジの UI と同じ名前を使う)
		using mission::GetEnemyName;
		using mission::GetMatchItemName;


		/**
		 * @brief ミッションの行を作る
		 * @param mission ミッション
		 * @return 行(ミッションの文の後ろに、枚数と結果を付ける)
		 */
		ui::ResultTextData MakeMissionRow(const mission::MissionData& mission)
		{
			const std::wstring text = mission.GetDisplayText();
			const bool isAchieved = mission.state == mission::MissionState::Achievement;

			ui::ResultTextData row;
			row.text = MISSION_MARK + text
				+ L"   " + std::to_wstring(mission.currentCount) + L" / " + std::to_wstring(mission.requiredCount)
				+ (isAchieved ? L"   達成" : L"   失敗");
			row.tone = isAchieved ? ui::ResultTone::Success : ui::ResultTone::Failed;
			return row;
		}


		/**
		 * @brief 写真の下に出す文を作る
		 * @param record 撮った記録
		 * @return 文(1 行目に結果、2 行目から NG の項目)
		 */
		std::wstring MakePhotoLabel(const mission::ShotRecord& record)
		{
			switch (record.status)
			{
			case mission::ShotStatus::NoSubject:      return L"失敗\n悪党が写っていない";
			case mission::ShotStatus::AlreadyCounted: return std::wstring(L"撮影済み  ") + GetEnemyName(record.subjectType) + L"\n同じ盗みはもう撮った";
			case mission::ShotStatus::NoMission:      return std::wstring(L"数えない  ") + GetEnemyName(record.subjectType) + L"\nミッションは終わっている";
			case mission::ShotStatus::Counted:        return std::wstring(L"成功  ") + GetEnemyName(record.subjectType);
			default: break;
			}

			// 失敗: NG の項目を並べる
			std::wstring text = std::wstring(L"失敗  ") + GetEnemyName(record.subjectType);
			std::vector<mission::MatchItem> failedItemList = record.result.GetFailedItems();

			// 狙いと違う悪党なら、ほかの項目は関係ないので、それだけ出す
			const bool isWrongTarget = std::find(failedItemList.begin(), failedItemList.end(), mission::MatchItem::Target) != failedItemList.end();
			if (isWrongTarget)
			{
				failedItemList = { mission::MatchItem::Target };
			}

			const int itemCount = static_cast<int>(failedItemList.size());
			const int showCount = (std::min)(itemCount, FAILED_ITEM_MAX);
			for (int i = 0; i < showCount; ++i)
			{
				// 1 行に FAILED_ITEM_PER_LINE 個ずつ並べる
				text += (i % FAILED_ITEM_PER_LINE == 0) ? L"\n" : L"・";
				text += GetMatchItemName(failedItemList[i]);
			}
			if (itemCount > showCount)
			{
				text += L" ほか";
			}
			return text;
		}


		/**
		 * @brief 撮った記録から、写真の見え方を決める
		 * @param status ミッションがどう数えたか
		 * @return 見え方
		 */
		ui::ResultTone ToTone(const mission::ShotStatus status)
		{
			switch (status)
			{
			case mission::ShotStatus::Counted:        return ui::ResultTone::Success;
			case mission::ShotStatus::AlreadyCounted:
			case mission::ShotStatus::NoMission:      return ui::ResultTone::Neutral;
			default:                                  return ui::ResultTone::Failed;
			}
		}


		/**
		 * @brief プレイの結果(Model)から、リザルトに出す値を作る
		 * @param gameResult プレイの結果
		 * @return 出す値
		 */
		ui::ResultViewData MakeViewData(GameResult& gameResult)
		{
			ui::ResultViewData viewData;

			// タイトル: どんな終わり方(全部達成・時間切れ・見つかった)でも、文字は「リザルト」のまま変えない
			// ※ 色だけは、全部達成したら成功の色、それ以外は失敗の色にする
			const bool isClear = gameResult.IsClear();
			viewData.title.text = TITLE_TEXT;
			viewData.title.tone = isClear ? ui::ResultTone::Success : ui::ResultTone::Failed;

			// ミッションの行
			for (const mission::MissionData& mission : gameResult.GetMissionList())
			{
				viewData.missionRowList.push_back(MakeMissionRow(mission));
			}

			// 写真: 新しい順に PHOTO_SHOW_COUNT 枚
			const std::vector<mission::ShotRecord>& recordList = gameResult.GetShotRecordList();
			const int recordCount = static_cast<int>(recordList.size());

			// 枚数: 撮った全部の枚数と、照合できた / できなかった枚数(撮影済み・ミッションが終わっている写真は、どちらにも入れない)
			viewData.count.shotCount = recordCount;
			for (const mission::ShotRecord& record : recordList)
			{
				if (record.status == mission::ShotStatus::Counted) ++viewData.count.matchCount;
				if (record.status == mission::ShotStatus::Failed || record.status == mission::ShotStatus::NoSubject) ++viewData.count.missCount;
			}

			const int showCount = (std::min)(recordCount, PHOTO_SHOW_COUNT);
			for (int i = 0; i < showCount; ++i)
			{
				const mission::ShotRecord& record = recordList[recordCount - 1 - i];

				ui::ResultPhotoData photo;
				// 上書きされた写真は nullptr(View は枠だけ出す)
				photo.texture = gameResult.GetAlbum().GetTexture(record.photoIndex);
				photo.label = MakePhotoLabel(record);
				photo.tone = ToTone(record.status);
				viewData.photoList.push_back(photo);
			}
			return viewData;
		}
	}


	ResultScene::ResultScene()
		: m_resultMenu(nullptr)
		, m_nextSceneId(INVALID_SCENE_ID)
		, m_selectIndex(static_cast<int>(ui::ResultMenu::SelectType::Retry))
	{}


	ResultScene::~ResultScene()
	{}


	bool ResultScene::Start()
	{
		// アルバムの UI(View)を作って、プレイの結果(Model)から作った値を渡す
		m_resultLayout.Initialize<ui::ResultMenu>(RESULT_LAYOUT_PATH);
		m_resultMenu = m_resultLayout.GetMenu<ui::ResultMenu>();
		if (m_resultMenu)
		{
			m_resultMenu->SetViewData(MakeViewData(*GameResult::GetInstance()));
			m_resultMenu->SetSelect(static_cast<ui::ResultMenu::SelectType>(m_selectIndex));
		}
		return true;
	}


	void ResultScene::Update()
	{
		// 入力を受ける(左右で選ぶ・A で決める)
		UpdateSelect();

		// アルバムの UI
		m_resultLayout.Update();
	}


	void ResultScene::Render(RenderContext & rc)
	{
		// アルバムを描く
		m_resultLayout.Render(rc);
	}


	bool ResultScene::RequestScene(uint32_t & id, float& waitTime)
	{
		// もう一度 / タイトルへ を決めたら移る
		if (m_nextSceneId != INVALID_SCENE_ID)
		{
			id = m_nextSceneId;
			waitTime = 1.0f;
			return true;
		}
		return false;
	}


	void ResultScene::UpdateSelect()
	{
		// 決めた後は動かさない
		if (m_nextSceneId != INVALID_SCENE_ID) return;

		constexpr int selectCount = static_cast<int>(ui::ResultMenu::SelectType::Max);
		const int prevIndex = m_selectIndex;
		if (g_pad[0]->IsTrigger(enButtonRight)) m_selectIndex = (m_selectIndex + 1) % selectCount;
		if (g_pad[0]->IsTrigger(enButtonLeft))  m_selectIndex = (m_selectIndex + selectCount - 1) % selectCount;

		// 変わった時だけ、View に選んでいる項目を伝える
		const ui::ResultMenu::SelectType selectType = static_cast<ui::ResultMenu::SelectType>(m_selectIndex);
		if (m_selectIndex != prevIndex && m_resultMenu)
		{
			m_resultMenu->SetSelect(selectType);
		}

		// A で決める(移るのは RequestScene)
		if (!g_pad[0]->IsTrigger(enButtonA)) return;
		// 決めたボタンを押した形にする(見た目は View が変える)
		if (m_resultMenu) m_resultMenu->PressSelect();
		switch (selectType)
		{
		case ui::ResultMenu::SelectType::Retry: m_nextSceneId = InGameScene::ID(); break;
		case ui::ResultMenu::SelectType::Title: m_nextSceneId = TitleScene::ID();  break;
		default: break;
		}
	}
}
