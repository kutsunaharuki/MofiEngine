/**
 * @file NavigationDebugWindow.cpp
 * @brief 経路探索のデバッグ表示
 */
#include "stdafx.h"
#include "NavigationDebugWindow.h"
#include "imgui.h"
#include "Source/Util/ParameterManager.h"
#include "Source/Actor/Character/Boy/Boy.h"
#include "Source/Actor/Character/Enemy/EnemyBase.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyDestinationMode.h"
#include "Source/Actor/Character/Enemy/EnemyAlertMode.h"
#include "Source/Actor/StageObject/StageObject.h"
#include "Source/Stage/Stage.h"
#include "Source/Stage/StageLayout.h"
#include "Source/AI/Navigation/NavGrid.h"
#include "Source/AI/Navigation/PathFollower.h"
#include "Source/AI/WeightedRandom.h"
#include "Source/AI/Vision.h"
#include <cmath>


namespace app
{
	namespace debug
	{
		namespace
		{
			/** 経路探索のミニマップで、1マスのを何ピクセルで描くか */
			constexpr float NAV_DEBUG_PIXEL_PER_CELL = 6.0f;
			/** マスにカーソルを当てた時に塞いでいる置物を何個まで出すか */
			constexpr int NAV_DEBUG_BLOCKER_MAX = 10;
		}


		NavigationDebugWindow::NavigationDebugWindow()
		{}


		NavigationDebugWindow::~NavigationDebugWindow()
		{}


		void NavigationDebugWindow::Initialize(const Target& target)
		{
			m_target = target;
		}


