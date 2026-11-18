/**
 * @file PhotoMatchDebugWindow.cpp
 * @brief 照合のデバッグ表示(今の写り方を出して、見本の数値を決める道具)
 */
#include "stdafx.h"
#include "PhotoMatchDebugWindow.h"
#include "imgui.h"
#include "Source/Util/ParameterManager.h"
#include "Source/Util/ScreenPosition.h"
#include "Source/Actor/Character/Enemy/EnemyBase.h"
#include "Source/Mission/ShotMeasure.h"
#include "Source/Manager/MissionManager.h"
#include <cstdio>


namespace app
{
	namespace debug
	{
		namespace
		{
			/** 顔の向きの矢印の長さ */
			constexpr float ARROW_LENGTH = 50.0f;
			/** 顔の向きの矢印の太さ */
			constexpr float ARROW_THICKNESS = 3.0f;
			/** 顔の向きの矢印の色(赤) */
			const ImU32 ARROW_COLOR = IM_COL32(255, 60, 60, 255);
			/** 画面の半分(UI の座標は真ん中が原点なので、左上を原点にするときに足す) */
			constexpr float HALF = 0.5f;
			/** クリップボードに入れる文字の最大の長さ */
			constexpr int SAMPLE_TEXT_SIZE = 256;


			/**
			 * @brief UI の座標を、ImGui の画面の座標にする
			 * @param uiPosition UI の座標(画面の真ん中が原点・上が +Y・1920×1080)
			 * @return ImGui の画面の座標(左上が原点・下が +Y・ウィンドウのピクセル)
			 */
			ImVec2 ToImGuiPosition(const Vector2& uiPosition)
			{
				const ImGuiIO& io = ImGui::GetIO();
				// ※ UI_SPACE_WIDTH は UINT(符号なし)なので、float にしてから割る
				const float rateX = uiPosition.x / static_cast<float>(UI_SPACE_WIDTH) + HALF;
				const float rateY = HALF - uiPosition.y / static_cast<float>(UI_SPACE_HEIGHT);
				return ImVec2(rateX * io.DisplaySize.x, rateY * io.DisplaySize.y);
			}


			/** 照合の項目の名前(MatchItem と同じ順番) */
			const char* MATCH_ITEM_NAME_LIST[] =
			{
				"Target",
				"Stealing",
				"Focus",
				"FaceAngle",
				"FaceBlocked",
				"FaceInFrame",
				"Yaw",
				"Pitch",
				"Size",
				"Position",
			};

			// MATCH_ITEM_NAME_LIST の数と、MatchItem の数が合っているかをコンパイル時にチェック
			static_assert(sizeof(MATCH_ITEM_NAME_LIST) / sizeof(MATCH_ITEM_NAME_LIST[0]) == static_cast<size_t>(mission::MatchItem::Max),
				"MATCH_ITEM_NAME_LIST と MatchItem の数が合っていません。");

			/** OK の色(緑) */
			const ImVec4 OK_COLOR = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
			/** NG の色(赤) */
			const ImVec4 NG_COLOR = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
		}


