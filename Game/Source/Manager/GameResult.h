/**
 * @file GameResult.h
 * @brief 1 回のプレイの結果(インゲーム → リザルトに渡す)
 * @details シーンは移るたびに作り直されるので、結果はシーンの外(ここ)に置く。
 *          Game が作って、Game と一緒に消える
 */
#pragma once
#include "Source/Manager/MissionManager.h"
#include "Source/Mission/PhotoAlbum.h"
#include <vector>


namespace app
{
	/**
	 * @brief 1 回のプレイの結果
	 */
	class GameResult : public Noncopyable
	{
	public:
		/**
		 * @brief インスタンスを作成する
		 */
		static void CreateInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new GameResult;
			}
		}

		/**
		 * @brief インスタンスを取得する
		 */
		static GameResult* GetInstance()
		{
			return m_instance;
		}

		/**
		 * @brief インスタンスを破棄する
		 */
		static void DestroyInstance()
		{
			if (m_instance != nullptr)
			{
				delete m_instance;
				m_instance = nullptr;
			}
		}


	public:
		/**
		 * @brief 新しいプレイを始める(インゲームの最初に呼ぶ)
		 * @details 前のプレイの結果と写真を空にする
		 */
		void Reset();
		/**
		 * @brief プレイの結果を入れる(インゲームを抜ける時に呼ぶ)
		 * @param missionManager ミッションの管理(Finish を呼んだ後のもの)
		 */
		void SetResult(const mission::MissionManager& missionManager);


	public:
		/**
		 * @brief 写真のアルバムを取得
		 * @return アルバム
		 */
		mission::PhotoAlbum& GetAlbum() { return m_album; }
		/**
		 * @brief ミッションの一覧を取得
		 * @return 終わった時のミッションの一覧
		 */
		const std::vector<mission::MissionData>& GetMissionList() const { return m_missionList; }
		/**
		 * @brief 撮った写真の記録を取得
		 * @return 撮った順の記録
		 */
		const std::vector<mission::ShotRecord>& GetShotRecordList() const { return m_shotRecordList; }
		/**
		 * @brief 全てのミッションを達成したか
		 * @return 達成していれば true
		 */
		bool IsClear() const { return m_isClear; }


	private:
		GameResult();
		~GameResult();


	private:
		/** シングルトンインスタンス */
		static GameResult* m_instance;

		/** 撮った写真のアルバム */
		mission::PhotoAlbum m_album;
		/** 終わった時のミッションの一覧 */
		std::vector<mission::MissionData> m_missionList;
		/** 撮った写真の記録 */
		std::vector<mission::ShotRecord> m_shotRecordList;
		/** 全てのミッションを達成したか */
		bool m_isClear;
	};
}
