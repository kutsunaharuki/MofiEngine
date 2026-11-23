/**
 * @file UIParts.cpp
 * @brief UIのパーツ群
 */
#include "stdafx.h"
#include "UIParts.h"
#include "Source/Util/JsonConverter.h"
#include <cstdlib>


namespace app
{
	namespace ui
	{
		namespace
		{
			/** 全部の画面で使い回す色と大きさの JSON */
			constexpr const char* UI_STYLE_PATH = "Assets/parameter/UI/Common/UIStyle.json";
			/** 枠に使う白い四角の画像(色は乗算カラーで付ける) */
			constexpr const char* WHITE_SPRITE_PATH = "Assets/spriteData/UI/Common/White.DDS";

			/** 今の色と大きさ(UIStyle::Get で返す) */
			UIStyle g_uiStyle;
			/** JSON を読んだか */
			bool g_isUIStyleLoaded = false;


			/**
			 * @brief 今の値から目標の値へ、少しずつ近づける
			 * @param current 今の値
			 * @param target 目標の値
			 * @param rate 近づける割合(0 〜 1。1 ならすぐ目標になる)
			 * @return 近づけた値
			 */
			Vector4 MoveToward(const Vector4& current, const Vector4& target, const float rate)
			{
				return current + (target - current) * rate;
			}
		}




		/*************************************/
		// UIStyle
		/*************************************/


		UIStyle::UIStyle()
			: mainColor(0.08f, 0.08f, 0.10f, 0.85f)
			, subColor(0.95f, 0.93f, 0.88f, 1.0f)
			, accentColor(0.90f, 0.68f, 0.20f, 1.0f)
			, frameBorderWidth(3.0f)
			, buttonSelectedScale(1.08f)
			, buttonPressedScale(0.94f)
			, buttonPressedTime(0.12f)
			, buttonBlendTime(0.08f)
		{}


		const UIStyle& UIStyle::Get()
		{
			if (!g_isUIStyleLoaded)
			{
				// 読めなくても、もう読みに行かない(毎回ファイルを開かないように)。値は初めの値のまま
				g_isUIStyleLoaded = true;
				Reload();
			}
			return g_uiStyle;
		}


		bool UIStyle::Reload()
		{
			nlohmann::json json;
			if (!util::JsonConverter::LoadFile(UI_STYLE_PATH, json)) return false;

			const UIStyle defaultStyle;
			g_uiStyle.mainColor = util::JsonConverter::ToVector4(json, "mainColor", defaultStyle.mainColor);
			g_uiStyle.subColor = util::JsonConverter::ToVector4(json, "subColor", defaultStyle.subColor);
			g_uiStyle.accentColor = util::JsonConverter::ToVector4(json, "accentColor", defaultStyle.accentColor);
			g_uiStyle.frameBorderWidth = util::JsonConverter::ToFloat(json, "frameBorderWidth", defaultStyle.frameBorderWidth);
			g_uiStyle.buttonSelectedScale = util::JsonConverter::ToFloat(json, "buttonSelectedScale", defaultStyle.buttonSelectedScale);
			g_uiStyle.buttonPressedScale = util::JsonConverter::ToFloat(json, "buttonPressedScale", defaultStyle.buttonPressedScale);
			g_uiStyle.buttonPressedTime = util::JsonConverter::ToFloat(json, "buttonPressedTime", defaultStyle.buttonPressedTime);
			g_uiStyle.buttonBlendTime = util::JsonConverter::ToFloat(json, "buttonBlendTime", defaultStyle.buttonBlendTime);
			g_isUIStyleLoaded = true;
			return true;
		}


		const char* UIStyle::GetFilePath()
		{
			return UI_STYLE_PATH;
		}




		/*************************************/
		// UIFrame
		/*************************************/


		UIFrame::UIFrame()
			: m_size(0.0f, 0.0f)
			, m_fillColor(Vector4::White)
			, m_borderColor(Vector4::White)
			, m_borderWidth(0.0f)
			, m_isInitialized(false)
		{}


