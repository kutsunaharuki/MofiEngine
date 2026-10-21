/**
 * @file UIParts.h
 * @brief UIのパーツ群
 * @details UIMenu / UILayout など、上位のUIを作るときの土台になるクラスをまとめている。
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "Source/UI/Animation/UIAnimation.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>


namespace app
{
	namespace ui
	{
		/**
		 * @brief UIの基盤クラス
		 */
		class UIBase : public Noncopyable
		{
		public:
			UIBase()
				: m_color(Vector4::White)
				, m_pivot(0.5f, 0.5f)
				, m_isDraw(true)
				, m_key(0)
			{}

			virtual ~UIBase()
			{}


			/** 更新処理 */
			virtual void Update() = 0;
			/** 描画処理 */
			virtual void Render(RenderContext& rc) = 0;

			/**
			 * @brief UIの大きさを取得(スケール適用前)
			 * @return 横幅(x)と縦幅(y)
			 * @details UILayout が、UIを並べるときの間隔の計算に使う。
			 *          大きさを持たないUIは、(0, 0) を返す
			 */
			virtual Vector2 GetSize() const { return Vector2(0.0f, 0.0f); }


		public:
			/**
			 * @brief トランスフォームを取得
			 * @return トランスフォーム
			 * @details 親子付け(SetParent)をしたいときに使う
			 */
			core::TransformNode& GetTransform() { return m_transform; }

			/**
			 * @brief 位置を設定(ローカル)
			 * @param position 位置
			 */
			void SetPosition(const Vector3& position) { m_transform.SetLocalPosition(position); }
			/**
			 * @brief 回転を設定(ローカル)
			 * @param rotation 回転
			 */
			void SetRotation(const Quaternion& rotation) { m_transform.SetLocalRotation(rotation); }
			/**
			 * @brief スケールを設定(ローカル)
			 * @param scale スケール
			 */
			void SetScale(const Vector3& scale) { m_transform.SetLocalScale(scale); }
			/**
			 * @brief 位置を取得(ローカル)
			 * @return 位置
			 */
			const Vector3& GetPosition() { return m_transform.GetLocalPosition(); }
			/**
			 * @brief スケールを取得(ローカル)
			 * @return スケール
			 */
			const Vector3& GetScale() { return m_transform.GetLocalScale(); }

			/**
			 * @brief カラー(乗算カラー)を設定
			 * @param color カラー
			 */
			void SetColor(const Vector4& color) { m_color = color; }
			/**
			 * @brief カラーを取得
			 * @return カラー
			 */
			const Vector4& GetColor() const { return m_color; }
			/**
			 * @brief 基底軸を設定
			 * @param pivot 基底軸(0.0〜1.0。中心は (0.5, 0.5))
			 */
			void SetPivot(const Vector2& pivot) { m_pivot = pivot; }
			/**
			 * @brief 基底軸を取得
			 * @return 基底軸
			 */
			const Vector2& GetPivot() const { return m_pivot; }

			/**
			 * @brief 描画するかどうか
			 * @return 描画するかどうか
			 */
			inline bool IsDraw() const { return m_isDraw; }
			/**
			 * @brief 描画するかどうかの設定
			 * @param isDraw 描画するかどうか
			 */
			inline void SetIsDraw(const bool isDraw) { m_isDraw = isDraw; }

			/**
			 * @brief キーの設定
			 * @param key キー
			 */
			void SetKey(const uint32_t key) { m_key = key; }
			/**
			 * @brief キーの取得
			 * @return キー
			 */
			uint32_t GetKey() const { return m_key; }

			/**
			 * @brief このUIのアニメーターを取得(初めて呼んだときに作る)
			 * @return アニメーター(アニメーションの対象は、このUI)
			 * @details アニメーションの更新は、各パーツの Update の中で行われる。
			 *          使用例 : ui->GetAnimator().CreateAnimation<UIColorAnimation>(KEY)->SetParameter(...);
			 *                   ui->GetAnimator().Play(KEY);
			 * ※ ホットリロードでUIが作り直されると、アニメーションも消えるので、InitializeLogic で作り直す
			 */
			UIAnimator& GetAnimator()
			{
				if (m_animator == nullptr)
				{
					m_animator = std::make_unique<UIAnimator>();
					m_animator->SetUI(this);
				}
				return *m_animator;
			}


		protected:
			/**
			 * @brief アニメーションを更新する
			 * @details 各パーツの Update の最初に呼ぶ(アニメーションで変えた値を、そのフレームの描画に反映するため)
			 */
			void UpdateAnimation()
			{
				if (m_animator) m_animator->Update();
			}


		protected:
			/** アニメーター(GetAnimator を呼ぶまでは nullptr) */
			std::unique_ptr<UIAnimator> m_animator;
			/** トランスフォーム */
			core::TransformNode m_transform;
			/** カラー */
			Vector4 m_color;
			/** 基底軸 */
			Vector2 m_pivot;
			/** 描画するかのフラグ */
			bool m_isDraw;
			/** キー */
			uint32_t m_key;
		};




		/*****************************************************/


		/**
		 * @brief 画像を表示するUI
		 */
		class UIImage : public UIBase
		{
		public:
			UIImage();
			~UIImage() override;

			/** 更新処理 */
			void Update() override;
			/** 描画処理 */
			void Render(RenderContext& rc) override;


		public:
			/**
			 * @brief 初期化
			 * @param assetName 画像のファイルパス(.dds)
			 * @param width 横
			 * @param height 縦
			 * @param position 位置
			 * @param scale スケール
			 * @param rotation 回転
			 * @param color 乗算カラー
			 * @param pivot 基底軸
			 */
			void Initialize(
				const char* assetName
				, const float width
				, const float height
				, const Vector3& position
				, const Vector3& scale
				, const Quaternion& rotation
				, const Vector4& color
				, const Vector2& pivot = Vector2(0.5f, 0.5f)
			);

			/**
			 * @brief 初期化時の画像サイズ(スケール適用前)を取得
			 * @return 横幅(x)と縦幅(y)
			 */
			Vector2 GetSize() const override { return m_size; }


		protected:
			/**
			 * @brief トランスフォーム・カラー・基底軸を、スプライトに反映する
			 * @details Initialize からも呼べるように、仮想関数にはしていない
			 */
			void ApplyToSprite();


		protected:
			/** スプライトレンダー */
			SpriteRender m_spriteRender;
			/** 初期化時の画像サイズ(スケール適用前) */
			Vector2 m_size;
			/** 初期化済みかどうか */
			bool m_isInitialized;
		};




		/*************************************************/


		/**
		 * @brief UIアイコン
		 */
		class UIIcon : public UIImage
		{
		public:
			UIIcon();
			~UIIcon() override;

			/** 更新処理 */
			void Update() override;

			/**
			 * @brief 初期化
			 * @param assetName
			 * @param width 横
			 * @param height 縦
			 * @param position 位置
			 * @param scale スケール
			 * @param rotation 回転
			 * @param color カラー
			 * @param pivot 基底軸
			 * @details 画像の読み込みは UIImage と同じ。
			 *          ※ pivot の引数が無いと、UIImage の Initialize(pivot あり)が隠れて、UILayout から呼べなくなる
			 */
			void Initialize(
				const char* assetName
				, const float width
				, const float height
				, const Vector3& position
				, const Vector3& scale
				, const Quaternion& rotation
				, const Vector4& color
				, const Vector2& pivot = Vector2(0.5f, 0.5f)
			);
		};




		/************************************************************/


		/**
		 * @brief ゲージUI
		 * @details 横方向に、スケールを変えて、ゲージの量を表す。
		 *          基底軸を左端(0.0, 0.5)にすると、左端を固定して、右へ伸び縮みする
		 */
		class UIGauge : public UIImage
		{
		public:
			UIGauge();
			~UIGauge() override;

			/** 更新処理 */
			void Update() override;


		public:
			/**
			 * @brief 初期化
			 * @param assetName 画像のファイルパス(.dds)
			 * @param width 横
			 * @param height 縦
			 * @param position 位置
			 * @param scale スケール(ゲージが満タンのときのスケール)
			 * @param rotation 回転
			 * @param color 乗算カラー
			 * @param pivot 基底軸
			 */
			void Initialize(
				const char* assetName
				, const float width
				, const float height
				, const Vector3& position
				, const Vector3& scale
				, const Quaternion& rotation
				, const Vector4& color
				, const Vector2& pivot
			);

			/**
			 * @brief ゲージの量を設定
			 * @param rate 0.0(空)〜1.0(満タン)
			 */
			void SetRate(const float rate);
			/**
			 * @brief ゲージの量を取得
			 * @return 0.0(空)〜1.0(満タン)
			 */
			float GetRate() const { return m_rate; }


		private:
			/** 満タンのときのスケール */
			Vector3 m_baseScale;
			/** ゲージの量 */
			float m_rate;
		};




		/************************************************************/


		/**
		 * @brief 数字を表示するUI(スコア表示などで使用)
		 * @details 0〜9 の画像を、assetPath/0.dds 〜 assetPath/9.dds の名前で用意する。
		 *          一の位が基準の位置で、桁が増えるほど左に並ぶ
		 */
		class UIDigit : public UIBase
		{
		public:
			UIDigit();
			~UIDigit() override;

			/** 更新処理 */
			void Update() override;
			/** 描画処理 */
			void Render(RenderContext& rc) override;


		public:
			/**
			 * @brief 初期化
			 * @param assetPath 数字の画像が入ったフォルダのパス
			 * @param digit 最低限、表示する桁数(足りない桁は0で埋める)
			 * @param number 表示する数
			 * @param width 1桁の横
			 * @param height 1桁の縦
			 * @param position 一の位の位置
			 * @param scale スケール
			 * @param rotation 回転
			 */
			void Initialize(
				const char* assetPath
				, const int digit
				, const int number
				, const float width
				, const float height
				, const Vector3& position
				, const Vector3& scale
				, const Quaternion& rotation
			);

			/**
			 * @brief 表示する数を設定
			 * @param number 表示する数(負の数は、符号なしの絶対値で表示される)
			 */
			void SetNumber(const int number) { m_requestNumber = number; }
			/**
			 * @brief 現在表示している数を取得
			 * @return 表示している数
			 */
			int GetNumber() const { return m_number; }

			/**
			 * @brief UIの大きさを取得(スケール適用前)
			 * @return 横幅(x)と縦幅(y)
			 */
			Vector2 GetSize() const override
			{
				return Vector2(m_width * static_cast<float>(m_digit), m_height);
			}


		private:
			/**
			 * @brief 現在の数と桁数に合わせて、各桁のスプライトを作り直す
			 */
			void RebuildSprites();
			/**
			 * @brief 桁数を計算
			 * @param number 数
			 * @return 桁数(0 のときは 1)
			 */
			static int ComputeDigit(const int number);
			/**
			 * @brief 指定した桁の数字を取得
			 * @param number 数
			 * @param index 桁のインデックス(0 が一の位)
			 * @return 指定した桁の数字(0〜9)
			 */
			static int GetDigitValue(const int number, const int index);


		private:
			/** 各桁のスプライトレンダー(index 0 が一の位) */
			std::vector<std::unique_ptr<SpriteRender>> m_renderList;
			/** 数字の画像が入ったフォルダのパス */
			std::string m_assetPath;
			/** 表示している数 */
			int m_number;
			/** 次に表示したい数 */
			int m_requestNumber;
			/** 最低限、表示する桁数 */
			int m_minDigit;
			/** 現在の桁数 */
			int m_digit;
			/** 1桁の横 */
			float m_width;
			/** 1桁の縦 */
			float m_height;
		};




		/*******************************************/


		/**
		 * @brief 文字を表示するUI
		 * @details 画像ではなくフォントで描画するので、拡大しても文字がぼやけない。
		 *          文字の大きさは、スケールの x で変える(1.0 がフォントの元の大きさ)。
		 *          基底軸で揃え方が変わる(左揃え (0, 0.5) / 中央揃え (0.5, 0.5) / 右揃え (1, 0.5))
		 */
		class UIText : public UIBase
		{
		public:
			UIText();
			~UIText() override;

			/** 更新処理 */
			void Update() override;
			/** 描画処理 */
			void Render(RenderContext& rc) override;


		public:
			/**
			 * @brief 初期化
			 * @param text 文字列(\n で改行できる)
			 * @param position 位置
			 * @param scale スケール(x が文字の大きさになる)
			 * @param rotation 回転
			 * @param color 文字の色
			 * @param pivot 基底軸
			 */
			void Initialize(
				const wchar_t* text
				, const Vector3& position
				, const Vector3& scale
				, const Quaternion& rotation
				, const Vector4& color
				, const Vector2& pivot = Vector2(0.5f, 0.5f)
			);

			/**
			 * @brief 文字列を設定
			 * @param text 文字列(\n で改行できる)
			 * @details 数字を出したいときは、SetText(std::to_wstring(number).c_str()) のように使う
			 */
			void SetText(const wchar_t* text) { m_fontRender.SetText(text); }
			/**
			 * @brief 文字列を取得
			 * @return 文字列
			 */
			const std::wstring& GetText() const { return m_fontRender.GetText(); }

			/**
			 * @brief 影(縁取り)を設定
			 * @param isDrawShadow 影を描画するか
			 * @param shadowOffset 影をずらすピクセル数
			 * @param shadowColor 影の色
			 */
			void SetShadow(const bool isDrawShadow, const float shadowOffset, const Vector4& shadowColor)
			{
				m_fontRender.SetShadowParam(isDrawShadow, shadowOffset, shadowColor);
			}

			/**
			 * @brief 文字列の大きさを取得(スケール適用前)
			 * @return 横幅(x)と縦幅(y)
			 */
			Vector2 GetSize() const override { return m_fontRender.GetTextSize(); }


		private:
			/** フォントレンダー */
			FontRender m_fontRender;
		};




		/*******************************************/


		/**
		 * @brief UIのダミー(何も描画しない)
		 * @details 位置だけを持つ目印として使う
		 */
		class UIDummy : public UIBase
		{
		public:
			UIDummy();
			~UIDummy() override;

			void Update() override;
			void Render(RenderContext& rc) override;
		};




		/****************************************************/


		/**
		 * @brief 複数のUIをまとめるキャンバス
		 * @details 持っているUIは、キャンバスのトランスフォームの子になる。
		 *          キャンバスを動かすと、中のUIが、まとめて動く。
		 *          UIMenu / UILayout の、土台になる
		 */
		class UICanvas : public UIBase
		{
		public:
			UICanvas();
			~UICanvas() override;

			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			/**
			 * @brief UIを作成する
			 * @tparam T UIの型(UIBase の派生クラス)
			 * @param key UIのキー
			 * @return 作成したUIのポインタ(所有はキャンバスが持つ)
			 */
			template <typename T>
			T* CreateUI(const uint32_t key)
			{
				static_assert(std::is_base_of<UIBase, T>::value, "T は UIBase の派生クラスにしてください。");

				auto ui = std::make_unique<T>();
				T* rawPtr = ui.get();
				ui->SetKey(key);
				ui->GetTransform().SetParent(&m_transform);
				m_uiList.push_back(std::move(ui));
				return rawPtr;
			}

			/**
			 * @brief UIを削除
			 * @param key UIのキー
			 */
			void RemoveUI(const uint32_t key)
			{
				auto it = std::find_if(m_uiList.begin(), m_uiList.end(),
					[&](const std::unique_ptr<UIBase>& ui) { return ui->GetKey() == key; });

				// 見つかったUIを削除する(TransformNode のデストラクタが、親子付けを外す)
				if (it != m_uiList.end())
				{
					m_uiList.erase(it);
				}
			}

			/**
			 * @brief UIを探す
			 * @tparam T UIの型
			 * @param key UIのキー
			 * @return 見つかったUIのポインタ(なければ、または型が違えば nullptr)
			 */
			template <typename T>
			T* FindUI(const uint32_t key)
			{
				auto it = std::find_if(m_uiList.begin(), m_uiList.end(),
					[&](const std::unique_ptr<UIBase>& ui) { return ui->GetKey() == key; });

				if (it != m_uiList.end())
				{
					return dynamic_cast<T*>(it->get());
				}
				return nullptr;
			}

			/**
			 * @brief 持っているUI全てに対して、処理を行う
			 * @param func UIに対して何を行うか
			 * @details UILayout が、UIを並べるときなどに使う
			 */
			void ForEach(const std::function<void(UIBase*)>& func)
			{
				for (auto& ui : m_uiList)
				{
					func(ui.get());
				}
			}

			/**
			 * @brief 持っているUIの数を取得
			 * @return UIの数
			 */
			size_t GetUICount() const { return m_uiList.size(); }


		private:
			/** UIのリスト */
			std::vector<std::unique_ptr<UIBase>> m_uiList;
		};
	}
}