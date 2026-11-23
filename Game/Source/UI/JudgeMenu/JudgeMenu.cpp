/**
 * @file JudgeMenu.cpp
 * @brief 照合ジャッジの UI(撮った直後に、照合〇 / 照合なし× を出す)
 */
#include "stdafx.h"
#include "JudgeMenu.h"
#include <algorithm>
#include <cmath>
#include <string>


namespace app
{
	namespace ui
	{
		namespace
		{
			/** UI の名前(JSON の "name") */
			constexpr const char* PHOTO_FRAME_NAME = "JudgePhotoFrame";
			constexpr const char* PHOTO_NAME = "JudgePhoto";
			constexpr const char* CHECK_PANEL_NAME = "JudgeCheckPanel";
			constexpr const char* CHECK_HEADING_NAME = "JudgeCheckHeading";
			constexpr const char* CHECK_BASE_NAME = "JudgeCheckBase";
			constexpr const char* STAMP_MATCH_NAME = "JudgeStampMatch";
			constexpr const char* STAMP_MISS_NAME = "JudgeStampMiss";
			constexpr const char* RATE_NAME = "JudgeRate";
			constexpr const char* DETAIL_NAME = "JudgeDetail";
			constexpr const char* HINT_NAME = "JudgeHint";

			// チェックリスト
			/** 四角いチェックボックスの画像(白で描いて、色はコードで付ける。仮の画像を上書きすれば差し替わる) */
			constexpr const char* CHECK_BOX_ASSET = "Assets/spriteData/UI/Judge/CheckBox.DDS";
			/** チェックの画像 */
			constexpr const char* CHECK_MARK_ASSET = "Assets/spriteData/UI/Judge/Check.DDS";
			/** チェックボックスの大きさ */
			const Vector2 CHECK_BOX_SIZE = { 36.0f,36.0f };
			/** チェックの大きさ(ボックスから少しはみ出すくらい) */
			const Vector2 CHECK_MARK_SIZE = { 44.0f,44.0f };
			/** 行の間隔 */
			constexpr float CHECK_ROW_SPACING = 46.0f;
			/** チェックボックスの真ん中から名前までの横の距離 */
			constexpr float CHECK_NAME_OFFSET_X = 40.0f;
			/** 行の文字のスケール */
			const Vector3 CHECK_ROW_SCALE = { 0.7f,0.7f,1.0f };
			/** 真ん中を基準にする(チェックは大きさが変わるので、真ん中から広げる) */
			const Vector2 PIVOT_CENTER = { 0.5f,0.5f };
			/** 左端の真ん中を基準にする */
			const Vector2 PIVOT_LEFT_CENTER = { 0.0f,0.5f };

			// 色
			/** 照合〇 の色(一致度 100%) */
			const Vector4 COLOR_MATCH = Vector4(0.35f, 0.90f, 0.45f, 1.0f);
			/** 惜しい色(一致度が NEAR_RATE の時) */
			const Vector4 COLOR_NEAR = Vector4(1.0f, 0.80f, 0.25f, 1.0f);
			/** 照合なし× の色(一致度 0%) */
			const Vector4 COLOR_MISS = Vector4(1.0f, 0.30f, 0.30f, 1.0f);
			/** 数えなかった(失敗ではない)色 */
			const Vector4 COLOR_NEUTRAL = Vector4(0.70f, 0.70f, 0.70f, 1.0f);
			/** 惜しい色にする一致度(ここまでは赤 → 黄色に、ここから先は黄色のまま) */
			constexpr float NEAR_RATE = 0.9f;

			// 出てくる動き
			/** 写真が出てくる時間(秒) */
			constexpr float PHOTO_IN_TIME = 0.15f;
			/** 写真が出てくる時の、始めの大きさ(1.0 が JSON の大きさ) */
			constexpr float PHOTO_IN_SCALE = 1.12f;
			/** チェックを付け始める時間(秒。写真が出てから) */
			constexpr float CHECK_START_TIME = 0.2f;
			/** 次の行にチェックを付けるまでの時間(秒) */
			constexpr float CHECK_INTERVAL = 0.08f;
			/** 印が付く時間(秒) */
			constexpr float CHECK_IN_TIME = 0.08f;
			/** 印の、始めの大きさ(1.0 が作った時の大きさ) */
			constexpr float CHECK_IN_SCALE = 1.8f;
			/** 最後のチェックから、ハンコを押し始めるまでの時間(秒) */
			constexpr float STAMP_DELAY = 0.15f;
			/** ハンコを押す時間(秒) */
			constexpr float STAMP_IN_TIME = 0.12f;
			/** ハンコの、始めの大きさ(1.0 が JSON の大きさ。上から押し付ける感じ) */
			constexpr float STAMP_IN_SCALE = 2.2f;
			/** ハンコを押し終わってから、閉じられるようになるまでの時間(秒) */
			constexpr float CLOSE_DELAY = 0.1f;
			/** 「A で続ける」の点滅の速さ(1 秒に何回) */
			constexpr float HINT_BLINK_SPEED = 1.5f;


