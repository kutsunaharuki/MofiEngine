/**
 * @file GameResult.cpp
 * @brief 1 回のプレイの結果(インゲーム → リザルトに渡す)
 */
#include "stdafx.h"
#include "GameResult.h"


namespace app
{
	GameResult* GameResult::m_instance = nullptr;


	GameResult::GameResult()
		: m_isClear(false)
	{}


	GameResult::~GameResult()
	{}


	void GameResult::Reset()
	{
		// 写真の描き先は、最初の 1 回だけ作る(写真用のRTと同じ大きさにする)
		m_album.Initialize(PhotoCapture::GetPhotoWidth(), PhotoCapture::GetPhotoHeight());
		m_album.Clear();

		m_missionList.clear();
		m_shotRecordList.clear();
		m_isClear = false;
	}


	void GameResult::SetResult(const mission::MissionManager& missionManager)
	{
		// インゲームが消えても読めるように、コピーして持つ
		m_missionList = missionManager.GetMissionList();
		m_shotRecordList = missionManager.GetShotRecordList();
		m_isClear = missionManager.IsAllAchieved();
	}
}
