/***
 * @file FinderMenu.cpp
 * @brief ファインダーを構えている時の UI
 */
#include "stdafx.h"
#include "FinderMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** UI の名前(JSON の "name") */
			constexpr const char* FOCUS_BAR_NAME = "FocusBar";
			constexpr const char* FOCUS_KNOB_NAME = "FocusKnob";
			constexpr const char* ZOOM_BAR_NAME = "ZoomBar";
			constexpr const char* ZOOM_KNOB_NAME = "ZoomKnob";
			constexpr const char* FOCUS_FRAME_NAME = "FinderFrame";
			constexpr const char* GRID_NAME = "FocusGrid";
			constexpr const char* GRID_CENTER_NAME = "FocusGridCenter";
			constexpr const char* IN_RANGE_CIRCLE_NAME = "InRangeCircle";
			constexpr const char* OUT_RANGE_CIRCLE_NAME = "OutRangeCircle";
			constexpr const char* SLOW_GAUGE_NAME = "SlowGauge";
			constexpr const char* SHOT_FLASH_NAME = "ShotFlash";

			/** チェック欄の印と文字の名前(上から 顔が見える・盗んでいる・ピントが合っている) */
			constexpr const char* CHECK_MARK_NAME_LIST[] = { "CheckMarkFace", "CheckMarkSteal", "CheckMarkFocus" };
			constexpr const char* CHECK_TEXT_NAME_LIST[] = { "CheckTextFace", "CheckTextSteal", "CheckTextFocus" };
			/** チェックが入った時の文字の色 / 入っていない時の色 */
			const Vector4 CHECK_ON_TEXT_COLOR = Vector4(0.55f, 1.0f, 0.55f, 1.0f);
			const Vector4 CHECK_OFF_TEXT_COLOR = Vector4(1.0f, 1.0f, 1.0f, 0.75f);
			/** チェックの印の色(みどり) */
			const Vector4 CHECK_MARK_COLOR = Vector4(0.4f, 1.0f, 0.4f, 1.0f);
			/** チェックが入った時に、印がポンと大きく出る時間(秒)と大きさ */
			constexpr float CHECK_POP_TIME = 0.2f;
			constexpr float CHECK_POP_SCALE = 1.6f;

			/** 撮った時のフラッシュの、一番白い時の濃さ(0〜1) */
			constexpr float SHOT_FLASH_MAX_ALPHA = 0.9f;
			/** 撮った時のフラッシュが、一番白くなるまでの時間(秒) */
			constexpr float SHOT_FLASH_RISE_TIME = 0.03f;
			/** 撮った時のフラッシュが、一番白い所から消えるまでの時間(秒) */
			constexpr float SHOT_FLASH_FALL_TIME = 0.4f;

			/** 使い切った直後(まだ遅くできない)の、スローモーションのゲージの色 */
			const Vector4 SLOW_GAUGE_EMPTY_COLOR = Vector4(0.9f, 0.3f, 0.3f, 0.9f);

			/**
			 * @brief つまみが動く範囲の、バーの端からの余白(バーの高さに対する割合)
			 * @details バーの画像は、上下の端が丸くなっているので、端ぴったりまでは動かさない
			 */
			constexpr float KNOB_MARGIN_RATE = 0.05f;
			/** 半分 */
			constexpr float HALF = 0.5f;
		}


		FinderMenu::FinderMenu()
			: m_focusBar(nullptr)
			, m_focusKnob(nullptr)
			, m_zoomBar(nullptr)
			, m_zoomKnob(nullptr)
			, m_grid(nullptr)
			, m_gridCenter(nullptr)
			, m_inRangeCircle(nullptr)
			, m_outRangeCircle(nullptr)
			, m_focusFrame(nullptr)
			, m_focusKnobBasePosition(0.0f, 0.0f, 0.0f)
			, m_zoomKnobBasePosition(0.0f, 0.0f, 0.0f)
			, m_focusRate(0.0f)
			, m_zoomRate(0.0f)
			, m_slowGauge(nullptr)
			, m_slowGaugeColor(1.0f, 1.0f, 1.0f, 1.0f)
			, m_slowGaugeRate(1.0f)
			, m_canSlowStart(true)
			, m_shotFlash(nullptr)
			, m_shotFlashTimer(-1.0f)
			, m_isOpen(false)
		{}


		FinderMenu::~FinderMenu()
		{}


		void FinderMenu::Update()
		{
			// 閉じている時は、何もしない(描画もされない)
			if (!m_isOpen) return;

			// つまみを、今の量の位置に動かす
			ApplyKnob(m_focusKnob, m_focusKnobBasePosition, m_focusBar, m_focusRate);
			ApplyKnob(m_zoomKnob, m_zoomKnobBasePosition, m_zoomBar, m_zoomRate);
			// スローモーションのゲージを、残りの量に合わせる
			ApplySlowGauge();
			// 撮った時のフラッシュ
			ApplyShotFlash();
			// チェック欄
			ApplyCheckList();

			UIMenuBase::Update();
		}


		void FinderMenu::InitializeLogic()
		{
			// ホットリロードで UI が作り直されたので、全て探し直す
			// ※ JSON の "type" と型が違うと nullptr になる(その UI は動かなくなるので、JSON を確認する)
			m_focusBar = FindUI<UIIcon>(FOCUS_BAR_NAME);
			m_focusKnob = FindUI<UIIcon>(FOCUS_KNOB_NAME);
			m_zoomBar = FindUI<UIIcon>(ZOOM_BAR_NAME);
			m_zoomKnob = FindUI<UIIcon>(ZOOM_KNOB_NAME);
			m_focusFrame = FindUI<UIIcon>(FOCUS_FRAME_NAME);
			m_grid = FindUI<UIIcon>(GRID_NAME);
			m_gridCenter = FindUI<UIIcon>(GRID_CENTER_NAME);
			m_inRangeCircle = FindUI<UIIcon>(IN_RANGE_CIRCLE_NAME);
			m_outRangeCircle = FindUI<UIIcon>(OUT_RANGE_CIRCLE_NAME);

			// つまみの x は JSON の位置のまま使うので、覚えておく(y は量に合わせて動かす)
			if (m_focusKnob) m_focusKnobBasePosition = m_focusKnob->GetPosition();
			if (m_zoomKnob) m_zoomKnobBasePosition = m_zoomKnob->GetPosition();

			// 丸は、被写体が見つかるまで出さない
			HideSubjectCircle();

			// スローモーションのゲージ(JSON の色を、遅くできる時の色として覚える)
			m_slowGauge = FindUI<UIIcon>(SLOW_GAUGE_NAME);

			// 撮った時のフラッシュ(光っていない時は出さない)
			m_shotFlash = FindUI<UIFrame>(SHOT_FLASH_NAME);
			if (m_shotFlash) m_shotFlash->SetIsDraw(false);

			// チェック欄(作り直したので探し直す。合っているかは残す)
			for (int i = 0; i < static_cast<int>(CheckItem::Max); ++i)
			{
				CheckRow& row = m_checkRowList[i];
				row.mark = FindUI<UIIcon>(CHECK_MARK_NAME_LIST[i]);
				row.text = FindUI<UIText>(CHECK_TEXT_NAME_LIST[i]);
				if (row.mark) row.markBaseScale = row.mark->GetScale();
			}
			if (m_slowGauge) m_slowGaugeColor = m_slowGauge->GetColor();

			// 作り直したキャンバスにも、開いているかを反映する
			GetCanvas()->SetIsDraw(m_isOpen);
		}


		void FinderMenu::Open()
		{
			m_isOpen = true;
			GetCanvas()->SetIsDraw(true);
		}


		void FinderMenu::Close()
		{
			m_isOpen = false;
			GetCanvas()->SetIsDraw(false);
			HideSubjectCircle();
		}


		void FinderMenu::ShowSubjectCircle(const Vector2& screenPosition, const float radius, const bool isInRange)
		{
			// 範囲内なら細い丸、範囲外なら太い丸を出す(片方だけ)
			if (m_inRangeCircle)
			{
				m_inRangeCircle->SetIsDraw(isInRange);
				ApplyCircle(m_inRangeCircle, screenPosition, radius);
			}
			if (m_outRangeCircle)
			{
				m_outRangeCircle->SetIsDraw(!isInRange);
				ApplyCircle(m_outRangeCircle, screenPosition, radius);
			}
		}


		void FinderMenu::HideSubjectCircle()
		{
			if (m_inRangeCircle) m_inRangeCircle->SetIsDraw(false);
			if (m_outRangeCircle) m_outRangeCircle->SetIsDraw(false);
		}


		Vector2 FinderMenu::GetGridHalfSize()
		{
			if (m_grid == nullptr) return Vector2(0.0f, 0.0f);

			const Vector2 size = m_grid->GetSize();
			const Vector3& scale = m_grid->GetScale();
			return Vector2(size.x * scale.x * HALF, size.y * scale.y * HALF);
		}


		void FinderMenu::ApplyKnob(UIIcon* knob, const Vector3& knobBasePosition, UIIcon* bar, const float rate)
		{
			if (knob == nullptr || bar == nullptr) return;

			// バーの基底軸は中央(0.5, 0.5)の前提。バーの下端から上端までを、量に合わせて動く
			const float barHeight = bar->GetSize().y * bar->GetScale().y;
			const float movableHeight = barHeight * (1.0f - KNOB_MARGIN_RATE * 2.0f);
			const float bottom = bar->GetPosition().y - movableHeight * 0.5f;

			Vector3 position = knobBasePosition;
			position.y = bottom + movableHeight * TClamp<float>::Clamp(rate, 0.0f, 1.0f);
			knob->SetPosition(position);
		}


		void FinderMenu::ApplySlowGauge()
		{
			if (m_slowGauge == nullptr) return;

			// 基底軸を左端(pivot の x = 0)にしてあるので、横のスケールを残りの割合にすると、左から縮む
			const float rate = TClamp<float>::Clamp(m_slowGaugeRate, 0.0f, 1.0f);
			Vector3 scale = m_slowGauge->GetScale();
			scale.x = rate;
			m_slowGauge->SetScale(scale);

			// 使い切った直後(まだ遅くできない)は赤くする
			m_slowGauge->SetColor(m_canSlowStart ? m_slowGaugeColor : SLOW_GAUGE_EMPTY_COLOR);
		}


		void FinderMenu::SetCheck(const CheckItem item, const bool isChecked)
		{
			const int index = static_cast<int>(item);
			if (index < 0 || index >= static_cast<int>(CheckItem::Max)) return;

			CheckRow& row = m_checkRowList[index];
			// 入った瞬間だけ、ポンと出る動きを最初から
			if (isChecked && !row.isChecked) row.checkTimer = 0.0f;
			row.isChecked = isChecked;
		}


		void FinderMenu::ApplyCheckList()
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			for (CheckRow& row : m_checkRowList)
			{
				if (row.text) row.text->SetColor(row.isChecked ? CHECK_ON_TEXT_COLOR : CHECK_OFF_TEXT_COLOR);
				if (row.mark == nullptr) continue;

				row.mark->SetIsDraw(row.isChecked);
				if (!row.isChecked) continue;

				// 入った時に大きく出て、元の大きさにもどる
				row.checkTimer += deltaTime;
				const float rate = TClamp<float>::Clamp(row.checkTimer / CHECK_POP_TIME, 0.0f, 1.0f);
				const float scale = CHECK_POP_SCALE + (1.0f - CHECK_POP_SCALE) * rate;
				row.mark->SetScale(Vector3(row.markBaseScale.x * scale, row.markBaseScale.y * scale, row.markBaseScale.z));
				row.mark->SetColor(CHECK_MARK_COLOR);
			}
		}


		void FinderMenu::ApplyShotFlash()
		{
			if (m_shotFlash == nullptr) return;

			// 光っていない
			if (m_shotFlashTimer < 0.0f)
			{
				m_shotFlash->SetIsDraw(false);
				return;
			}

			m_shotFlashTimer += g_gameTime->GetFrameDeltaTime();

			// 一気に白くして、はじめは速く、だんだんゆっくり消す
			float power = 0.0f;
			if (m_shotFlashTimer < SHOT_FLASH_RISE_TIME)
			{
				power = m_shotFlashTimer / SHOT_FLASH_RISE_TIME;
			}
			else
			{
				const float rate = TClamp<float>::Clamp((m_shotFlashTimer - SHOT_FLASH_RISE_TIME) / SHOT_FLASH_FALL_TIME, 0.0f, 1.0f);
				power = (1.0f - rate) * (1.0f - rate);
				// 消え切ったら終わり
				if (rate >= 1.0f) m_shotFlashTimer = -1.0f;
			}

			m_shotFlash->SetIsDraw(power > 0.0f);
			m_shotFlash->SetFillColor(Vector4(1.0f, 1.0f, 1.0f, SHOT_FLASH_MAX_ALPHA * power));
		}


		void FinderMenu::ApplyCircle(UIIcon* circle, const Vector2& screenPosition, const float radius)
		{
			circle->SetPosition(Vector3(screenPosition.x, screenPosition.y, 0.0f));
			
			// 丸の画像の横幅が、直径(半径 × 2)になるように拡大する
			const Vector2 size = circle->GetSize();
			if (size.x > 0.0f)
			{
				const float scale = (radius * 2.0f) / size.x;
				circle->SetScale(Vector3(scale, scale, 1.0f));
			}
		}
	}
}
