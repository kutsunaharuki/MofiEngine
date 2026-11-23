/**
 * @file ResultMenu.h
 * @brief リザルトの UI(写真アルバム風)
 * @details MVP の View。渡された表示用の値(ResultViewData)を出すだけで、ゲームの結果(Model)も入力も知らない。
 *          値を作るのと、入力を受けるのは Presenter(ResultScene)の役目。
 *          上にミッションの結果と、照合できた / できなかった枚数、真ん中に撮った写真、下に「もう一度」「タイトルへ」を出す。
 *          写真はプリントした写真のように白いふちを付け、照合のハンコを押す。左上から順に 1 枚ずつ出てくる。
 *          ミッションの行と写真は JSON ではなく、渡された数だけ、ここで作る。
 *          JSON : Assets/parameter/UI/Result/Result.json
 */
#pragma once
#include "Source/UI/UIMenu.h"
#include <array>
#include <string>
#include <vector>


namespace app
{
	namespace ui
	{
		/**
		 * @brief 結果の見え方(色は View が決める)
		 */
		enum class ResultTone : uint8_t
		{
			Success,	// 成功・達成
			Failed,		// 失敗
			Neutral		// 数えなかった(失敗ではない)
		};


		/**
		 * @brief 1 行の文字
		 */
		struct ResultTextData
		{
			/** 文字 */
			std::wstring text;
			/** 見え方 */
			ResultTone tone = ResultTone::Neutral;
		};


		/**
		 * @brief 写真 1 枚
		 */
		struct ResultPhotoData
		{
			/** 写真の絵(借りているだけ。上書きされて無い時は nullptr → 枠だけ出す) */
			Texture* texture = nullptr;
			/** 写真の下に出す文字(\n で改行) */
			std::wstring label;
			/** 見え方(枠と文字の色) */
			ResultTone tone = ResultTone::Neutral;
		};


		/**
		 * @brief 撮った写真の枚数
		 */
		struct ResultCountData
		{
			/** 撮った枚数 */
			int shotCount = 0;
			/** 照合できた枚数(ミッションに数えた) */
			int matchCount = 0;
			/** 照合できなかった枚数(項目が NG・悪党が写っていない) */
			int missCount = 0;
		};


		/**
		 * @brief リザルトに出す値をまとめたもの(Presenter が作って View に渡す)
		 */
		struct ResultViewData
		{
			/** タイトル */
			ResultTextData title;
			/** ミッションの行(上から順) */
			std::vector<ResultTextData> missionRowList;
			/** 写真(左上から順) */
			std::vector<ResultPhotoData> photoList;
			/** 撮った写真の枚数 */
			ResultCountData count;
		};




		/****************************************************/


		/**
		 * @brief リザルトの UI
		 */
		class ResultMenu : public UIMenuBase
		{
		public:
			/** 選べる項目 */
			enum class SelectType : uint8_t
			{
				Retry,	// もう一度
				Title,	// タイトルへ
				Max
			};


		public:
			ResultMenu();
			~ResultMenu() override;

			/** 初期化処理(ホットリロードのたびにも呼ばれる) */
			void InitializeLogic() override;
			/** 更新処理(写真が順に出てくる動きを進める) */
			void Update() override;


		public:
			/**
			 * @brief 出す値を設定する
			 * @param viewData 出す値
			 * @details 設定したら、ミッションの行と写真を作り直す(ホットリロードでも出し直せるように、コピーして持つ)
			 */
			void SetViewData(const ResultViewData& viewData);
			/**
			 * @brief 選んでいる項目を設定する(色と大きさが変わる)
			 * @param type 選んでいる項目
			 */
			void SetSelect(const SelectType type);
			/**
			 * @brief 選んでいる項目のボタンを押した形にする(決めた時に呼ぶ)
			 */
			void PressSelect();


