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
		 * @brief 1 枚の写真を、ミッションがどう数えたか
		 */
		enum class ShotStatus : uint8_t
		{
			NoSubject,		// 悪党が写っていない
			AlreadyCounted,	// 同じ盗みを、もう数えた
			NoMission,		// 進行中のミッションが無い
			Counted,		// ミッションに数えた(成功)
			Failed			// 照合して、どれかの項目が NG
		};


		/**
		 * @brief 撮った写真 1 枚の記録(リザルトのアルバムに出す)
		 */
		struct ShotRecord
		{
			/** アルバムに残した写真の番号(残していなければ -1) */
			int photoIndex = -1;
			/** ミッションがどう数えたか */
			ShotStatus status = ShotStatus::NoSubject;
			/** 写っていた悪党の種類(写っていなければ Max) */
			actor::EnemyBase::EnemyType subjectType = actor::EnemyBase::EnemyType::Max;
			/** 照合の結果(Counted / Failed の時だけ使う) */
			MatchResult result;
			/** 比べたミッションの番号(ミッションの一覧の順番。比べていなければ -1。見本の絵を出すのに使う) */
			int missionIndex = -1;
		};


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
			 * @details 同じ盗みを何枚撮っても、1枚として数える。
			 *          数えなかった写真も、撮った記録(ShotRecord)には残す
			 */
			bool OnShot(const ShotInfo& shot);

			/**
			 * @brief ミッションを終わらせる
			 * @details 達成していないミッションを失敗にする(インゲームを抜ける時に呼ぶ)
			 */
			void Finish();

			/**
			 * @brief デバッグ: ミッションを 1 枚進める(撮らずにゲームループを確かめる用)
			 * @param index ミッションの番号(一覧の順番)
			 * @return 進めたら true(進行中でない・番号が無い時は false)
			 * @details 必要な枚数になったら達成にする(撮って数えた時と同じ)
			 */
			bool DebugAddCount(const int index);
			/**
			 * @brief デバッグ: 進行中のミッションを全部達成にする
			 */
			void DebugAchieveAll();


		public:
			/**
			 * @brief ミッションの一覧を取得
			 * @return ミッションの一覧
			 * @details ミッションの一覧は Jsonの missions の配列の中身を読み込んだもの。
			 */
			const std::vector<MissionData>& GetMissionList() const { return m_missionList; }
			/**
		 	 * @brief 見本を取得
			 */
			const MatchTolerance GetMatchTolerance() const { return m_commonTolerance; }
			/**
			 * @brief 制限時間を取得
			 * @return 制限時間(秒。JSON の "timeLimit")
			 */
			float GetTimeLimit() const { return m_timeLimit; }

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
			/**
			 * @brief 最後に撮った写真の照合結果があるか
			 * @return 悪党が写っていない・同じ盗みを取り直した際は false　それ以外は true
			 */
			bool HasLastMatchResult() const { return m_hasLastMatchResult; }
			/**
			 * @brief 最後に撮った写真の照合結果
			 * @return 写真の照合結果
			 */
			const MatchResult& GetLastMatchResult() const { return m_lastMatchResult; }
			/**
			 * @brief 撮った写真の記録を取得
			 * @return 撮った順の記録(リザルトのアルバムに出す)
			 */
			const std::vector<ShotRecord>& GetShotRecordList() const { return m_shotRecordList; }
			/**
			 * @brief 今撮ったらどうなるかを、数えずに照合だけする(ファインダーのチェック欄に使う)
			 * @param shot 今の写り方
			 * @return 進行中のミッションとの照合の結果(狙いの悪党のミッションを優先。無ければ全部 NG)
			 */
			MatchResult PreviewShot(const ShotInfo& shot) const;


		private:
			/**
			 * @brief 撮った写真をミッションと照合して、数える
			 * @param shot 撮った瞬間の情報
			 * @param record 撮った記録(どう数えたかを入れる)
			 * @return どれかのミッションの枚数が増えたら true
			 */
			bool JudgeShot(const ShotInfo& shot, ShotRecord& record);
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
			/** 撮った写真の記録(撮った順) */
			std::vector<ShotRecord> m_shotRecordList;
			/** 最後に撮った写真の照合結果 */
			MatchResult m_lastMatchResult;
			/** 照合の許容値をまとめた情報 */
			MatchTolerance m_commonTolerance;
			/** 制限時間(秒) */
			float m_timeLimit;
			/** 最後に撮った写真の照合結果があるか */
			bool m_hasLastMatchResult;
		};
	}
}