		UIFrame::~UIFrame()
		{}


		void UIFrame::Update()
		{
			if (!m_isInitialized) return;

			UpdateAnimation();
			m_transform.UpdateTransform();
			ApplyToSprite();
		}


		void UIFrame::Render(RenderContext& rc)
		{
			if (!m_isInitialized || !m_isDraw) return;

			m_fillRender.Draw(rc);
			// 線の太さが 0 なら、線は出さない
			if (m_borderWidth <= 0.0f) return;
			for (SpriteRender& border : m_borderRenderList)
			{
				border.Draw(rc);
			}
		}


		void UIFrame::Initialize(const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation, const Vector4& fillColor, const Vector4& borderColor, const float borderWidth)
		{
			m_transform.SetLocalPosition(position);
			m_transform.SetLocalScale(scale);
			m_transform.SetLocalRotation(rotation);
			m_size = Vector2(width, height);
			m_fillColor = fillColor;
			m_borderColor = borderColor;
			m_borderWidth = (std::max)(0.0f, borderWidth);

			// 白い四角を、中と線 4 本の大きさで作る(色は乗算カラーで付ける)
			m_fillRender.Init(WHITE_SPRITE_PATH, width, height);
			m_borderRenderList[static_cast<size_t>(BorderSide::Top)].Init(WHITE_SPRITE_PATH, width, m_borderWidth);
			m_borderRenderList[static_cast<size_t>(BorderSide::Bottom)].Init(WHITE_SPRITE_PATH, width, m_borderWidth);
			m_borderRenderList[static_cast<size_t>(BorderSide::Left)].Init(WHITE_SPRITE_PATH, m_borderWidth, height);
			m_borderRenderList[static_cast<size_t>(BorderSide::Right)].Init(WHITE_SPRITE_PATH, m_borderWidth, height);
			m_isInitialized = true;

			m_transform.UpdateTransform();
			ApplyToSprite();
		}


		void UIFrame::ApplyToSprite()
		{
			const Vector3& position = m_transform.GetWorldPosition();
			const Quaternion& rotation = m_transform.GetWorldRotation();
			const Vector3& scale = m_transform.GetWorldScale();
			// アニメーションで色(透明度)を変えた時も効くように、乗算カラーをかける
			const Vector4 fillColor(m_fillColor.x * m_color.x, m_fillColor.y * m_color.y, m_fillColor.z * m_color.z, m_fillColor.w * m_color.w);
			const Vector4 borderColor(m_borderColor.x * m_color.x, m_borderColor.y * m_color.y, m_borderColor.z * m_color.z, m_borderColor.w * m_color.w);

			m_fillRender.SetMulColor(fillColor);
			m_fillRender.SetPivot(Vector2(0.5f, 0.5f));
			m_fillRender.SetTRS(position, rotation, scale);
			m_fillRender.Update();

			// 線は、枠の内側の端に置く(真ん中からのずらし量。回転とスケールに合わせる)
			const float halfWidth = (m_size.x - m_borderWidth) * 0.5f;
			const float halfHeight = (m_size.y - m_borderWidth) * 0.5f;
			const Vector3 offsetList[] =
			{
					Vector3(0.0f, halfHeight, 0.0f)		// 上
				,	Vector3(0.0f, -halfHeight, 0.0f)	// 下
				,	Vector3(-halfWidth, 0.0f, 0.0f)		// 左
				,	Vector3(halfWidth, 0.0f, 0.0f)		// 右
			};
			for (size_t i = 0; i < m_borderRenderList.size(); ++i)
			{
				Vector3 offset(offsetList[i].x * scale.x, offsetList[i].y * scale.y, 0.0f);
				rotation.Apply(offset);

				SpriteRender& border = m_borderRenderList[i];
				border.SetMulColor(borderColor);
				border.SetPivot(Vector2(0.5f, 0.5f));
				border.SetTRS(position + offset, rotation, scale);
				border.Update();
			}
		}