			/**
			 * @brief 0 ～ 1 に収める
			 * @param value 値
			 * @return 0 ～ 1 にした値
			 */
			float Clamp01(const float value)
			{
				return (std::max)(0.0f, (std::min)(1.0f, value));
			}


			/**
			 * @brief 2 つの色の間の色
			 * @param from 始めの色
			 * @param to 終わりの色
			 * @param rate 0 で from、1 で to
			 * @return 間の色
			 */
			Vector4 MixColor(const Vector4& from, const Vector4& to, const float rate)
			{
				return from + (to - from) * rate;
			}


			/**
			 * @brief 大きさに倍率をかける(奥行きの z はそのまま)
			 * @param scale 元の大きさ
			 * @param rate 倍率
			 * @return かけた大きさ
			 */
			Vector3 MulScale(const Vector3& scale, const float rate)
			{
				return Vector3(scale.x * rate, scale.y * rate, scale.z);
			}


			/**
			 * @brief 大きい所から元の大きさに戻る時の倍率
			 * @param time 始まってからの時間(秒。マイナスなら、まだ始まっていない)
			 * @param inTime 戻るのにかける時間(秒)
			 * @param startScale 始めの倍率
			 * @return 倍率
			 */
			float GetInScale(const float time, const float inTime, const float startScale)
			{
				const float rate = Clamp01(time / inTime);
				return startScale + (1.0f - startScale) * rate;
			}
		}


		JudgeMenu::JudgeMenu()
			: m_photoFrame(nullptr)
			, m_photo(nullptr)
			, m_checkPanel(nullptr)
			, m_checkHeading(nullptr)
			, m_checkBase(nullptr)
			, m_stampMatch(nullptr)
			, m_stampMiss(nullptr)
			, m_rateText(nullptr)
			, m_detailText(nullptr)
			, m_hintText(nullptr)
			, m_photoFrameBaseScale(Vector3::One)
			, m_photoBaseScale(Vector3::One)
			, m_stampBaseScale(Vector3::One)
			, m_timer(0.0f)
		{}


		JudgeMenu::~JudgeMenu()
		{}


		void JudgeMenu::InitializeLogic()
		{
			// ホットリロードでキャンバスが作り直されると、作った行は消えている(キーも要らない)
			m_createdKeyList.clear();
			m_checkRowList.clear();

			// ホットリロードで UI が作り直されたので、探し直す
			// ※ JSON の "type" と型が違うと nullptr になる(その UI は出なくなるので、JSON を確認する)
			m_photoFrame = FindUI<UIFrame>(PHOTO_FRAME_NAME);
			m_photo = FindUI<UIIcon>(PHOTO_NAME);
			m_checkPanel = FindUI<UIFrame>(CHECK_PANEL_NAME);
			m_checkHeading = FindUI<UIText>(CHECK_HEADING_NAME);
			m_checkBase = FindUI<UIBase>(CHECK_BASE_NAME);
			m_stampMatch = FindUI<UIText>(STAMP_MATCH_NAME);
			m_stampMiss = FindUI<UIText>(STAMP_MISS_NAME);
			m_rateText = FindUI<UIText>(RATE_NAME);
			m_detailText = FindUI<UIText>(DETAIL_NAME);
			m_hintText = FindUI<UIText>(HINT_NAME);

			// 動きの基準は JSON に書いた大きさ(作り直した直後なので、まだ動きで変わっていない)
			if (m_photoFrame) m_photoFrameBaseScale = m_photoFrame->GetScale();
			if (m_photo) m_photoBaseScale = m_photo->GetScale();
			if (m_stampMatch) m_stampBaseScale = m_stampMatch->GetScale();

			// 作り直した UI は値を持っていないので、入れ直す
			Apply();
			ApplyAnimation();
		}


		void JudgeMenu::Update()
		{
			// 出てくる動きを進める
			m_timer += g_gameTime->GetFrameDeltaTime();
			ApplyAnimation();

			UIMenuBase::Update();
		}


		void JudgeMenu::Show(const JudgeViewData& viewData)
		{
			m_viewData = viewData;
			m_timer = 0.0f;
			Apply();
			ApplyAnimation();
		}


