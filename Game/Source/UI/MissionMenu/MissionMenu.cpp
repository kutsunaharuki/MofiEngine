/**
 * @file MissionMenu.cpp
 * @brief ミッションメニュー
 */
#include "stdafx.h"
#include "MissionMenu.h"
#include "Source/Manager/MissionManager.h"


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
			/** 文の中で必要な枚数に置き換える目印 */
			constexpr const wchar_t* COUNT_TAG = L"{count}";

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
			 * @brief 表示する文を作る関数
			 * @param mission ミッションデータ
			 * @return マーク付きの文({count} は必要な枚数に置き換えて、後ろに今の枚数を付ける)
			 */
			std::wstring MakeMissionText(const mission::MissionData& mission)
			{
				std::wstring text = mission.text;

				const size_t tagPos = text.find(COUNT_TAG);
				if (tagPos != std::wstring::npos)
				{
					text.replace(tagPos, wcslen(COUNT_TAG), std::to_wstring(mission.requiredCount));
				}

				// 今の枚数(例: (1/3))
				const std::wstring progress = L" (" + std::to_wstring(mission.currentCount) + L"/" + std::to_wstring(mission.requiredCount) + L")";

				return MISSION_TEXT_MARK + text + progress;
			}


			/**
			 * @brief ミッションの状態から、行の色を決める
			 * @param state ミッションの状態
			 * @return 行の色
			 */
			const Vector4& GetMissionColor(const mission::MissionState state)
			{
				switch (state)
				{
				case mission::MissionState::Achievement: return COLOR_ACHIEVEMENT;
				case mission::MissionState::Failed:      return COLOR_FAILED;
				default:                                 return COLOR_PROGRESS;
				}
			}
		}


		MissionMenu::MissionMenu()
			: m_mainUnderLine(nullptr)
			, m_titleText(nullptr)
			, m_missionManager(nullptr)
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

			// マネージャーを設定した後(ホットリロード)なら、ここで行を作り直す
			CreateMissionTexts();
		}


		void MissionMenu::CreateMissionTexts()
		{
			// ミッションの目次とミッションマネージャーが nullptr なら処理しない
			if (!m_titleText || !m_missionManager) return;

			// 複数個作ると、前のテキストが残ってしまうため、作る前に消す
			// ※ ポインタは使えなくなっていることがあるので、名前からキーを作り直して消す
			for (int i = 0; i < static_cast<int>(m_missionTextList.size()); ++i)
			{
				// キャンバスから作っているテキストだけ消す
				GetCanvas()->RemoveUI(Hash32(MakeMissionTextName(i).c_str()));
			}
			m_missionTextList.clear();


			const std::vector<mission::MissionData>& missionList = m_missionManager->GetMissionList();
			const Vector3& titlePosition = m_titleText->GetPosition();
			for (int i = 0; i < static_cast<int>(missionList.size()); ++i)
			{
				// ミッションから行を作る
				const mission::MissionData& mission = missionList[i];

				// 名前からキーを作って、キャンバスにテキストを作る
				UIText* text = GetCanvas()->CreateUI<UIText>(Hash32(MakeMissionTextName(i).c_str()));

				// タイトルの位置から、右に少し、下に行の数だけずらす(0 行目がタイトルと重ならないように i + 1)
				const Vector3 position(
					titlePosition.x + MISSION_TEXT_OFFSET_X,
					titlePosition.y - MISSION_TEXT_SPACING * static_cast<float>(i + 1),
					titlePosition.z);

				const std::wstring missionText = MakeMissionText(mission);
				text->Initialize(
					missionText.c_str(),
					position,
					MISSION_TEXT_SCALE,
					Quaternion::Identity,
					GetMissionColor(mission.state),
					MISSION_TEXT_PIVOT);

				// 一覧に入れる(達成・失敗の色を変えるときに使う)
				m_missionTextList.push_back(text);
			}
		}


		void MissionMenu::UpdateMissionTexts()
		{
			if (!m_missionManager) return;

			const std::vector<mission::MissionData>& missionList = m_missionManager->GetMissionList();
			// 行とミッションの数が違う時は、作り直す
			if (missionList.size() != m_missionTextList.size())
			{
				CreateMissionTexts();
				return;
			}

			for (int i = 0; i < static_cast<int>(missionList.size()); ++i)
			{
				const mission::MissionData& mission = missionList[i];
				m_missionTextList[i]->SetText(MakeMissionText(mission).c_str());
				m_missionTextList[i]->SetColor(GetMissionColor(mission.state));
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


		void MissionMenu::SetMissionManager(const mission::MissionManager* missionManager)
		{
			m_missionManager = missionManager;
			// マネージャーが来たので、行を作る
			CreateMissionTexts();
		}
	}
}