		/*************************************/
		// UIButton
		/*************************************/


		UIButton::UIButton()
			: m_baseScale(Vector3::One)
			, m_state(ButtonState::Normal)
			, m_pressTimer(0.0f)
			, m_currentFill(Vector4::White)
			, m_currentBorder(Vector4::White)
			, m_currentText(Vector4::White)
			, m_currentScale(1.0f)
		{
			// 枠と文字はボタンの子にする(ボタンを動かすと一緒に動く)
			m_frame.GetTransform().SetParent(&m_transform);
			m_label.GetTransform().SetParent(&m_transform);
		}


		UIButton::~UIButton()
		{}


		void UIButton::Update()
		{
			UpdateAnimation();

			// 押した形は、時間が来たら選択中に戻す
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			if (m_state == ButtonState::Pressed)
			{
				m_pressTimer -= deltaTime;
				if (m_pressTimer <= 0.0f) m_state = ButtonState::Selected;
			}

			// 目標の色と大きさへ、少しずつ近づける(パッと変わるより、押した感じが出る)
			Vector4 targetFill, targetBorder, targetText;
			float targetScale = 1.0f;
			GetTargetLook(targetFill, targetBorder, targetText, targetScale);

			const float blendTime = UIStyle::Get().buttonBlendTime;
			const float rate = blendTime > 0.0f ? (std::min)(1.0f, deltaTime / blendTime) : 1.0f;
			m_currentFill = MoveToward(m_currentFill, targetFill, rate);
			m_currentBorder = MoveToward(m_currentBorder, targetBorder, rate);
			m_currentText = MoveToward(m_currentText, targetText, rate);
			m_currentScale += (targetScale - m_currentScale) * rate;

			// ボタンの大きさ = JSON のスケール × 形の倍率
			m_transform.SetLocalScale(m_baseScale * m_currentScale);
			m_transform.UpdateTransform();

			// アニメーションで透明度を変えた時も効くように、ボタンの色をかける
			m_frame.SetColor(m_color);
			m_frame.SetFillColor(m_currentFill);
			m_frame.SetBorderColor(m_currentBorder);
			m_frame.Update();

			m_label.SetColor(Vector4(m_currentText.x * m_color.x, m_currentText.y * m_color.y, m_currentText.z * m_color.z, m_currentText.w * m_color.w));
			m_label.Update();
		}


		void UIButton::Render(RenderContext& rc)
		{
			if (!m_isDraw) return;

			m_frame.Render(rc);
			m_label.Render(rc);
		}


		void UIButton::Initialize(const wchar_t* text, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation, const float textScale)
		{
			const UIStyle& style = UIStyle::Get();

			m_transform.SetLocalPosition(position);
			m_transform.SetLocalRotation(rotation);
			m_transform.SetLocalScale(scale);
			m_baseScale = scale;

			// 枠と文字は、ボタンの真ん中に置く(位置・大きさはボタンが持つ)
			m_frame.Initialize(width, height, Vector3::Zero, Vector3::One, Quaternion::Identity, style.mainColor, style.subColor, style.frameBorderWidth);
			m_label.Initialize(text, Vector3::Zero, Vector3(textScale, textScale, 1.0f), Quaternion::Identity, style.subColor, Vector2(0.5f, 0.5f));

			// 最初は目標の色と大きさにそろえておく(出した瞬間に色が動かないように)
			GetTargetLook(m_currentFill, m_currentBorder, m_currentText, m_currentScale);
			Update();
		}


		void UIButton::SetState(const ButtonState state)
		{
			// 押している間は、押し終わるまで形を変えない(押した感じが消えないように)
			if (m_state == ButtonState::Pressed && state != ButtonState::Normal) return;

			m_state = state;
			if (state == ButtonState::Pressed) m_pressTimer = UIStyle::Get().buttonPressedTime;
		}


		void UIButton::Press()
		{
			m_state = ButtonState::Pressed;
			m_pressTimer = UIStyle::Get().buttonPressedTime;
		}


