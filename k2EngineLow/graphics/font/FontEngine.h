#pragma once

namespace nsK2EngineLow {
	/// <summary>
	/// フォントエンジン。
	/// </summary>
	/// <remarks>
	/// DirectXTKのフォント表示処理の初期化～解放を行うクラス。
	/// BeginDrawでフォントの描画開始、Drawで描画リクエスト、Endでリクエストを一括処理。
	/// フォントの描画はまとめて行うほど高速になります。
	/// </remarks>
	/// <example>
	/// Font font;
	/// font.BeginDraw( renderContext );
	/// font.Draw("Hello world", pos, color, rot, scale, pivot);
	/// font.EndDraw( renderContext );
	/// </example>
	class FontEngine : public Noncopyable {

	public:
		/// <summary>
		/// デストラクタ。
		/// </summary>
		~FontEngine();
		/// <summary>
		/// 初期化。
		/// </summary>
		void Init();
		/// <summary>
		/// 描画開始。
		/// </summary>
		void BeginDraw(RenderContext& rc);
		/// <summary>
		/// 文字列を描画。
		/// </summary>
		/// <param name="position">座標</param>
		/// <param name="color">カラー</param>
		/// <param name="rotation">回転</param>
		/// <param name="scale">拡大</param>
		/// <param name="pivot">ピボット</param>
		void Draw(
			const wchar_t* text,
			const Vector2& position,
			const Vector4& color,
			float rotation,
			float scale,
			Vector2 pivot
		);
		/// <summary>
		/// 描画終了。
		/// </summary>
		void EndDraw(RenderContext& rc);
		/// <summary>
		/// 文字列の大きさを測る(拡大前のピクセル数)。
		/// </summary>
		/// <param name="text">文字列</param>
		/// <returns>横幅(x)と縦幅(y)。改行があれば、全ての行を合わせた大きさ</returns>
		Vector2 MeasureString(const wchar_t* text) const;
	private:
		std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;	//スプライトバッチ。
		std::unique_ptr<DirectX::SpriteFont>	m_spriteFont;	//スプライトフォント。
		ID3D12DescriptorHeap* m_srvDescriptorHeap = nullptr;	//SRVのディスクリプタヒープ。
	};
}