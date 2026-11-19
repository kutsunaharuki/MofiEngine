/**
 * @file MissionMenu.cpp
 * @brief ミッションメニュー
 */
#include "stdafx.h"
#include "MissionMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** タイトルの名前(JSON の "name") */
			constexpr const char* MISSION_TITLE_NAME = "MissionTitle";
			/** 下線の名前(JSON の "name") */
			constexpr const char* MISSION_UNDER_LINE_NAME = "MissionTitleLine";

			/** タイトルの中心から下線までの距離 */
			constexpr float MISSION_TITLE_LINE_OFFSET_Y = 20.0f;
			/** 行の名前の頭 */
			constexpr const char* MISSION_TEXT_NAME = "MissionText";
			/** 親から子にかけてずらす値 */
			constexpr float MISSION_TEXT_OFFSET_X = 20.0f;
			/** テキストの間隔(今回は行なので縦) */
			constexpr float MISSION_TEXT_SPACING = 40.0f;
			/** テキストのスケール */
			const Vector3 MISSION_TEXT_SCALE = { 0.7f,0.7f,1.0f };
			/** テキストの基底軸(左端をそろえる) */
			const Vector2 MISSION_TEXT_PIVOT = { 0.0f,0.5f };
			/** ミッション文のマーク(● ○ はフォントに無いので 〇 を使う) */
			constexpr const wchar_t* MISSION_TEXT_MARK = L"〇";

			/** 進行中の色 */
			const Vector4 COLOR_PROGRESS = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			/** 達成した色 */
			const Vector4 COLOR_ACHIEVEMENT = Vector4(1.0f, 0.85f, 0.2f, 1.0f);
			/** 失敗した色 */
			const Vector4 COLOR_FAILED = Vector4(0.6f, 0.6f, 0.6f, 1.0f);


			/**
			 * @brief 行の名前を作る
			 * @param index 何行目か
			 * @return 行の名前(MissionText0 など)
			 */
			std::string MakeMissionTextName(const int index)
			{
				return MISSION_TEXT_NAME + std::to_string(index);
			}


			/**
			 * @brief 行の見え方から、色を決める(色は View の持ち物)
			 * @param tone 行の見え方
			 * @return 行の色
			 */
			const Vector4& GetToneColor(const MissionRowTone tone)
			{
				switch (tone)
				{
				case MissionRowTone::Achievement: return COLOR_ACHIEVEMENT;
				case MissionRowTone::Failed:      return COLOR_FAILED;
				default:                          return COLOR_PROGRESS;
				}
			}
		}


		MissionMenu::MissionMenu()
			: m_mainUnderLine(nullptr)
			, m_titleText(nullptr)
		{}


		MissionMenu::~MissionMenu()
		{}


		void MissionMenu::InitializeLogic()
		{
			// ホットリロードでキャンバスが作り直されると、持っていた行は消えている(ポインタも使えない)
			m_missionTextList.clear();
			// タイトルのテキストを探す
			m_titleText = FindUI<UIText>(MISSION_TITLE_NAME);
			// 下線を探す
			m_mainUnderLine = FindUI<UIIcon>(MISSION_UNDER_LINE_NAME);
			// 下線の長さと位置を、タイトルに合わせる
			FitTitleLine();

			// 行を設定した後(ホットリロード)なら、ここで作り直す
			CreateMissionTexts();
		}


		void MissionMenu::SetMissionRows(const std::vector<MissionRowData>& rowList)
		{
			m_rowDataList = rowList;
			UpdateMissionTexts();
		}


		void MissionMenu::CreateMissionTexts()
		{
			// ミッションの目次が nullptr なら処理しない
			if (!m_titleText) return;

			// 複数個作ると、前のテキストが残ってしまうため、作る前に消す
			// ※ ポインタは使えなくなっていることがあるので、名前からキーを作り直して消す
			for (int i = 0; i < static_cast<int>(m_missionTextList.size()); ++i)
			{
				// キャンバスから作っているテキストだけ消す
				GetCanvas()->RemoveUI(Hash32(MakeMissionTextName(i).c_str()));
			}
			m_missionTextList.clear();

			const Vector3& titlePosition = m_titleText->GetPosition();
			for (int i = 0; i < static_cast<int>(m_rowDataList.size()); ++i)
			{
				const MissionRowData& row = m_rowDataList[i];

				// 名前からキーを作って、キャンバスにテキストを作る
				UIText* text = GetCanvas()->CreateUI<UIText>(Hash32(MakeMissionTextName(i).c_str()));

				// タイトルの位置から、右に少し、下に行の数だけずらす(0 行目がタイトルと重ならないように i + 1)
				const Vector3 position(
					titlePosition.x + MISSION_TEXT_OFFSET_X,
					titlePosition.y - MISSION_TEXT_SPACING * static_cast<float>(i + 1),
					titlePosition.z);

				const std::wstring missionText = MISSION_TEXT_MARK + row.text;
				text->Initialize(
					missionText.c_str(),
					position,
					MISSION_TEXT_SCALE,
					Quaternion::Identity,
					GetToneColor(row.tone),
					MISSION_TEXT_PIVOT);

				// 一覧に入れる(達成・失敗の色を変えるときに使う)
				m_missionTextList.push_back(text);
			}
		}


		void MissionMenu::UpdateMissionTexts()
		{
			// 行とミッションの数が違う時は、作り直す
			if (m_rowDataList.size() != m_missionTextList.size())
			{
				CreateMissionTexts();
				return;
			}

			for (int i = 0; i < static_cast<int>(m_rowDataList.size()); ++i)
			{
				const MissionRowData& row = m_rowDataList[i];
				m_missionTextList[i]->SetText((MISSION_TEXT_MARK + row.text).c_str());
				m_missionTextList[i]->SetColor(GetToneColor(row.tone));
			}
		}


		void MissionMenu::FitTitleLine()
		{
			if (!m_titleText || !m_mainUnderLine) return;

			// タイトルの見た目の横幅 = 文字列の大きさ(スケール前) × スケール
			const float titleWidth = m_titleText->GetSize().x * m_titleText->GetScale().x;

			// 線はタイトルの左端から、少し下に置く
			const Vector3& titlePosition = m_titleText->GetPosition();
			m_mainUnderLine->SetPosition(Vector3(titlePosition.x, titlePosition.y - MISSION_TITLE_LINE_OFFSET_Y, titlePosition.z));

			// 画像の横幅を 1 にしているので、スケールの x がそのまま線の長さになる
			const Vector3 lineScale = m_mainUnderLine->GetScale();
			m_mainUnderLine->SetScale(Vector3(titleWidth, lineScale.y, lineScale.z));
		}
	}
}