		void UIButton::GetTargetLook(Vector4& outFill, Vector4& outBorder, Vector4& outText, float& outScale) const
		{
			const UIStyle& style = UIStyle::Get();

			switch (m_state)
			{
			case ButtonState::Selected:
				// 選択中 : 線と文字を目立たせる色にして、少し大きくする
				outFill = style.mainColor;
				outBorder = style.accentColor;
				outText = style.accentColor;
				outScale = style.buttonSelectedScale;
				return;

			case ButtonState::Pressed:
				// 押した : 中を目立たせる色で塗って、少し小さくする
				outFill = style.accentColor;
				outBorder = style.accentColor;
				outText = style.mainColor;
				outScale = style.buttonPressedScale;
				return;

			default:
				// 通常 : 濃い中に明るい線と文字
				outFill = style.mainColor;
				outBorder = style.subColor;
				outText = style.subColor;
				outScale = 1.0f;
				return;
			}
		}
		/*************************************/
		// UIImage
		/*************************************/


		UIImage::UIImage()
			: m_size(0.0f, 0.0f)
			, m_texture(nullptr)
			, m_isInitialized(false)
		{}


		UIImage::~UIImage()
		{}


		void UIImage::Update()
		{
			// 初期化前は、スプライトが使えないので何もしない
			if (!m_isInitialized) return;

			// アニメーション → 位置の計算 → スプライトに反映、の順にする
			// ※ UIIcon / UIGauge もこの関数を呼ぶので、アニメーションはここで1回だけ更新される
			UpdateAnimation();
			m_transform.UpdateTransform();
			ApplyToSprite();
		}


		void UIImage::Render(RenderContext& rc)
		{
			if (!m_isInitialized) return;

			if (m_isDraw)
			{
				m_spriteRender.Draw(rc);
			}
		}


		void UIImage::Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation, const Vector4& color, const Vector2& pivot)
		{
			m_transform.SetLocalPosition(position);
			m_transform.SetLocalScale(scale);
			m_transform.SetLocalRotation(rotation);
			m_color = color;
			m_pivot = pivot;
			m_size = Vector2(width, height);

			// 画像が無い時は、スプライトを作らない(あとで SetTexture で渡す)
			// ※ 先に DDS で作ってからテクスチャで作り直すと、同じスプライトを 2 回 Init することになる
			if (assetName == nullptr || assetName[0] == '\0')
			{
				m_transform.UpdateTransform();
				return;
			}

			m_spriteRender.Init(assetName, width, height);
			m_isInitialized = true;

			// 初期状態をスプライトに反映する
			m_transform.UpdateTransform();
			ApplyToSprite();
		}


		void UIImage::SetTexture(Texture& texture)
		{
			// 同じ絵を渡し直した時は作り直さない(毎フレーム呼ばれても重くならないように)
			if (m_texture == &texture) return;
			m_texture = &texture;

			// 大きさは JSON の width / height のまま
			m_spriteRender.Init(texture, m_size.x, m_size.y);
			m_isInitialized = true;

			// 位置・色などをスプライトに反映する
			m_transform.UpdateTransform();
			ApplyToSprite();
		}


		void UIImage::ApplyToSprite()
		{
			m_spriteRender.SetMulColor(m_color);
			m_spriteRender.SetPivot(m_pivot);
			// 描画には、親の影響を受けたワールド値を使う
			m_spriteRender.SetTRS(
				m_transform.GetWorldPosition(),
				m_transform.GetWorldRotation(),
				m_transform.GetWorldScale()
			);
			m_spriteRender.Update();
		}




		/*************************************/
		// UIIcon
		/*************************************/


		UIIcon::UIIcon()
		{}


		UIIcon::~UIIcon()
		{}


		void UIIcon::Update()
		{
			// アニメーション・位置の計算・スプライトへの反映は、UIImage と同じ
			// ※ UIImage::Update の中でアニメーションも更新されるので、ここでは呼ばない(2回進んでしまう)
			// ※ m_transform.UpdateTransform() だけだと、スプライトに位置や色が渡らず、画面に出ない
			UIImage::Update();

			// アイコンだけの処理は、ここに足す
		}


