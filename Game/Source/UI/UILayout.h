/**
 * @file UILayout.h
 * @brief UIのレイアウト管理
 * @details JSON ファイルからUIを作り、メニューに渡す。
 *          K2_DEBUG の構成(Debug / Preview)では、JSON を保存すると作り直す(ホットリロード)
 *
 * JSON の書き方
 * {
 *   "canvas": {
 *     "position": [0, 0, 0],
 *     "elements": [
 *       { "type": "UIImage", "name": "Title", "asset": "Assets/spriteData/title.dds",
 *         "width": 1920, "height": 1080, "position": [0, 0, 0], "scale": [1, 1, 1],
 *         "rotation": 0, "color": [1, 1, 1, 1], "pivot": [0.5, 0.5] }
 *     ]
 *   }
 * }
 * type は UIImage / UIIcon / UIGauge / UIDigit / UIText / UIDummy
 *
 * UIText の書き方(asset / width / height はいらない)
 *       { "type": "UIText", "name": "MasterLabel", "text": "マスター",
 *         "position": [-450, 180, 0], "scale": [1, 1, 1], "color": [1, 1, 1, 1], "pivot": [0, 0.5],
 *         "shadow": true, "shadowOffset": 2, "shadowColor": [0, 0, 0, 1] }
 */
#pragma once
#include "Source/UI/UIMenu.h"
#include <ctime>
#include <memory>
#include <string>
#include <type_traits>


#ifdef K2_DEBUG
#define APP_ENABLE_LAYOUT_HOTRELOAD
#endif


namespace app
{
	namespace ui
	{
		/**
		 * @brief UIのレイアウト管理クラス
		 */
		class UILayout : public Noncopyable
		{
		public:
			UILayout();
			~UILayout();

			/** 更新処理 */
			void Update();
			/** 描画処理 */
			void Render(RenderContext& rc);


		public:
			/**
			 * @brief 初期化
			 * @tparam T メニューの型(UIMenuBase の派生クラス)
			 * @param filePath レイアウトの JSON ファイルのパス
			 */
			template <typename T>
			void Initialize(const std::string& filePath)
			{
				static_assert(std::is_base_of<UIMenuBase, T>::value, "T は UIMenuBase の派生クラスにしてください。");

				InitializeImpl(filePath, std::make_unique<T>());
			}

			/**
			 * @brief JSON を読み直して、UIを作り直す
			 * @return 読み込めたかどうか(失敗したときは、今のUIのまま)
			 */
			bool Reload();

			/**
			 * @brief メニューを取得
			 * @tparam T メニューの型
			 * @return メニュー(型が違えば nullptr)
			 */
			template <typename T = UIMenuBase>
			T* GetMenu() { return dynamic_cast<T*>(m_menu.get()); }


		private:
			/**
			 * @brief 初期化の中身
			 * @param filePath レイアウトの JSON ファイルのパス
			 * @param menu メニュー
			 * @details JSON をヘッダーに出さないように、cpp に置いている
			 */
			void InitializeImpl(const std::string& filePath, std::unique_ptr<UIMenuBase> menu);


		private:
			/** レイアウトの JSON ファイルのパス */
			std::string m_filePath;
			/** メニュー */
			std::unique_ptr<UIMenuBase> m_menu;
#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
			/** JSON ファイルの最終更新日時 */
			time_t m_lastWriteTime;
#endif
		};
	}
}
