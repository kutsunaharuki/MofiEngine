/**
 * @file FontRender.h
 * @brief フォントレンダークラス(文字の描画)
 */
#pragma once
#include <string>


namespace nsK2EngineLow
{
	/**
	 * @brief 文字を描画するクラス
	 * @details SpriteRender と同じ使い方で、Draw を呼ぶと RenderingEngine の2Dパスで描画される。
	 *          座標はスプライトと同じ(画面の中心が原点、上が+Y)
	 */
	class FontRender : public IRender2D
	{
	public:
		FontRender();
		~FontRender() override;

		/**
		 * @brief 描画処理
		 * @param rc レンダリングコンテキスト
		 * @details RenderingEngine に登録するだけで、実際の描画は2Dパスで行われる
		 */
		void Draw(RenderContext& rc);

		/**
		 * @brief 2Dパスでの実際の描画
		 * @param rc レンダリングコンテキスト
		 * @details RenderingEngine の2Dパスから呼ばれる
		 */
		void OnRender2D(RenderContext& rc) override;


	public:
		/**
		 * @brief 文字列を設定
		 * @param text 文字列(\n で改行できる)
		 */
		void SetText(const wchar_t* text) { m_text = (text != nullptr) ? text : L""; }
		/**
		 * @brief 文字列を取得
		 * @return 文字列
		 */
		const std::wstring& GetText() const { return m_text; }

		/**
		 * @brief 位置を設定
		 * @param position 位置
		 */
		void SetPosition(const Vector3& position) { m_position = position; }
		/**
		 * @brief 回転を設定
		 * @param rotation 回転(Z軸回転だけが使われる)
		 */
		void SetRotation(const Quaternion& rotation) { m_rotation = rotation; }
		/**
		 * @brief 拡大率を設定
		 * @param scale 拡大率(1.0 がフォントの元の大きさ)
		 */
		void SetScale(const float scale) { m_scale = scale; }
		/**
		 * @brief 色を設定
		 * @param color 色
		 */
		void SetColor(const Vector4& color) { m_color = color; }
		/**
		 * @brief 基底軸を設定
		 * @param pivot 基底軸(0.0〜1.0。(0, 1) で左上、(0.5, 0.5) で中央)
		 */
		void SetPivot(const Vector2& pivot) { m_pivot = pivot; }

		/**
		 * @brief 影(縁取り)の設定
		 * @param isDrawShadow 影を描画するか
		 * @param shadowOffset 影をずらすピクセル数
		 * @param shadowColor 影の色
		 */
		void SetShadowParam(const bool isDrawShadow, const float shadowOffset, const Vector4& shadowColor)
		{
			m_isDrawShadow = isDrawShadow;
			m_shadowOffset = shadowOffset;
			m_shadowColor = shadowColor;
		}

		/**
		 * @brief 文字列の大きさを取得(拡大前)
		 * @return 横幅(x)と縦幅(y)
		 */
		Vector2 GetTextSize() const;


	private:
		/** 文字列 */
		std::wstring m_text;
		/** 位置 */
		Vector3 m_position;
		/** 回転 */
		Quaternion m_rotation;
		/** 拡大率 */
		float m_scale;
		/** 色 */
		Vector4 m_color;
		/** 基底軸 */
		Vector2 m_pivot;
		/** 影を描画するか */
		bool m_isDrawShadow;
		/** 影をずらすピクセル数 */
		float m_shadowOffset;
		/** 影の色 */
		Vector4 m_shadowColor;
	};
}