		private:
			/**
			 * @brief タイトル・ミッションの行・写真を作る(前に作った物は消してから作る)
			 */
			void CreateResultUI();
			/**
			 * @brief ここで作った UI を全部消す
			 */
			void RemoveCreatedUI();
			/**
			 * @brief ミッションの行を作る
			 */
			void CreateMissionRows();
			/**
			 * @brief 写真と、写真ごとの文字とハンコを作る
			 */
			void CreatePhotos();
			/**
			 * @brief 枚数の文字を入れる
			 */
			void ApplyCount();
			/**
			 * @brief 写真が順に出てくる動きを、今の時間で決める
			 */
			void ApplyAnimation();
			/**
			 * @brief 選んでいる項目のボタンを、選択中の形にする(ほかは通常の形)
			 */
			void ApplySelect();

			/**
			 * @brief 文字を作って、作った一覧に入れる
			 * @param name UI の名前
			 * @param text 文字
			 * @param position 位置
			 * @param scale スケール
			 * @param color 色
			 * @param pivot 基底軸
			 * @param rotation 回転(ハンコを少し傾ける時に使う)
			 * @return 作った文字
			 */
			UIText* CreateText(const std::string& name, const std::wstring& text, const Vector3& position, const Vector3& scale, const Vector4& color, const Vector2& pivot,
				const Quaternion& rotation = Quaternion::Identity);
			/**
			 * @brief 画像を作って、作った一覧に入れる
			 * @param name UI の名前
			 * @param assetName 画像のファイルパス(空なら、あとで SetTexture で渡す)
			 * @param size 横幅と縦幅
			 * @param position 位置
			 * @param color 色
			 * @param pivot 基底軸
			 * @return 作った画像
			 */
			UIIcon* CreateIcon(const std::string& name, const char* assetName, const Vector2& size, const Vector3& position, const Vector4& color, const Vector2& pivot);


		private:
			/**
			 * @brief 写真 1 枚分の UI(出てくる時に、まとめて出す)
			 */
			struct PhotoGroup
			{
				/** ふち・写真・文字 */
				std::vector<UIBase*> partList;
				/** 照合のハンコ(少し遅れて押す) */
				UIText* stamp = nullptr;
				/** 画びょう(手前から落ちてきて、コルクにささる) */
				UIIcon* pin = nullptr;
				/** 画びょうのかげ(画びょうが高い所にある間は、遠くにうすく落ちる) */
				UIIcon* pinShadow = nullptr;
				/** 画びょうが止まる場所 */
				Vector3 pinPosition = Vector3::Zero;
				/** 画びょうが止まった時の傾き(度) */
				float pinRotationDeg = 0.0f;
			};


			/**
			 * @brief 画びょうが手前から落ちてきて、コルクにささる動きを、今の時間で決める
			 * @param group 写真 1 枚分の UI
			 * @param pinTime 画びょうが落ちはじめてからの時間(秒。マイナスの間は出さない)
			 */
			void ApplyPinAnimation(PhotoGroup& group, const float pinTime);


		private:
			/** 結果のタイトル */
			UIText* m_titleText;
			/** 撮った枚数の文字 */
			UIText* m_countShotText;
			/** 照合できた枚数の文字 */
			UIText* m_countMatchText;
			/** 照合できなかった枚数の文字 */
			UIText* m_countMissText;
			/** 写真 1 枚分の UI(左上から順) */
			std::vector<PhotoGroup> m_photoGroupList;
			/** 出てからの時間(秒) */
			float m_timer;
			/** ミッションの行を並べる基準 */
			UIBase* m_missionBase;
			/** 写真を並べる基準(1 枚目の写真の左上) */
			UIBase* m_photoBase;
			/** 写真が無い時の文字 */
			UIText* m_emptyText;
			/** 選べる項目のボタン(SelectType の順。色と大きさはボタンが変える) */
			std::array<UIButton*, static_cast<size_t>(SelectType::Max)> m_selectButtonList;

			/** ここで作った UI のキー(作り直す時に消す) */
			std::vector<uint32_t> m_createdKeyList;
			/** 出す値 */
			ResultViewData m_viewData;
			/** 選んでいる項目 */
			SelectType m_selectType;
			/** 値を設定されたか(設定される前は、行も写真も作らない) */
			bool m_hasViewData;
		};
	}
}