		bool JudgeMenu::CanClose() const
		{
			return m_timer >= GetStampStartTime() + STAMP_IN_TIME + CLOSE_DELAY;
		}


		void JudgeMenu::Apply()
		{
			const bool isMatch = m_viewData.tone == JudgeTone::Match;
			const Vector4 rateColor = GetRateColor();

			// 写真(絵が無い時は、枠だけ残る)
			if (m_photo)
			{
				if (m_viewData.photo) m_photo->SetTexture(*m_viewData.photo);
				m_photo->SetIsDraw(m_viewData.photo != nullptr);
			}
			// 写真の枠の線を一致度の色にする
			if (m_photoFrame) m_photoFrame->SetBorderColor(rateColor);

			// チェックリスト(項目が無い時は、欄ごと消す)
			const bool hasCheck = !m_viewData.checkList.empty();
			if (m_checkPanel) m_checkPanel->SetIsDraw(hasCheck);
			if (m_checkHeading) m_checkHeading->SetIsDraw(hasCheck);
			CreateCheckRows();

			// ハンコ(どちらか 1 つだけ。文字は JSON に書いた物)
			if (m_stampMatch) m_stampMatch->SetColor(COLOR_MATCH);
			if (m_stampMiss) m_stampMiss->SetColor(m_viewData.tone == JudgeTone::Neutral ? COLOR_NEUTRAL : COLOR_MISS);

			// 一致度(数えなかった時は、比べていないので出さない)
			if (m_rateText)
			{
				const int percent = static_cast<int>(Clamp01(m_viewData.matchRate) * 100.0f + 0.5f);
				const std::wstring rateText = L"一致度 " + std::to_wstring(percent) + L"%";
				m_rateText->SetText(rateText.c_str());
				m_rateText->SetColor(rateColor);
				m_rateText->SetIsDraw(m_viewData.tone != JudgeTone::Neutral);
			}

			// 写真の下の文
			if (m_detailText)
			{
				m_detailText->SetText(m_viewData.detail.c_str());
				m_detailText->SetColor(isMatch ? COLOR_MATCH : UIStyle::Get().subColor);
			}
		}


		void JudgeMenu::CreateCheckRows()
		{
			// 前に作った行を消す
			for (const uint32_t key : m_createdKeyList)
			{
				GetCanvas()->RemoveUI(key);
			}
			m_createdKeyList.clear();
			m_checkRowList.clear();

			if (m_checkBase == nullptr) return;

			const Vector3& basePosition = m_checkBase->GetPosition();
			const Vector4& subColor = UIStyle::Get().subColor;
			for (int i = 0; i < static_cast<int>(m_viewData.checkList.size()); ++i)
			{
				const JudgeCheckData& check = m_viewData.checkList[i];
				const std::string number = std::to_string(i);

				// 基準から下に、行の数だけずらす
				const Vector3 boxPosition(basePosition.x, basePosition.y - CHECK_ROW_SPACING * static_cast<float>(i), basePosition.z);

				// ※ キャンバスは作った順に描くので、ボックス → チェックの順に作る(チェックを手前に出す)
				CheckRow row;
				// 空のチェックボックス(色は、チェックを付ける時に ApplyAnimation が変える)
				row.box = CreateIcon("JudgeCheckBox" + number, CHECK_BOX_ASSET, CHECK_BOX_SIZE, boxPosition, subColor);
				// チェック(合っている項目だけ)
				if (check.isOk)
				{
					row.check = CreateIcon("JudgeCheckMark" + number, CHECK_MARK_ASSET, CHECK_MARK_SIZE, boxPosition, COLOR_MATCH);
				}
				// 項目の名前
				row.name = CreateText("JudgeCheckName" + number, check.name,
					Vector3(boxPosition.x + CHECK_NAME_OFFSET_X, boxPosition.y, boxPosition.z), CHECK_ROW_SCALE, subColor);
				m_checkRowList.push_back(row);
			}
		}


