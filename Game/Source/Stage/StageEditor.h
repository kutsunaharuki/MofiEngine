/**
 * @file StageEditor.h
 * @brief ステージエディタ(Debug の時だけ使う)
 */
#pragma once
#include <string>


namespace app
{
	namespace stage
	{
		/** 前方宣言 */
		class Stage;


		/**
		 * @brief ステージエディタ
		 * @details ImGui のウィンドウで置物を選んで、数値かギズモ(ImGuizmo)で動かす。
		 *          1 / 2 / 3 キーで、移動 / 回転 / スケールを切り替える
		 *          ※ W/A/S/D はプレイヤーの移動に使っているので、3ds Max の W/E/R は使わない
		 */
		class StageEditor
		{
		public:
			/**
			 * @brief コンストラクタ
			 * @param stage 編集するステージ
			 * @param filePath 保存する JSON のパス
			 * @param tklFilePath 読み直す tkl のパス
			 * @param modelDirectory モデルのフォルダ(最後に / を付ける)
			 */
			StageEditor(Stage* stage, const std::string& filePath, const std::string& tklFilePath, const std::string& modelDirectory);
			~StageEditor() = default;

			/**
			 * @brief 更新処理(ImGui のウィンドウとギズモを出す)
			 * @details ImGui::NewFrame と ImGui::Render の間で呼ぶ(Update の中なら大丈夫)
			 */
			void Update();


		private:
			/** ファイルの操作(保存・読み直し・tkl から読み直し) */
			void DrawFileMenu();
			/** 置物の一覧 */
			void DrawObjectList();
			/** 選んでいる置物の値 */
			void DrawInspector();
			/** 置物の追加 */
			void DrawAddObject();
			/** ギズモの設定(移動・回転・スケール、ワールド・ローカル、スナップ) */
			void DrawGizmoSetting();
			/** ギズモを出して、ドラッグした結果を置物に反映する */
			void UpdateGizmo();
			/** キーでギズモの操作を切り替える */
			void UpdateShortcut();

			/** 変更があったことを記録する(ウィンドウの名前に * が付く) */
			void MarkDirty() { m_isDirty = true; }


		private:
			/** 編集するステージ */
			Stage* m_stage;
			/** 保存する JSON のパス */
			std::string m_filePath;
			/** 読み直す tkl のパス */
			std::string m_tklFilePath;
			/** モデルのフォルダ */
			std::string m_modelDirectory;
			/** 選んでいる置物の番号(選んでいなければ -1) */
			int m_selectedIndex;
			/** 保存していない変更があるか */
			bool m_isDirty;
			/** ギズモの操作(ImGuizmo::OPERATION) */
			int m_gizmoOperation;
			/** ギズモの向き(ImGuizmo::MODE) */
			int m_gizmoMode;
			/** スナップするか */
			bool m_isSnap;
			/** 移動のスナップ */
			float m_snapTranslation;
			/** 回転のスナップ(度数) */
			float m_snapRotation;
			/** スケールのスナップ */
			float m_snapScale;
			/** 前のフレームでギズモを使っていたか(放した瞬間にコリジョンを作り直すため) */
			bool m_isGizmoUsingLastFrame;
			/** 一覧の絞り込み */
			char m_filter[64];
			/** 追加するモデルのパス */
			char m_addModelPath[256];
			/** 最後の操作の結果(画面に出す) */
			std::string m_message;
		};
	}
}