		void UIIcon::Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation, const Vector4& color, const Vector2& pivot)
		{
			// 画像の読み込みと、位置・回転・スケール・色・基底軸の設定は、UIImage と同じ
			// ※ 位置などを入れるだけで画像を読み込まない(m_spriteRender.Init を呼ばない)と、描画しても何も出ない
			UIImage::Initialize(assetName, width, height, position, scale, rotation, color, pivot);

			// アイコンだけの初期化は、ここに足す
		}



		/*************************************/
		// UIGauge
		/*************************************/


		UIGauge::UIGauge()
			: m_baseScale(Vector3::One)
			, m_rate(1.0f)
		{}


		UIGauge::~UIGauge()
		{}


		void UIGauge::Update()
		{
			// ゲージの量に合わせて、横のスケールを変える
			Vector3 scale = m_baseScale;
			scale.x *= m_rate;
			m_transform.SetLocalScale(scale);

			// アニメーション・位置の計算・スプライトへの反映は、UIImage と同じ
			// ※ アニメーションはこの中で更新される。拡縮のアニメーションを付けると、ゲージの量のスケールより優先される
			UIImage::Update();
		}


		void UIGauge::Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation, const Vector4& color, const Vector2& pivot)
		{
			// 満タンのときのスケールを覚えておく
			m_baseScale = scale;
			m_rate = 1.0f;

			UIImage::Initialize(assetName, width, height, position, scale, rotation, color, pivot);
		}


		void UIGauge::SetRate(const float rate)
		{
			// 0.0〜1.0 の範囲に収める
			float clamped = rate;
			if (clamped < 0.0f)
			{
				clamped = 0.0f;
			}
			else if (clamped > 1.0f)
			{
				clamped = 1.0f;
			}
			m_rate = clamped;
		}




		/*************************************/
		// UIDigit
		/*************************************/


		UIDigit::UIDigit()
			: m_number(0)
			, m_requestNumber(0)
			, m_minDigit(1)
			, m_digit(1)
			, m_width(0.0f)
			, m_height(0.0f)
		{}


		UIDigit::~UIDigit()
		{}


		void UIDigit::Update()
		{
			// 表示する数が変わったら、各桁のスプライトを作り直す
			if (m_number != m_requestNumber)
			{
				m_number = m_requestNumber;
				RebuildSprites();
			}

			// アニメーション → 位置の計算 → スプライトに反映、の順にする
			UpdateAnimation();
			m_transform.UpdateTransform();

			const Vector3& basePosition = m_transform.GetWorldPosition();
			const Quaternion& rotation = m_transform.GetWorldRotation();
			const Vector3& scale = m_transform.GetWorldScale();

			for (size_t i = 0; i < m_renderList.size(); ++i)
			{
				// 一の位を基準の位置にして、桁が増えるほど左に並べる
				Vector3 position = basePosition;
				position.x -= m_width * scale.x * static_cast<float>(i);

				SpriteRender* spriteRender = m_renderList[i].get();
				spriteRender->SetMulColor(m_color);
				spriteRender->SetPivot(m_pivot);
				spriteRender->SetTRS(position, rotation, scale);
				spriteRender->Update();
			}
		}


		void UIDigit::Render(RenderContext& rc)
		{
			if (!m_isDraw) return;

			for (auto& spriteRender : m_renderList)
			{
				spriteRender->Draw(rc);
			}
		}


		void UIDigit::Initialize(const char* assetPath, const int digit, const int number, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation)
		{
			m_assetPath = assetPath;
			m_minDigit = digit;
			m_number = number;
			m_requestNumber = number;
			m_width = width;
			m_height = height;

			m_transform.SetLocalPosition(position);
			m_transform.SetLocalScale(scale);
			m_transform.SetLocalRotation(rotation);

			RebuildSprites();
		}


		void UIDigit::RebuildSprites()
		{
			// 桁数は、最低桁数と、数に必要な桁数の、大きいほう
			const int needDigit = ComputeDigit(m_number);
			m_digit = (needDigit > m_minDigit) ? needDigit : m_minDigit;

			// 古いスプライトを、全て解放する
			m_renderList.clear();

			for (int i = 0; i < m_digit; ++i)
			{
				const int value = GetDigitValue(m_number, i);
				const std::string path = m_assetPath + "/" + std::to_string(value) + ".dds";

				auto spriteRender = std::make_unique<SpriteRender>();
				spriteRender->Init(path.c_str(), m_width, m_height);
				m_renderList.push_back(std::move(spriteRender));
			}
		}


		int UIDigit::ComputeDigit(const int number)
		{
			int n = std::abs(number);
			if (n == 0) return 1;

			int count = 0;
			while (n > 0)
			{
				n /= 10;
				count++;
			}
			return count;
		}


		int UIDigit::GetDigitValue(const int number, const int index)
		{
			// 10 の累乗を、整数のまま求める(pow は誤差が出ることがあるため使わない)
			int divisor = 1;
			for (int i = 0; i < index; ++i)
			{
				divisor *= 10;
			}
			return (std::abs(number) / divisor) % 10;
		}




		/*************************************/
		// UIText
		/*************************************/


		UIText::UIText()
		{}


		UIText::~UIText()
		{}


		void UIText::Update()
		{
			// アニメーション → 位置の計算 → フォントに反映、の順にする
			UpdateAnimation();
			m_transform.UpdateTransform();

			// 描画には、親の影響を受けたワールド値を使う
			m_fontRender.SetPosition(m_transform.GetWorldPosition());
			m_fontRender.SetRotation(m_transform.GetWorldRotation());
			m_fontRender.SetScale(m_transform.GetWorldScale().x);
			m_fontRender.SetColor(m_color);
			m_fontRender.SetPivot(m_pivot);
		}


		void UIText::Render(RenderContext& rc)
		{
			if (!m_isDraw) return;
			m_fontRender.Draw(rc);
		}


		void UIText::Initialize(const wchar_t* text, const Vector3& position, const Vector3& scale, const Quaternion& rotation, const Vector4& color, const Vector2& pivot)
		{
			m_transform.SetLocalPosition(position);
			m_transform.SetLocalScale(scale);
			m_transform.SetLocalRotation(rotation);
			m_color = color;
			m_pivot = pivot;
			m_fontRender.SetText(text);

			// 初期状態を反映する
			Update();
		}




		/*************************************/
		// UIDummy
		/*************************************/


		UIDummy::UIDummy()
		{}


		UIDummy::~UIDummy()
		{}


		void UIDummy::Update()
		{
			// 描画は無いが、位置のアニメーションで子のUIをまとめて動かすことがあるので更新する
			UpdateAnimation();
			m_transform.UpdateTransform();
		}


		void UIDummy::Render(RenderContext& rc)
		{
			// 描画なし
		}




		/*************************************/
		// UICanvas
		/*************************************/


		UICanvas::UICanvas()
		{}


		UICanvas::~UICanvas()
		{
			// 子のUIを先に破棄する(子の TransformNode が、親(自分)から外れる)
			m_uiList.clear();
		}


		void UICanvas::Update()
		{
			// キャンバス自身のアニメーション(全体を拡縮・移動するなど)を先に進める
			UpdateAnimation();

			// ルートであるキャンバスのワールド値を、先に確定させる
			// ※ 子のUIは、親のワールド行列を使って、自分の位置を計算する
			m_transform.UpdateTransform();

			for (auto& ui : m_uiList)
			{
				ui->Update();
			}
		}


		void UICanvas::Render(RenderContext& rc)
		{
			// キャンバスごと隠せるようにする
			if (!m_isDraw) return;

			for (auto& ui : m_uiList)
			{
				ui->Render(rc);
			}
		}
	}
}