		void JudgeMenu::ApplyAnimation()
		{
			// 写真: 少し大きい所から、元の大きさに戻る
			const float photoScale = GetInScale(m_timer, PHOTO_IN_TIME, PHOTO_IN_SCALE);
			if (m_photoFrame) m_photoFrame->SetScale(MulScale(m_photoFrameBaseScale, photoScale));
			if (m_photo) m_photo->SetScale(MulScale(m_photoBaseScale, photoScale));

			// チェックリスト: 空のボックスを先に全部出しておき、上から順にチェックを付けていく
			// 合っている項目 → チェックが大きい所からポンと付く
			// 合っていない項目 → チェックは付かず、ボックスと名前が赤くなる
			const Vector4& subColor = UIStyle::Get().subColor;
			for (int i = 0; i < static_cast<int>(m_checkRowList.size()) && i < static_cast<int>(m_viewData.checkList.size()); ++i)
			{
				CheckRow& row = m_checkRowList[i];
				const bool isOk = m_viewData.checkList[i].isOk;
				const float rowTime = m_timer - (CHECK_START_TIME + CHECK_INTERVAL * static_cast<float>(i));
				const bool isChecked = rowTime >= 0.0f;

				if (row.check)
				{
					row.check->SetIsDraw(isChecked);
					row.check->SetScale(MulScale(Vector3::One, GetInScale(rowTime, CHECK_IN_TIME, CHECK_IN_SCALE)));
				}
				const Vector4& rowColor = (isChecked && !isOk) ? COLOR_MISS : subColor;
				if (row.box) row.box->SetColor(rowColor);
				if (row.name) row.name->SetColor(rowColor);
			}

			// ハンコ: チェックが全部付いた後に、大きい所から押し付ける
			const float stampTime = m_timer - GetStampStartTime();
			const bool isStampStarted = stampTime >= 0.0f;
			const float stampScale = GetInScale(stampTime, STAMP_IN_TIME, STAMP_IN_SCALE);
			const bool isMatch = m_viewData.tone == JudgeTone::Match;
			if (m_stampMatch)
			{
				m_stampMatch->SetIsDraw(isStampStarted && isMatch);
				m_stampMatch->SetScale(MulScale(m_stampBaseScale, stampScale));
			}
			if (m_stampMiss)
			{
				m_stampMiss->SetIsDraw(isStampStarted && !isMatch);
				m_stampMiss->SetScale(MulScale(m_stampBaseScale, stampScale));
			}

			// 一致度と文: ハンコと一緒に出す(先に出ると、チェックを見る前に結果が分かってしまう)
			if (m_rateText) m_rateText->SetIsDraw(isStampStarted && m_viewData.tone != JudgeTone::Neutral);
			if (m_detailText) m_detailText->SetIsDraw(isStampStarted);

			// 「A で続ける」: 閉じられるようになったら、ゆっくり点滅させる
			if (m_hintText)
			{
				const bool canClose = CanClose();
				m_hintText->SetIsDraw(canClose);
				if (canClose)
				{
					const float wave = std::sin(m_timer * HINT_BLINK_SPEED * 2.0f * Math::PI);
					Vector4 color = UIStyle::Get().subColor;
					color.w *= 0.6f + 0.4f * (wave * 0.5f + 0.5f);
					m_hintText->SetColor(color);
				}
			}
		}


		float JudgeMenu::GetStampStartTime() const
		{
			// チェックが無い時は、写真が出たらすぐ押す
			const int rowCount = static_cast<int>(m_viewData.checkList.size());
			if (rowCount == 0) return CHECK_START_TIME;
			return CHECK_START_TIME + CHECK_INTERVAL * static_cast<float>(rowCount - 1) + CHECK_IN_TIME + STAMP_DELAY;
		}


		Vector4 JudgeMenu::GetRateColor() const
		{
			switch (m_viewData.tone)
			{
			case JudgeTone::Match:   return COLOR_MATCH;
			case JudgeTone::Neutral: return COLOR_NEUTRAL;
			default: break;
			}

			// NG: 一致度が低いほど赤、高いほど黄色(惜しい)
			const float rate = Clamp01(m_viewData.matchRate / NEAR_RATE);
			return MixColor(COLOR_MISS, COLOR_NEAR, rate);
		}


		UIText* JudgeMenu::CreateText(const std::string& name, const std::wstring& text, const Vector3& position, const Vector3& scale, const Vector4& color)
		{
			const uint32_t key = Hash32(name.c_str());
			UIText* ui = GetCanvas()->CreateUI<UIText>(key);
			ui->Initialize(text.c_str(), position, scale, Quaternion::Identity, color, PIVOT_LEFT_CENTER);
			m_createdKeyList.push_back(key);
			return ui;
		}


		UIIcon* JudgeMenu::CreateIcon(const std::string& name, const char* assetName, const Vector2& size, const Vector3& position, const Vector4& color)
		{
			const uint32_t key = Hash32(name.c_str());
			UIIcon* ui = GetCanvas()->CreateUI<UIIcon>(key);
			ui->Initialize(assetName, size.x, size.y, position, Vector3::One, Quaternion::Identity, color, PIVOT_CENTER);
			m_createdKeyList.push_back(key);
			return ui;
		}
	}
}
