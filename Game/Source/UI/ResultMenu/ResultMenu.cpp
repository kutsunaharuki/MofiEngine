/**
 * @file ResultMenu.cpp
 * @brief リザルトの UI(写真アルバム風)
 */
#include "stdafx.h"
#include "ResultMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** UI の名前(JSON の "name") */
			constexpr const char* TITLE_NAME = "ResultTitle";
			constexpr const char* MISSION_BASE_NAME = "ResultMissionBase";
			constexpr const char* PHOTO_BASE_NAME = "ResultPhotoBase";
			constexpr const char* EMPTY_NAME = "ResultEmpty";
			constexpr const char* RETRY_NAME = "ResultRetry";
			constexpr const char* TITLE_SELECT_NAME = "ResultToTitle";
			/** 枠などに使う白い画像 */
			constexpr const char* WHITE_ASSET = "Assets/spriteData/UI/Common/White.DDS";

			// ミッションの行
			/** 行の間隔 */
			constexpr float MISSION_ROW_SPACING = 45.0f;
			/** 行の文字のスケール */
			const Vector3 MISSION_ROW_SCALE = { 0.7f,0.7f,1.0f };

			// 写真
			/** 横に並べる枚数 */
			constexpr int PHOTO_COLUMN = 4;
			/** 写真の大きさ(16:9) */
			const Vector2 PHOTO_SIZE = { 320.0f,180.0f };
			/** 写真の横の間隔(写真の左端から次の左端まで) */
			constexpr float PHOTO_SPACING_X = 360.0f;
			/** 写真の縦の間隔(写真の上端から次の上端まで。下に結果の文字が入る) */
			constexpr float PHOTO_SPACING_Y = 290.0f;
			/** 写真のまわりの枠の太さ */
			constexpr float PHOTO_FRAME_WIDTH = 6.0f;
			/** 写真の下端から、結果の文字までの距離 */
			constexpr float PHOTO_LABEL_OFFSET_Y = 12.0f;
			/** 結果の文字のスケール */
			const Vector3 PHOTO_LABEL_SCALE = { 0.5f,0.5f,1.0f };

			// 色
			/** 成功・達成の色 */
			const Vector4 COLOR_SUCCESS = Vector4(1.0f, 0.85f, 0.2f, 1.0f);
			/** 失敗の色 */
			const Vector4 COLOR_FAILED = Vector4(1.0f, 0.35f, 0.3f, 1.0f);
			/** 数えなかった(失敗ではない)色 */
			const Vector4 COLOR_NEUTRAL = Vector4(0.65f, 0.65f, 0.65f, 1.0f);
			/** 写真そのままの色 */
			const Vector4 COLOR_WHITE = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

			/** 左上を基準にする */
			const Vector2 PIVOT_LEFT_TOP = { 0.0f,1.0f };
			/** 左端の真ん中を基準にする */
			const Vector2 PIVOT_LEFT_CENTER = { 0.0f,0.5f };


			/**
			 * @brief 見え方から色を決める(色は View の持ち物)
			 * @param tone 見え方
			 * @return 色
			 */
			const Vector4& GetToneColor(const ResultTone tone)
			{
				switch (tone)
				{
				case ResultTone::Success: return COLOR_SUCCESS;
				case ResultTone::Failed:  return COLOR_FAILED;
				default:                  return COLOR_NEUTRAL;
				}
			}
		}


		ResultMenu::ResultMenu()
			: m_titleText(nullptr)
			, m_missionBase(nullptr)
			, m_photoBase(nullptr)
			, m_emptyText(nullptr)
			, m_selectButtonList{}
			, m_selectType(SelectType::Retry)
			, m_hasViewData(false)
		{}


		ResultMenu::~ResultMenu()
		{}


		void ResultMenu::InitializeLogic()
		{
			// ホットリロードでキャンバスが作り直されると、作った UI は消えている(キーも要らない)
			m_createdKeyList.clear();

			// ※ JSON の "type" と型が違うと nullptr になる(その UI は出なくなるので、JSON を確認する)
			m_titleText = FindUI<UIText>(TITLE_NAME);
			m_missionBase = FindUI<UIBase>(MISSION_BASE_NAME);
			m_photoBase = FindUI<UIBase>(PHOTO_BASE_NAME);
			m_emptyText = FindUI<UIText>(EMPTY_NAME);
			m_selectButtonList[static_cast<size_t>(SelectType::Retry)] = FindUI<UIButton>(RETRY_NAME);
			m_selectButtonList[static_cast<size_t>(SelectType::Title)] = FindUI<UIButton>(TITLE_SELECT_NAME);
			ApplySelect();

			// 値を設定した後(ホットリロード)なら、ここで作り直す
			CreateResultUI();
		}


		void ResultMenu::SetViewData(const ResultViewData& viewData)
		{
			m_viewData = viewData;
			m_hasViewData = true;
			CreateResultUI();
		}


		void ResultMenu::SetSelect(const SelectType type)
		{
			m_selectType = type;
			ApplySelect();
		}


		void ResultMenu::CreateResultUI()
		{
			RemoveCreatedUI();
			if (!m_hasViewData) return;

			if (m_titleText)
			{
				m_titleText->SetText(m_viewData.title.text.c_str());
				m_titleText->SetColor(GetToneColor(m_viewData.title.tone));
			}

			CreateMissionRows();
			CreatePhotos();
		}


		void ResultMenu::RemoveCreatedUI()
		{
			for (const uint32_t key : m_createdKeyList)
			{
				GetCanvas()->RemoveUI(key);
			}
			m_createdKeyList.clear();
		}


		void ResultMenu::CreateMissionRows()
		{
			if (m_missionBase == nullptr) return;

			const Vector3& basePosition = m_missionBase->GetPosition();
			for (int i = 0; i < static_cast<int>(m_viewData.missionRowList.size()); ++i)
			{
				const ResultTextData& row = m_viewData.missionRowList[i];

				// 基準から下に、行の数だけずらす
				const Vector3 position(basePosition.x, basePosition.y - MISSION_ROW_SPACING * static_cast<float>(i), basePosition.z);
				CreateText("ResultMission" + std::to_string(i), row.text, position, MISSION_ROW_SCALE, GetToneColor(row.tone), PIVOT_LEFT_CENTER);
			}
		}


		void ResultMenu::CreatePhotos()
		{
			// 写真が無い時だけ「写真はありません」を出す
			if (m_emptyText) m_emptyText->SetIsDraw(m_viewData.photoList.empty());
			if (m_photoBase == nullptr) return;

			const Vector3& basePosition = m_photoBase->GetPosition();
			for (int i = 0; i < static_cast<int>(m_viewData.photoList.size()); ++i)
			{
				const ResultPhotoData& photoData = m_viewData.photoList[i];

				// 左上から、横に PHOTO_COLUMN 枚ずつ並べる
				const int column = i % PHOTO_COLUMN;
				const int row = i / PHOTO_COLUMN;
				const Vector3 photoPosition(
					basePosition.x + PHOTO_SPACING_X * static_cast<float>(column),
					basePosition.y - PHOTO_SPACING_Y * static_cast<float>(row),
					basePosition.z);

				const Vector4& toneColor = GetToneColor(photoData.tone);
				const std::string number = std::to_string(i);

				// 枠(写真より少し大きい板を、写真の後ろに置く)
				// ※ キャンバスは作った順に描くので、枠を先に作る
				CreateIcon("ResultPhotoFrame" + number, WHITE_ASSET,
					Vector2(PHOTO_SIZE.x + PHOTO_FRAME_WIDTH * 2.0f, PHOTO_SIZE.y + PHOTO_FRAME_WIDTH * 2.0f),
					Vector3(photoPosition.x - PHOTO_FRAME_WIDTH, photoPosition.y + PHOTO_FRAME_WIDTH, photoPosition.z),
					toneColor, PIVOT_LEFT_TOP);

				// 写真(絵が無い時は、枠だけ残る)
				UIIcon* photo = CreateIcon("ResultPhoto" + number, "", PHOTO_SIZE, photoPosition, COLOR_WHITE, PIVOT_LEFT_TOP);
				if (photoData.texture)
				{
					photo->SetTexture(*photoData.texture);
				}

				// 結果の文字(写真の下)
				const Vector3 labelPosition(photoPosition.x, photoPosition.y - PHOTO_SIZE.y - PHOTO_FRAME_WIDTH - PHOTO_LABEL_OFFSET_Y, photoPosition.z);
				CreateText("ResultPhotoLabel" + number, photoData.label, labelPosition, PHOTO_LABEL_SCALE, toneColor, PIVOT_LEFT_TOP);
			}
		}


		void ResultMenu::ApplySelect()
		{
			for (int i = 0; i < static_cast<int>(m_selectButtonList.size()); ++i)
			{
				UIButton* button = m_selectButtonList[i];
				if (button == nullptr) continue;

				// 色と大きさは、ボタンが形に合わせて変える(全部の画面で同じ見た目)
				const bool isSelected = i == static_cast<int>(m_selectType);
				button->SetState(isSelected ? UIButton::ButtonState::Selected : UIButton::ButtonState::Normal);
			}
		}


		void ResultMenu::PressSelect()
		{
			UIButton* button = m_selectButtonList[static_cast<size_t>(m_selectType)];
			if (button) button->Press();
		}


		UIText* ResultMenu::CreateText(const std::string& name, const std::wstring& text, const Vector3& position, const Vector3& scale, const Vector4& color, const Vector2& pivot)
		{
			const uint32_t key = Hash32(name.c_str());
			UIText* ui = GetCanvas()->CreateUI<UIText>(key);
			ui->Initialize(text.c_str(), position, scale, Quaternion::Identity, color, pivot);
			m_createdKeyList.push_back(key);
			return ui;
		}


		UIIcon* ResultMenu::CreateIcon(const std::string& name, const char* assetName, const Vector2& size, const Vector3& position, const Vector4& color, const Vector2& pivot)
		{
			const uint32_t key = Hash32(name.c_str());
			UIIcon* ui = GetCanvas()->CreateUI<UIIcon>(key);
			ui->Initialize(assetName, size.x, size.y, position, Vector3::One, Quaternion::Identity, color, pivot);
			m_createdKeyList.push_back(key);
			return ui;
		}
	}
}