		void PhotoMatchDebugWindow::Draw(actor::EnemyBase* focusEnemy, Camera& finderCamera, const btCollisionObject* photographer, const mission::MissionManager& missionManager)
		{
#ifdef APP_ENABLE_PARAMETER_HOTRELOAD
			ImGui::Begin("Photo Match");

			// 最後に撮った写真の照合結果(ピントの被写体がいなくても出す)
			DrawLastShot(missionManager);

			// ピントの被写体が悪党でなければ、測るものが無い
			if (focusEnemy == nullptr)
			{
				ImGui::TextDisabled("No enemy in focus.");
				ImGui::End();
				return;
			}

			// 今の写り方を毎フレーム測る(撮った時と同じ関数なので、ここで見た値がそのまま判定に使われる)
			mission::ShotMeasure measure =  mission::MeasureShot(*focusEnemy, finderCamera, photographer);
			if (!measure.isMeasured)
			{
				ImGui::TextDisabled("Head bone not found. (headBone in the enemy JSON)");
				ImGui::End();
				return;
			}

			// 顔が見えているか
			ImGui::SeparatorText("Face");
			ImGui::Text("Angle    : %.1f deg  (0 = front, 180 = back)", measure.faceAngleDeg);
			ImGui::Text("Blocked  : %s", measure.isFaceBlocked ? "yes" : "no");
			ImGui::Text("In front : %s", measure.isFaceInFront ? "yes" : "no");
			ImGui::Text("Screen   : %.0f, %.0f", measure.faceScreenPosition.x, measure.faceScreenPosition.y);

			// 角度・大きさ・位置(見本と比べる値)
			ImGui::SeparatorText("Sample values");
			ImGui::Text("Yaw      : %.1f deg", measure.yawDeg);
			ImGui::Text("Pitch    : %.1f deg", measure.pitchDeg);
			ImGui::Text("Size     : %.3f", measure.size);
			ImGui::Text("Position : %.3f, %.3f", measure.position.x, measure.position.y);

			// 今の値を、MissionList.json の "sample" の形でクリップボードへ(貼るだけで見本になる)
			if (ImGui::Button("Copy as sample"))
			{
				char text[SAMPLE_TEXT_SIZE];
				sprintf_s(text, "\"sample\":{ \"yawDeg\":%.1f, \"pitchDeg\":%.1f, \"size\":%.3f, \"position\":[%.3f, %.3f] }",
					measure.yawDeg, measure.pitchDeg, measure.size, measure.position.x, measure.position.y);
				ImGui::SetClipboardText(text);
			}
			ImGui::SameLine();
			ImGui::TextDisabled("-> paste into MissionList.json");

			ImGui::End();

			// 顔の向きの矢印(ゲームの画面に描く)
			DrawFaceArrow(*focusEnemy);
#else
			// Release では何もしない(引数を使っていない警告を出さない)
			(void)focusEnemy;
			(void)finderCamera;
			(void)photographer;
			(void)missionManager;
#endif
		}


		void PhotoMatchDebugWindow::DrawLastShot(const mission::MissionManager& missionManager)
		{
			ImGui::SeparatorText("Last shot");

			// 悪党が写っていない・同じ盗みを撮り直した時は、照合していない
			if (!missionManager.HasLastMatchResult())
			{
				ImGui::TextDisabled("No match result. (no enemy / same steal)");
				return;
			}

			const mission::MatchResult& result = missionManager.GetLastMatchResult();
			if (result.IsSuccess())
			{
				ImGui::TextColored(OK_COLOR, "MATCH OK");
			}
			else
			{
				ImGui::TextColored(NG_COLOR, "MATCH NG");
			}

			// 項目ごとの合否とずれ(NG の項目は全部出す)
			for (int i = 0; i < static_cast<int>(mission::MatchItem::Max); ++i)
			{
				const bool isSuccess = result.isSuccess[i];
				ImGui::TextColored(isSuccess ? OK_COLOR : NG_COLOR, "%-12s %s  diff %.2f",
					MATCH_ITEM_NAME_LIST[i], isSuccess ? "OK" : "NG", result.diff[i]);
			}
		}


		void PhotoMatchDebugWindow::DrawFaceArrow(actor::EnemyBase& enemy)
		{
			Vector3 headPosition;
			Vector3 headForward;
			if (!enemy.GetHead(headPosition, headForward)) return;

			// 頭の位置と、そこから顔の前へ少し進んだ位置を、ゲームのカメラで画面に写す
			Vector2 from;
			Vector2 to;
			const bool isFromVisible = util::CalcScreenPosition(*g_camera3D, headPosition, from);
			const bool isToVisible = util::CalcScreenPosition(*g_camera3D, headPosition + headForward * ARROW_LENGTH, to);
			// どちらかがカメラの後ろなら描かない(画面の外に変な線が出る)
			if (!isFromVisible || !isToVisible) return;

			// ウィンドウより後ろ(ゲームの画面の上)に描く
			ImGui::GetBackgroundDrawList()->AddLine(ToImGuiPosition(from), ToImGuiPosition(to), ARROW_COLOR, ARROW_THICKNESS);
		}
	}
}