		void NavigationDebugWindow::Draw()
		{
#ifdef APP_ENABLE_PARAMETER_HOTRELOAD

			if (m_target.boy == nullptr || m_target.enemyList == nullptr || m_target.navGrid == nullptr
				|| m_target.stage == nullptr || m_target.stageLayout == nullptr || m_target.enemyController == nullptr)
			{
				return;
			}

			// 敵の一覧(何度も使うので、名前を短くしておく)
			std::vector<std::unique_ptr<actor::EnemyBase>>& enemyList = *m_target.enemyList;

			ImGui::Begin("Navigation");

			/** 1. 操作ボタン */
			if (ImGui::Button("Move enemies to Boy"))
			{
				// 敵を主人公の今の位置へ向かわせる
				const Vector3& boyPosition = m_target.boy->GetTransform().GetWorldPosition();
				m_message.clear();
				for (int i = 0; i < static_cast<int>(enemyList.size()); ++i)
				{
					const bool isFound = enemyList[i]->GetEnemyStateMachine()->MoveTo(boyPosition);
					m_message += "Enemy" + std::to_string(i) + (isFound ? ": path found " : ": NO PATH");
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Stop"))
			{
				for (auto& enemy : enemyList)
				{
					enemy->GetEnemyStateMachine()->StopPathFollow();
				}

				m_message = "Stopped.";
			}
			ImGui::SameLine();
			// ステージから美術品を探し直す(エディタで置物を動かした後に押す)
			if (ImGui::Button("Collect art"))
			{
				const int count = m_target.enemyController->CollectArtTargets(*m_target.stage);
				m_message = "Art targets: " + std::to_string(count);
			}
			ImGui::SameLine();
			// 主人公の今の位置に美術品の目印を置く / 全部消す
			if (ImGui::Button("Add target at Boy"))
			{
				m_target.enemyController->AddTarget(m_target.boy->GetTransform().GetWorldPosition());
				m_message = "Targets: " + std::to_string(m_target.enemyController->GetTargetCount());
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear targets"))
			{
				m_target.enemyController->ClearTargets();
				m_message = "Targets cleared.";
			}
			ImGui::SameLine();

			// エディタで置物を動かした後に押す(地図が古いままだと、敵が棚に突っ込む)
			if (ImGui::Button("Rebuild NavGrid"))
			{
				m_target.navGrid->Build(*m_target.stage);
				m_message = "Rebuilt.";
			}
			ImGui::SameLine();
			
			// 重み付きくじ引きを 10000 回引いて、重み通りの割合になるか確認
			if (ImGui::Button("Test hide lottery"))
			{
				// std::random_device の {} は random_device を 1つ作る
				// () は 作ったものを呼ぶ → ランダムな数が 1つ出る
				// ボタンを押すたびに乱数を作りたくないので static にする
				static std::mt19937 random(std::random_device{}());
				// 柱のかげ / 階段のわき / 普通 / ベンチ と同じ重み
				const std::vector<float> weightList = { 2.0f,1.5f,1.0f,0.5f };
				const int tryCount = 10000;

				// 回数を数える処理をラムダにする
				auto makeResult = [&](int excludeIndex)
					{
						std::vector<int> countList(weightList.size(), 0);

						for (int i = 0; i < tryCount; ++i)
						{
							const int index = ai::PickWeighted(weightList, excludeIndex, random);
							if (index < 0) continue;
							++countList[index];
						}

						std::string text;
						for (int i = 0; i < static_cast<int>(countList.size()); ++i)
						{
							text += " [" + std::to_string(i) + "] " + std::to_string(countList[i]);
						}
						return text;
					};

				// 結果を文字で出力
				m_message = "Lottery (2.0 / 1.5 / 1.0 / 0.5)\n";
				m_message += "  exclude -1:" + makeResult(-1) + "\n";
				m_message += "  exclude  0:" + makeResult(0);
			}
			ImGui::SameLine();

			// 確認のために全員を警戒させる
			if (ImGui::Button("Alert all"))
			{
				for (auto& enemy : enemyList)
				{
					enemy->StartAlert();
				}
				m_message = "Alert started";
			}
			ImGui::SameLine();
			// 警戒の時間を待たずに解く(警戒の前の美術品へ戻るかの確認用)
			if (ImGui::Button("Calm all"))
			{
				for (auto& enemy : enemyList)
				{
					enemy->EndAlert();
				}
				m_message = "Alert ended";
			}


			if (!m_message.empty())
			{
				ImGui::TextUnformatted(m_message.c_str());
			}

			if (!m_target.navGrid->IsBuilt())
			{
				ImGui::TextDisabled("NavGrid is not built.");
				ImGui::End();
				return;
			}

			const int width = m_target.navGrid->GetWidth();
			const int height = m_target.navGrid->GetHeight();
			ImGui::Text("Cells: %d x %d", width, height);

			// 地図の階の美術品だけを出す(ほかの階の美術品は、敵も向かわない)
			const int navFloor = m_target.enemyController->ToFloorIndex(m_target.navGrid->GetSetting().floorHeight);
			const std::vector<Vector3> targetList = m_target.enemyController->MakeFloorTargetList(navFloor);
			ImGui::Text("Art targets  floor %d: %d  (all floors: %d)",
				navFloor, static_cast<int>(targetList.size()), m_target.enemyController->GetTargetCount());

			// 敵ごとの状態(道順の点の数・今何番目か・着いたか)
			for (int i = 0; i < static_cast<int>(enemyList.size()); ++i)
			{
				const ai::PathFollower& follower = enemyList[i]->GetEnemyStateMachine()->GetPathFollower();
				const Vector3& position = enemyList[i]->GetTransform().GetWorldPosition();
				ImGui::Text("Enemy%d  pos(%.0f, %.0f, %.0f)  path %d  index %d  arrived %s  stuck %s",
					i, position.x, position.y, position.z,
					static_cast<int>(follower.GetPath().size()), follower.GetIndex(),
					follower.IsArrived() ? "yes" : "no",
					follower.IsStuck() ? "yes" : "no");

				// 目的地モードの今の段階と、向かっている美術品の番号
				const actor::EnemyDestinationMode* destinationMode = enemyList[i]->GetDestinationMode();

				if (destinationMode != nullptr)
				{
					// 目的地モードの Phase を表示
					ImGui::Text("        phase %s  target %d / %d",
						destinationMode->GetPhaseName(), destinationMode->GetTargetIndex(),
						static_cast<int>(targetList.size()));

					// 渡された隠れ場所の数(今いる階の分)
					ImGui::Text("        hide points %d / %d",
						static_cast<int>(enemyList[i]->GetHidePointList().size()),
						m_target.enemyController->GetHidePointCount());
				}

				// 警戒モードの今の段階と、向かっている隠れ場所の番号
				const actor::EnemyAlertMode* alertMode = enemyList[i]->GetAlertMode();
				// 警戒している時だけ表示
				if (alertMode != nullptr && enemyList[i]->GetEnemyStateMachine()->IsAlert())
				{
					// 警戒モードの Phase を表示
					ImGui::Text("        alert %s point %d remain %.1f",
						alertMode->GetPhaseName(), alertMode->GetPointIndex(), alertMode->GetRemainTime());
				}

				// ビジョンを取得
				const ai::Vision& vision = enemyList[i]->GetVision();
				// 見えていたら判定は See 見えていないなら ---
				ImGui::Text("        vision %s  %.2f", vision.IsSeeing() ? "SEE" : "---", vision.GetSeeTimer());

			}

			const Vector3& boyWorld = m_target.boy->GetTransform().GetWorldPosition();
			ImGui::Text("Boy  pos(%.0f, %.0f, %.0f)", boyWorld.x, boyWorld.y, boyWorld.z);

			// 主人公がいる階と区画(ステージの構造の JSON が合っているかの確認用)
			const int boyFloor = m_target.stageLayout->FindFloorIndex(boyWorld.y);
			const stage::StageArea* boyArea = m_target.stageLayout->FindArea(boyWorld);
			ImGui::Text("Boy  floor %s  area %s",
				boyFloor >= 0 ? m_target.stageLayout->GetFloor(boyFloor).name.c_str() : "-",
				boyArea != nullptr ? boyArea->name.c_str() : "-");

			// 地図を作った時の高さの帯(この範囲に入る置物が塞ぐ)。主人公の y(床の高さ)とずれていないか確認する
			const ai::NavGridSetting& debugSetting = m_target.navGrid->GetSetting();
			ImGui::Text("Block band  y[%.0f ~ %.0f]  (floorHeight %.0f)",
				debugSetting.floorHeight + debugSetting.stepHeight,
				debugSetting.floorHeight + debugSetting.agentHeight,
				debugSetting.floorHeight);


			/** 2. 地図(赤 = 通れない、灰 = 通れる) */
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			const ImVec2 origin = ImGui::GetCursorScreenPos();
			const float pixel = NAV_DEBUG_PIXEL_PER_CELL;

			// マス番号から、画面上のマスの左上の位置を求める
			// ※ 画面は下に行くほど Y が増えるので、奥(z)を上にするため反転する
			auto toScreen = [&](const int x, const int z)
				{
					return ImVec2(origin.x + x * pixel, origin.y + (height - 1 - z) * pixel);
				};

			for (int z = 0; z < height; ++z)
			{
				for (int x = 0; x < width; ++x)
				{
					const ImVec2 topLeft = toScreen(x, z);
					const ImU32 color = m_target.navGrid->IsBlocked(x, z)
						? IM_COL32(200, 60, 60, 255)
						: IM_COL32(60, 60, 60, 255);
					drawList->AddRectFilled(topLeft, ImVec2(topLeft.x + pixel, topLeft.y + pixel), color);
				}
			}

			// 座標から、画面上のマスの中心の位置を求める(地図の外なら false)
			auto worldToScreen = [&](const Vector3& position, ImVec2& outScreen)
				{
					int x = 0;
					int z = 0;
					if (!m_target.navGrid->WorldToCell(position, x, z)) return false;
					const ImVec2 topLeft = toScreen(x, z);
					outScreen = ImVec2(topLeft.x + pixel * 0.5f, topLeft.y + pixel * 0.5f);
					return true;
				};


			/** 3. 敵の道順(黄色の線)と、敵(水色)・主人公(緑)の位置 */
			for (auto& enemy : enemyList)
			{
				const ai::PathFollower& follower = enemy->GetEnemyStateMachine()->GetPathFollower();
				const std::vector<Vector3>& path = follower.GetPath();

				// 敵の今の位置から、今目指している点までと、その先の点同士を線でつなぐ
				ImVec2 previous;
				bool hasPrevious = worldToScreen(enemy->GetTransform().GetWorldPosition(), previous);
				for (int i = follower.GetIndex(); i < static_cast<int>(path.size()); ++i)
				{
					ImVec2 current;
					if (!worldToScreen(path[i], current)) continue;
					if (hasPrevious)
					{
						drawList->AddLine(previous, current, IM_COL32(255, 220, 0, 255), 2.0f);
					}
					previous = current;
					hasPrevious = true;
				}

				ImVec2 enemyScreen;
				if (worldToScreen(enemy->GetTransform().GetWorldPosition(), enemyScreen))
				{
					drawList->AddCircleFilled(enemyScreen, pixel, IM_COL32(80, 200, 255, 255));
				}
			}

			// 主人公がいる階の区画の枠(オレンジ)
			// ※ 区画は地図の外にはみ出すことがあるので、マス番号を通さずに座標から直接求める
			if (boyFloor >= 0)
			{
				const float cellSize = m_target.navGrid->GetCellSize();
				const Vector3 gridOrigin = m_target.navGrid->CellToWorld(0, 0) - Vector3(cellSize * 0.5f, 0.0f, cellSize * 0.5f);
				auto worldToScreenFree = [&](const float worldX, const float worldZ)
					{
						return ImVec2(
							origin.x + (worldX - gridOrigin.x) / cellSize * pixel,
							origin.y + (height - (worldZ - gridOrigin.z) / cellSize) * pixel);
					};

				for (const stage::StageArea& area : m_target.stageLayout->GetFloor(boyFloor).areaList)
				{
					// 奥(z)を上にしているので、max.z が上の辺になる
					const ImVec2 topLeft = worldToScreenFree(area.min.x, area.max.z);
					const ImVec2 bottomRight = worldToScreenFree(area.max.x, area.min.z);
					drawList->AddRect(topLeft, bottomRight, IM_COL32(255, 160, 40, 255), 0.0f, 0, 2.0f);
					drawList->AddText(ImVec2(topLeft.x + 2.0f, topLeft.y + 2.0f), IM_COL32(255, 160, 40, 255), area.name.c_str());
				}
			}

			// 美術品の目印(紫の四角と番号)
			for (int i = 0; i < static_cast<int>(targetList.size()); ++i)
			{
				ImVec2 targetScreen;
				if (!worldToScreen(targetList[i], targetScreen)) continue;
				drawList->AddRectFilled(
					ImVec2(targetScreen.x - pixel, targetScreen.y - pixel),
					ImVec2(targetScreen.x + pixel, targetScreen.y + pixel),
					IM_COL32(200, 100, 255, 255));
				drawList->AddText(ImVec2(targetScreen.x + pixel, targetScreen.y - pixel * 2.0f),
					IM_COL32(255, 255, 255, 255), std::to_string(i).c_str());
			}

			// 隠れ場所の目印(水色の丸と重み)
			for (const stage::HidePoint& hidePoint : m_target.stage->GetHidePointList())
			{
				// 地図の階の分だけ取り出す
				if (m_target.enemyController->ToFloorIndex(hidePoint.position.y) != navFloor) continue;

				ImVec2 hideScreen;
				if (!worldToScreen(hidePoint.position, hideScreen)) continue;
				drawList->AddCircle(hideScreen, pixel * 1.5f, IM_COL32(80, 220, 255, 255), 0, 2.0f);

				char label[32];
				// 重みの値を文字で出力
				sprintf_s(label, "H%.1f", hidePoint.weight);
				drawList->AddText(ImVec2(hideScreen.x + pixel, hideScreen.y), IM_COL32(80, 220, 255, 255), label);
			}


			ImVec2 boyScreen;
			if (worldToScreen(m_target.boy->GetTransform().GetWorldPosition(), boyScreen))
			{
				drawList->AddCircleFilled(boyScreen, pixel, IM_COL32(80, 255, 120, 255));
			}

			// 描いた分だけ場所を空ける(ウィンドウの大きさとスクロールに使われる)
			const ImVec2 mapSize(width* pixel, height* pixel);
			ImGui::Dummy(mapSize);

			/** 4. マスにカーソルを当てると、そのマスを塞いでいる置物を出す */
			// ※ NavGrid::Build と同じ条件(高さの帯 + 敵の半径ぶん太らせた箱)で探す
			if (ImGui::IsItemHovered())
			{
				const ImVec2 mouse = ImGui::GetIO().MousePos;
				const int cellX = static_cast<int>(std::floor((mouse.x - origin.x) / pixel));
				const int cellZ = height - 1 - static_cast<int>(std::floor((mouse.y - origin.y) / pixel));

				if (m_target.navGrid->IsInside(cellX, cellZ))
				{
					const ai::NavGridSetting& setting = m_target.navGrid->GetSetting();
					const Vector3 center = m_target.navGrid->CellToWorld(cellX, cellZ);
					const float half = setting.cellSize * 0.5f;
					const float bandMin = setting.floorHeight + setting.stepHeight;
					const float bandMax = setting.floorHeight + setting.agentHeight;

					ImGui::BeginTooltip();
					ImGui::Text("Cell (%d, %d)  world (%.0f, %.0f)  %s",
						cellX, cellZ, center.x, center.z,
						m_target.navGrid->IsBlocked(cellX, cellZ) ? "BLOCKED" : "open");

					int blockerCount = 0;
					for (int i = 0; i < m_target.stage->GetObjectCount(); ++i)
					{
						const actor::StageObject* object = m_target.stage->GetStageObject(i);
						if (object == nullptr || !object->IsCollision()) continue;

						const Vector3& boxMin = object->GetWorldAABBMin();
						const Vector3& boxMax = object->GetWorldAABBMax();

						// 体の高さに重なっていなければ、塞いでいない
						if (boxMax.y <= bandMin || boxMin.y >= bandMax) continue;

						// 太らせた箱と、マスが重なっているか
						const bool isOverlapX = (center.x + half >= boxMin.x - setting.agentRadius) && (center.x - half <= boxMax.x + setting.agentRadius);
						const bool isOverlapZ = (center.z + half >= boxMin.z - setting.agentRadius) && (center.z - half <= boxMax.z + setting.agentRadius);
						if (!isOverlapX || !isOverlapZ) continue;

						// 名前、高さの範囲、箱の大きさ(大きすぎる物は、部屋ごと塞いでいる犯人の可能性が高い)
						if (blockerCount < NAV_DEBUG_BLOCKER_MAX)
						{
							ImGui::Text("%d: %s  y[%.0f ~ %.0f]  size %.0f x %.0f",
								i, object->GetName().c_str(), boxMin.y, boxMax.y,
								boxMax.x - boxMin.x, boxMax.z - boxMin.z);
						}
						++blockerCount;
					}
					if (blockerCount > NAV_DEBUG_BLOCKER_MAX)
					{
						ImGui::Text("... and %d more", blockerCount - NAV_DEBUG_BLOCKER_MAX);
					}
					ImGui::EndTooltip();
				}
			}
			ImGui::End();
#endif
		}
	}
}
