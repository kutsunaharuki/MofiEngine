/**
 * @file MissionManager.h
 * @brief ミッションを一元管理する
 * @details MissionManager はシングルトンにしない。
 * 理由は、ミッションはプレイの間だけ意味があるのと、
 * 使う場所が限られるのと誰が書き換えているか分かりやすいからでもある。
 */
#pragma once
#include "Source/Mission/MissionData.h"
#include "Source/Mission/MissionJudge.h"
#include <unordered_map>
#include <vector>


namespace app
{
	namespace mission
	{
		/**
		 * @brief ミッションを管理
		 */
		class MissionManager : public Noncopyable
		{
		public:
			MissionManager();
			~MissionManager();

			/**
			 * @brief 初期化処理
			 * @param filePath ミッションの Json ファイルのパス
			 */
			void Initialize(const char* filePath);

			/**
			 * @brief 撮影した時に呼ぶ
			 * @param shot 撮った瞬間の情報
			 * @return どれかのミッションの枚数が増えたら true(UI を更新する合図)
			 * @details 同じ盗みを何枚撮っても、1枚として数える
			 */
			bool OnShot(const ShotInfo& shot);

			/**
			 * @brief ミッションを終わらせる
			 * @details 達成していないミッションを失敗にする(インゲームを抜ける時に呼ぶ)
			 */
			void Finish();


		public:
			/**
			 * @brief ミッションの一覧を取得
			 * @return ミッションの一覧
			 * @details ミッションの一覧は Jsonの missions の配列の中身を読み込んだもの。
			 */
			const std::vector<MissionData>& GetMissionList() const { return m_missionList; }

			/**
			 * @brief 状態ごとのミッションの一覧を取得
			 * @param state 取り出したい状態(達成・失敗など)
			 * @return その状態のミッションの一覧(コピー。リザルトに渡せる)
			 */
			std::vector<MissionData> GetMissionList(const MissionState state) const;

			/**
			 * @brief 全てのミッションを達成したか
			 * @return 全て達成していれば true(ミッションが無ければ false)
			 */
			bool IsAllAchieved() const;


		private:
			/**
			 * @brief "BigMan" などの文字を敵の種類にする
			 * @param name 敵の種類("BigMan")...
			 */
			static actor::EnemyBase::EnemyType ToEnemyType(const std::string& name);


		private:
			/** ミッションごとの状態 */
			std::vector<MissionData> m_missionList;
			/** 敵ごとに、最後に数えた盗みの番号(同じ盗みを二重に数えないため) */
			std::unordered_map<const actor::EnemyBase*, int> m_countedStealList;
		};
	}
}


