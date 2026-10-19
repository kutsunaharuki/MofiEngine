/**
 * @file FontRender.cpp
 * @brief フォントレンダークラス(文字の描画)
 */
#include "k2EngineLowPreCompile.h"
#include "FontRender.h"
#include <cmath>


namespace nsK2EngineLow
{
	FontRender::FontRender()
		: m_position(Vector3::Zero)
		, m_rotation(Quaternion::Identity)
		, m_scale(1.0f)
		, m_color(Vector4::White)
		, m_pivot(0.5f, 0.5f)
		, m_isDrawShadow(false)
		, m_shadowOffset(0.0f)
		, m_shadowColor(0.0f, 0.0f, 0.0f, 1.0f)
	{}


	FontRender::~FontRender()
	{}


	void FontRender::Draw(RenderContext& rc)
	{
		// 文字が無ければ描画しない
		if (m_text.empty()) return;

		// 描画を RenderingEngine に任せる(SpriteRender::Draw と同じ形)
		RenderingEngine::GetInstance().AddFontRender(*this);
	}


	void FontRender::OnRender2D(RenderContext& rc)
	{
		const wchar_t* text = m_text.c_str();

		// 座標をスプライトと合わせる(中心が原点・上が+Y → 左上が原点・下が+Y)
		Vector2 position(
			m_position.x + UI_SPACE_WIDTH * 0.5f,
			-m_position.y + UI_SPACE_HEIGHT * 0.5f
		);

		// DirectXTK の origin は、文字列の左上からのピクセル数なので、基底軸(0〜1)から計算する
		// ※ 基底軸はスプライトと同じく、上が 1.0 なので、Y を反転する
		const Vector2 size = GetTextSize();
		const Vector2 origin(size.x * m_pivot.x, size.y * (1.0f - m_pivot.y));

		// Z軸回転の角度を取り出す。画面は下が+Yなので、向きを反転する
		const float rotation = -2.0f * std::atan2(m_rotation.z, m_rotation.w);

		// ※ FontEngine::Draw の最後の引数は、そのまま DirectXTK の origin(ピクセル)として使われる
		auto& fontEngine = g_graphicsEngine->GetFontEngine();
		fontEngine.BeginDraw(rc);

		if (m_isDrawShadow)
		{
			// 8方向にずらして描画して、縁取りにする
			const Vector2 offsetTable[] = {
				{ m_shadowOffset, 0.0f },
				{ -m_shadowOffset, 0.0f },
				{ 0.0f, m_shadowOffset },
				{ 0.0f, -m_shadowOffset },
				{ m_shadowOffset, m_shadowOffset },
				{ m_shadowOffset, -m_shadowOffset },
				{ -m_shadowOffset, m_shadowOffset },
				{ -m_shadowOffset, -m_shadowOffset },
			};
			for (const Vector2& offset : offsetTable)
			{
				Vector2 shadowPosition(position.x + offset.x, position.y + offset.y);
				fontEngine.Draw(text, shadowPosition, m_shadowColor, rotation, m_scale, origin);
			}
		}

		fontEngine.Draw(text, position, m_color, rotation, m_scale, origin);
		fontEngine.EndDraw(rc);
	}


	Vector2 FontRender::GetTextSize() const
	{
		return g_graphicsEngine->GetFontEngine().MeasureString(m_text.c_str());
	}
}
