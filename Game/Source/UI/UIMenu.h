/**
 * @file UIMenu.h
 * @brief UIのメニュー(UIの動き・ロジックを書く場所)
 * @details UILayout が JSON から作ったUIを、UIMenuBase の派生クラスで動かす
 */
#pragma once
#include "Source/UI/UIParts.h"
#include "Source/Util/CRC32.h"
#include <memory>
#include <type_traits>


namespace app
{
	namespace ui
	{
		/**
		 * @brief メニューの基底クラス
		 * @details UIはキャンバスが持っている。メニューは、キャンバスからUIを探して動かすだけ。
		 *          ボタンが押されたときの処理などは、派生クラスの InitializeLogic と Update に書く
		 */
		class UIMenuBase : public Noncopyable
		{
		public:
			UIMenuBase();
			virtual ~UIMenuBase();

			/** 更新処理 */
			virtual void Update();
			/** 描画処理 */
			virtual void Render(RenderContext& rc);

			/**
			 * @brief UIのロジックの初期化
			 * @details UILayout がUIを作り終わった後に呼ばれる(ホットリロードのたびにも呼ばれる)。
			 *          ここでUIを探して、ポインタを持ち直す
			 */
			virtual void InitializeLogic() {}


		public:
			/**
			 * @brief キャンバスを取得
			 * @return キャンバス
			 */
			UICanvas* GetCanvas() { return m_canvas.get(); }

			/**
			 * @brief キャンバスを作り直す
			 * @details 持っていたUIは全て破棄される。外で持っているUIのポインタも使えなくなる
			 */
			void ResetCanvas();

			/**
			 * @brief UIを探す
			 * @tparam T UIの型
			 * @param key UIのキー
			 * @return 見つかったUI(なければ、または型が違えば nullptr)
			 */
			template <typename T>
			T* FindUI(const uint32_t key)
			{
				static_assert(std::is_base_of<UIBase, T>::value, "T は UIBase の派生クラスにしてください。");
				return m_canvas->FindUI<T>(key);
			}

			/**
			 * @brief UIを名前で探す
			 * @tparam T UIの型
			 * @param name UIの名前(JSON の "name")
			 * @return 見つかったUI(なければ、または型が違えば nullptr)
			 */
			template <typename T>
			T* FindUI(const char* name)
			{
				return FindUI<T>(Hash32(name));
			}

			/**
			 * @brief UIがあるかどうか
			 * @param key UIのキー
			 * @return あるかどうか
			 */
			bool HasUI(const uint32_t key) { return m_canvas->FindUI<UIBase>(key) != nullptr; }


		protected:
			/** UIを持っているキャンバス */
			std::unique_ptr<UICanvas> m_canvas;
		};
	}
}
