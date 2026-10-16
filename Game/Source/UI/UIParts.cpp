/**
 * @file UIParts.cpp
 * @brief UIのパーツ群
 */
#include "stdafx.h"
#include "UIParts.h"
#include <cstdlib>


namespace app
{
	namespace ui
	{
		/*************************************/
		// UIImage
		/*************************************/


		UIImage::UIImage()
			: m_size(0.0f, 0.0f)
			, m_isInitialized(false)
		{}


		UIImage::~UIImage()
		{}


		void UIImage::Update()
		{
			// 初期化前は、スプライトが使えないので何もしない
			if (!m_isInitialized) return;

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

			m_spriteRender.Init(assetName, width, height);
			m_isInitialized = true;

			// 初期状態をスプライトに反映する
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
		// UIDummy
		/*************************************/


		UIDummy::UIDummy()
		{}


		UIDummy::~UIDummy()
		{}


		void UIDummy::Update()
		{
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