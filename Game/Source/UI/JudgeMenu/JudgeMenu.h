/**
 * @file JudgeMenu.h
 * @brief 照合ジャッジの UI(撮った直後に、照合〇 / 照合なし× を出す)
 * @details MVP の View。渡された表示用の値(JudgeViewData)を出すだけで、ミッション(Model)も入力も知らない。
 *          値を作るのと、開く・閉じる・時間を止めるのは Presenter(InGameScene)の役目。
 *          撮った写真を大きく出して、まわりの枠の色を一致度で変える。
 *          横に照合の項目をチェックリストで並べ、上から順にチェックボックスにチェックを付けていき(合っていない項目は赤くなる)、最後にハンコを押す。
 *          チェックリストの行は JSON ではなく、渡された数だけ、ここで作る。
 *          JSON : Assets/parameter/UI/Judge/Judge.json
 */
#pragma once
#include "Source/UI/UIMenu.h"
#include <string>
#include <vector>


namespace app
{
	namespace ui
	{
		/**
		 * @brief ジャッジの結果(色とハンコの文字は View が決める)
		 */
		enum class JudgeTone : uint8_t
		{
			Match,		// 照合〇(ミッションに数えた)
			Miss,		// 照合なし×(どれかの項目が NG・悪党が写っていない)
			Neutral		// 数えなかった(同じ盗みをもう撮った・ミッションが終わっている)
		};


		/**
		 * @brief チェックリストの 1 行
		 */
		struct JudgeCheckData
		{
			/** 項目の名前 */
			std::wstring name;
			/** 合っているか(true ならチェックが付く) */
			bool isOk = false;
		};


		/**
		 * @brief ジャッジに出す値をまとめたもの(Presenter が作って View に渡す)
		 */
		struct JudgeViewData
		{
			/** 撮った写真(借りているだけ。無い時は nullptr → 枠だけ出す) */
			Texture* photo = nullptr;
			/** 結果 */
			JudgeTone tone = JudgeTone::Miss;
			/** 一致度(0 ～ 1。枠の色を決める) */
			float matchRate = 0.0f;
			/** チェックリスト(上から順。空ならチェックリストの欄ごと消す) */
			std::vector<JudgeCheckData> checkList;
			/** 写真の下に出す文(\n で改行) */
			std::wstring detail;
		};




		/****************************************************/


		/**
		 * @brief 照合ジャッジの UI
		 */
		class JudgeMenu : public UIMenuBase
		{
		public:
			JudgeMenu();
			~JudgeMenu() override;

			/** 初期化処理(ホットリロードのたびにも呼ばれる) */
			void InitializeLogic() override;
			/** 更新処理(出てくる動きを進める) */
			void Update() override;


		public:
			/**
			 * @brief 出す値を設定して、出てくる動きを最初からやり直す
			 * @param viewData 出す値
			 */
			void Show(const JudgeViewData& viewData);
			/**
			 * @brief 閉じてよいか(出てすぐに閉じると、結果が読めないため)
			 * @return ハンコを押し終わったら true
			 */
			bool CanClose() const;


		private:
			/**
			 * @brief 持っている値を UI に入れる(色・文字・絵。チェックリストの行も作り直す)
			 */
			void Apply();
			/**
			 * @brief チェックリストの行を作る(前に作った行は消してから作る)
			 */
			void CreateCheckRows();
			/**
			 * @brief 出てくる動き(写真が戻る・チェックが順に付く・ハンコが押される)を、今の時間で決める
			 */
			void ApplyAnimation();
			/**
			 * @brief ハンコを押し始める時間(チェックが全部付いた後)
			 * @return 出てからの秒数
			 */
			float GetStampStartTime() const;
			/**
			 * @brief 一致度から枠の色を決める
			 * @return 枠の色
			 */
			Vector4 GetRateColor() const;
			/**
			 * @brief 文字を作って、作った一覧に入れる
			 * @param name UI の名前
			 * @param text 文字
			 * @param position 位置
			 * @param scale スケール
			 * @param color 色
			 * @return 作った文字
			 */
			UIText* CreateText(const std::string& name, const std::wstring& text, const Vector3& position, const Vector3& scale, const Vector4& color);
			/**
			 * @brief 画像を作って、作った一覧に入れる(真ん中が基準)
			 * @param name UI の名前
			 * @param assetName 画像のファイルパス
			 * @param size 横幅と縦幅
			 * @param position 位置
			 * @param color 色(白い画像にかける色)
			 * @return 作った画像
			 */
			UIIcon* CreateIcon(const std::string& name, const char* assetName, const Vector2& size, const Vector3& position, const Vector4& color);


		private:
			/**
			 * @brief チェックリストの 1 行の UI
			 */
			struct CheckRow
			{
				/** 四角いチェックボックス */
				UIIcon* box = nullptr;
				/** チェック(合っている項目だけ作る。合っていなければ nullptr) */
				UIIcon* check = nullptr;
				/** 項目の名前 */
				UIText* name = nullptr;
			};


		private:
			/** 写真のまわりの枠(色が一致度で変わる) */
			UIFrame* m_photoFrame;
			/** 撮った写真 */
			UIIcon* m_photo;
			/** チェックリストの枠 */
			UIFrame* m_checkPanel;
			/** 「チェック」の見出し */
			UIText* m_checkHeading;
			/** チェックリストの 1 行目の左端(この下に行を作る) */
			UIBase* m_checkBase;
			/** 照合〇 のハンコ */
			UIText* m_stampMatch;
			/** 照合なし× のハンコ */
			UIText* m_stampMiss;
			/** 一致度の文字 */
			UIText* m_rateText;
			/** 写真の下の文 */
			UIText* m_detailText;
			/** 「A で続ける」の文字 */
			UIText* m_hintText;

			/** 写真の枠の、JSON に書いた大きさ(動きの基準) */
			Vector3 m_photoFrameBaseScale;
			/** 写真の、JSON に書いた大きさ(動きの基準) */
			Vector3 m_photoBaseScale;
			/** ハンコの、JSON に書いた大きさ(動きの基準) */
			Vector3 m_stampBaseScale;

			/** チェックリストの行(上から順) */
			std::vector<CheckRow> m_checkRowList;
			/** ここで作った UI のキー(作り直す時に消す) */
			std::vector<uint32_t> m_createdKeyList;
			/** 出す値 */
			JudgeViewData m_viewData;
			/** 出てからの時間(秒) */
			float m_timer;
		};
	}
}
