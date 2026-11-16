/**
 * @file StageEditor.cpp
 * @brief ステージエディタ(Debug の時だけ使う)
 */
#include "stdafx.h"
#include "StageEditor.h"
#include "Stage.h"
#include "Source/Actor/StageObject/StageObject.h"
#include "Source/Util/RayCast.h"
#include "system/system.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <map>
#include <vector>


namespace app
{
	namespace stage
	{
		namespace
		{
			/** 追加するモデルのパスの初期値 */
			constexpr const char* DEFAULT_ADD_MODEL_PATH = "Assets/modelData/Stage/SM_Bench.tkm";
			/** 複製した時に、元の位置からずらす量 */
			const Vector3 DUPLICATE_OFFSET = { 50.0f, 0.0f, 0.0f };
			/** 一覧の高さ */
			constexpr float OBJECT_LIST_HEIGHT = 200.0f;
			/** モデルごとのポリゴン数の表の高さ */
			constexpr float POLYGON_TABLE_HEIGHT = 200.0f;
			/** パレットの高さ */
			constexpr float PALETTE_HEIGHT = 150.0f;

			/** 視点の描き先の解像度(16:9) */
			constexpr int VIEW_RENDER_WIDTH = 1280;
			constexpr int VIEW_RENDER_HEIGHT = 720;
			/**
			 * 1 つの画面の縦横の比(16:9)
			 * ※ エンジンのカメラは、縦横の比をゲームの画面全体(16:9)から決めるので、パースの画面がゆがまないように合わせる
			 */
			constexpr float VIEW_ASPECT = 16.0f / 9.0f;
			/** 1 つの画面の最小の幅(ウィンドウを小さくしすぎても、これより小さくしない) */
			constexpr float VIEW_CELL_MIN_WIDTH = 160.0f;
			/** 4 つの画面を並べるウィンドウの、最初の大きさ(1 つの画面が 448×252 になるくらい) */
			constexpr float VIEW_WINDOW_DEFAULT_WIDTH = 930.0f;
			constexpr float VIEW_WINDOW_DEFAULT_HEIGHT = 610.0f;
			/** 4 つの画面を並べるウィンドウの、最初の位置(画面の右上からの余白) */
			constexpr float VIEW_WINDOW_MARGIN = 10.0f;
			/** 1 行に並べる画面の数(2×2) */
			constexpr int VIEW_COLUMN_COUNT = 2;
			/** 視点のカメラの近平面(断面のすぐ奥から映す) */
			constexpr float VIEW_NEAR = 1.0f;
			/** 視点のカメラの遠平面 */
			constexpr float VIEW_FAR = 20000.0f;
			/** ズーム(1 ピクセルあたりの長さ)の範囲 */
			constexpr float VIEW_ZOOM_MIN = 0.2f;
			constexpr float VIEW_ZOOM_MAX = 50.0f;
			/** ホイール 1 目盛りでズームが変わる割合 */
			constexpr float VIEW_ZOOM_STEP = 1.15f;
			/** 数値の欄の幅 */
			constexpr float VIEW_ITEM_WIDTH = 110.0f;

			/** 画面の見出し */
			const char* VIEW_LABEL[] = { "Top", "Front", "Side", "Perspective" };
			/** 断面の欄の名前(平行投影の画面だけ) */
			const char* VIEW_CUT_LABEL[] = { "Cut Y", "Cut Z", "Cut X" };
			/** 見る向き */
			const Vector3 VIEW_FORWARD[] = { { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } };
			/** 画面の上の向き(真上から見る時は、見る向きと平行にならないように Z を上にする) */
			const Vector3 VIEW_UP[] = { { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
			/** 見る向きの軸の番号(0:X 1:Y 2:Z。断面の位置を入れる軸) */
			constexpr int VIEW_AXIS[] = { 1, 2, 0 };
			/** 断面の位置の初期値(真上は 1 階の天井より下、正面と横は建物の外) */
			constexpr float VIEW_DEFAULT_CUT[] = { 450.0f, -3000.0f, -3000.0f };
			/** 見ている中心の初期値 */
			const Vector3 VIEW_DEFAULT_CENTER = { 0.0f, 200.0f, 0.0f };
			/** ズームの初期値 */
			constexpr float VIEW_DEFAULT_ZOOM = 4.0f;

			/** パースの画面の画角(度数) */
			constexpr float PERSPECTIVE_VIEW_ANGLE_DEG = 60.0f;
			/** パースの画面の近平面・遠平面 */
			constexpr float PERSPECTIVE_NEAR = 10.0f;
			constexpr float PERSPECTIVE_FAR = 30000.0f;
			/** パースの画面で、1 ピクセル動かした時に回る角度(度数) */
			constexpr float PERSPECTIVE_ROTATE_SPEED = 0.3f;
			/** パースの画面の縦の回転の範囲(真上・真下で向きが決まらなくならないように) */
			constexpr float PERSPECTIVE_PITCH_MIN = -89.0f;
			constexpr float PERSPECTIVE_PITCH_MAX = 89.0f;
			/** パースの画面の、注視点からカメラまでの距離の範囲 */
			constexpr float PERSPECTIVE_DISTANCE_MIN = 50.0f;
			constexpr float PERSPECTIVE_DISTANCE_MAX = 20000.0f;
			/** パースの画面の初期値(美術館の真ん中あたりを、斜め上から見る) */
			const Vector3 PERSPECTIVE_DEFAULT_TARGET = { 700.0f, 200.0f, 450.0f };
			constexpr float PERSPECTIVE_DEFAULT_YAW_DEG = -45.0f;
			constexpr float PERSPECTIVE_DEFAULT_PITCH_DEG = 35.0f;
			constexpr float PERSPECTIVE_DEFAULT_DISTANCE = 3500.0f;
			/** パースの画面の上の向き */
			const Vector3 PERSPECTIVE_UP = { 0.0f, 1.0f, 0.0f };
			/** パースのカメラを WASD で飛ばす速さ(1 秒に進む長さ) */
			constexpr float PERSPECTIVE_FLY_SPEED = 800.0f;
			/** Shift を押している時の速さの倍率 */
			constexpr float PERSPECTIVE_FLY_FAST_RATE = 3.0f;

			/** 元に戻せる回数 */
			constexpr int MAX_UNDO_COUNT = 50;

			/**
			 * ImGui に登録した視点の描き先の番号
			 * ※ 描き先は RenderingEngine がずっと持っているので、登録もゲーム中に 1 回だけにする(imgui のヒープは 64 個しかない)
			 */
			UINT64 g_viewTextureIdList[RenderingEngine::MAX_SUB_VIEW] = {};


			/**
			 * @brief 数を3桁ごとにカンマで区切った文字列にする(1234567 → "1,234,567")
			 * @param value 数
			 * @return 文字列
			 */
			std::string FormatNumber(const long long value)
			{
				std::string text = std::to_string(value < 0 ? -value : value);
				for (int i = static_cast<int>(text.size()) - 3; i > 0; i -= 3)
				{
					text.insert(static_cast<size_t>(i), ",");
				}
				return value < 0 ? "-" + text : text;
			}


			/**
			 * @brief 文字列に、絞り込みの文字が入っているか(大文字・小文字は区別しない)
			 * @param text 文字列
			 * @param filter 絞り込みの文字
			 * @return 入っていれば true(絞り込みが空なら true)
			 */
			bool IsMatchFilter(const std::string& text, const char* filter)
			{
				if (filter[0] == '\0') return true;

				std::string lowerText = text;
				std::string lowerFilter = filter;
				for (char& c : lowerText) c = static_cast<char>(tolower(c));
				for (char& c : lowerFilter) c = static_cast<char>(tolower(c));
				return lowerText.find(lowerFilter) != std::string::npos;
			}


			/**
			 * @brief モデルのパスから、名前(ファイル名から拡張子を取ったもの)を作る
			 * @param modelPath モデルのパス
			 * @return 名前
			 */
			std::string MakeNameFromPath(const std::string& modelPath)
			{
				const size_t slash = modelPath.find_last_of("/\\");
				const size_t start = (slash == std::string::npos) ? 0 : slash + 1;
				const size_t dot = modelPath.find_last_of('.');
				const size_t end = (dot == std::string::npos || dot < start) ? modelPath.size() : dot;
				return modelPath.substr(start, end - start);
			}


			/**
			 * @brief 横と縦の回転から、見る向きを作る
			 * @param yawDeg 横の回転(度数)
			 * @param pitchDeg 縦の回転(度数。プラスで見下ろす)
			 * @return 見る向き(長さ 1)
			 */
			Vector3 MakeForward(const float yawDeg, const float pitchDeg)
			{
				const float yaw = Math::DegToRad(yawDeg);
				const float pitch = Math::DegToRad(pitchDeg);
				return Vector3(std::cos(pitch) * std::sin(yaw), -std::sin(pitch), std::cos(pitch) * std::cos(yaw));
			}
		}


		StageEditor::StageEditor(Stage* stage, const std::string& filePath, const std::string& tklFilePath, const std::string& modelDirectory)
			: m_stage(stage)
			, m_filePath(filePath)
			, m_tklFilePath(tklFilePath)
			, m_modelDirectory(modelDirectory)
			, m_selectedIndex(-1)
			, m_isDirty(false)
			, m_gizmoOperation(ImGuizmo::TRANSLATE)
			, m_gizmoMode(ImGuizmo::WORLD)
			, m_isSnap(false)
			, m_snapTranslation(10.0f)
			, m_snapRotation(15.0f)
			, m_snapScale(0.1f)
			, m_isGizmoUsingLastFrame(false)
			, m_isPaletteLoaded(false)
			, m_isPlaceMode(false)
			, m_isViewOpen(false)
			, m_hoveredViewIndex(-1)
			, m_gizmoViewIndex(-1)
			, m_dragViewIndex(-1)
			, m_isInspectorEditing(false)
			, m_isTransformEditing(false)
			, m_transformBeforeObject(nullptr)
		{
			m_filter[0] = '\0';
			m_paletteFilter[0] = '\0';
			strncpy_s(m_addModelPath, DEFAULT_ADD_MODEL_PATH, _TRUNCATE);

			for (int i = 0; i < ViewCount; ++i)
			{
				ViewState& view = m_viewList[i];
				view.center = VIEW_DEFAULT_CENTER;
				view.zoom = VIEW_DEFAULT_ZOOM;
				view.cut = (i < ViewPerspective) ? VIEW_DEFAULT_CUT[i] : 0.0f;
				view.yawDeg = PERSPECTIVE_DEFAULT_YAW_DEG;
				view.pitchDeg = PERSPECTIVE_DEFAULT_PITCH_DEG;
				view.distance = PERSPECTIVE_DEFAULT_DISTANCE;
				view.imageX = 0.0f;
				view.imageY = 0.0f;
				view.imageWidth = 1.0f;
				view.imageHeight = 1.0f;
			}
			m_viewList[ViewPerspective].center = PERSPECTIVE_DEFAULT_TARGET;
		}


		StageEditor::~StageEditor()
		{
			// エディタが無くなったら、視点を描くのをやめる(RenderingEngine はずっと残るため)
			for (int i = 0; i < ViewCount; ++i)
			{
				RenderingEngine::GetInstance().DisableSubView(i);
			}
		}


		void StageEditor::Update()
		{
			if (m_stage == nullptr) return;

			// 消したなどで、選んでいた番号が無くなっていたら、選んでいないことにする
			if (m_stage->GetStageObject(m_selectedIndex) == nullptr)
			{
				m_selectedIndex = -1;
			}

			UpdateShortcut();

			// ギズモを出す所を決める(動かしている間は変えない)
			// ※ ImGuizmo は 1 フレームに 1 つだけ動かせるので、マウスが乗っている視点に出す
			if (!ImGuizmo::IsUsing())
			{
				if (m_hoveredViewIndex >= 0)
				{
					m_gizmoViewIndex = m_hoveredViewIndex;
				}
				else if (!ImGui::GetIO().WantCaptureMouse)
				{
					// どのウィンドウにも乗っていない(ゲームの画面の上)
					m_gizmoViewIndex = -1;
				}
			}
			if (!m_isViewOpen) m_gizmoViewIndex = -1;

			// 名前が変わってもウィンドウが別物にならないように、### の後ろを ID にする
			const char* title = m_isDirty ? "Stage Editor *###StageEditor" : "Stage Editor###StageEditor";
			ImGui::Begin(title);

			DrawFileMenu();
			ImGui::Separator();
			DrawGizmoSetting();
			ImGui::Separator();
			DrawViewSetting();
			ImGui::Separator();
			DrawPolygonStats();
			ImGui::Separator();
			DrawObjectList();
			ImGui::Separator();
			DrawInspector();
			ImGui::Separator();
			DrawAddObject();

			ImGui::End();

			// 4 つの画面のウィンドウ
			UpdateViews();

			// ギズモはウィンドウの外(ゲームの画面)に出す
			UpdateGizmo();

			// 動かし終わったら、元に戻せるように記録する(ギズモと数値の欄を触った後に呼ぶ)
			UpdateTransformUndo();
		}


		bool StageEditor::IsCameraFlying() const
		{
			// パースの画面で右ドラッグを始めて、まだ押している間
			return m_isViewOpen && m_dragViewIndex == ViewPerspective && ImGui::IsMouseDown(ImGuiMouseButton_Right);
		}


		void StageEditor::DrawFileMenu()
		{
			ImGui::TextDisabled("%s", m_filePath.c_str());

			if (ImGui::Button("Save"))
			{
				if (m_stage->Save(m_filePath))
				{
					m_isDirty = false;
					m_message = "Saved.";
				}
				else
				{
					m_message = "Save failed.";
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Reload"))
			{
				m_message = m_stage->Load(m_filePath) ? "Reloaded." : "Reload failed (no file?).";
				m_selectedIndex = -1;
				m_isDirty = false;
				// 置物が全部作り直されるので、元に戻す記録は使えなくなる
				ClearUndo();
			}

			// tkl から読み直すと、エディタで直した配置が全部消えるので、確認してから
			ImGui::SameLine();
			if (ImGui::Button("Import TKL"))
			{
				ImGui::OpenPopup("ImportTklConfirm");
			}
			if (ImGui::BeginPopupModal("ImportTklConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Replace all objects with:\n%s", m_tklFilePath.c_str());
				if (ImGui::Button("OK"))
				{
					m_message = m_stage->ImportTkl(m_tklFilePath.c_str(), m_modelDirectory) ? "Imported TKL." : "Import failed.";
					m_selectedIndex = -1;
					MarkDirty();
					ClearUndo();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			// 1 つ前に戻す(記録が無い時は押せない)
			ImGui::SameLine();
			ImGui::BeginDisabled(m_undoList.empty());
			const std::string undoLabel = "Undo (" + std::to_string(m_undoList.size()) + ")";
			if (ImGui::Button(undoLabel.c_str()))
			{
				Undo();
			}
			ImGui::EndDisabled();

			if (!m_message.empty())
			{
				ImGui::SameLine();
				ImGui::TextUnformatted(m_message.c_str());
			}
		}


		void StageEditor::DrawGizmoSetting()
		{
			ImGui::RadioButton("Move (1)", &m_gizmoOperation, ImGuizmo::TRANSLATE);
			ImGui::SameLine();
			ImGui::RadioButton("Rotate (2)", &m_gizmoOperation, ImGuizmo::ROTATE);
			ImGui::SameLine();
			ImGui::RadioButton("Scale (3)", &m_gizmoOperation, ImGuizmo::SCALE);

			// スケールはローカルでしか使えないので、スケールの時は選べないようにする
			if (m_gizmoOperation != ImGuizmo::SCALE)
			{
				ImGui::RadioButton("World", &m_gizmoMode, ImGuizmo::WORLD);
				ImGui::SameLine();
				ImGui::RadioButton("Local", &m_gizmoMode, ImGuizmo::LOCAL);
				ImGui::SameLine();
			}

			ImGui::Checkbox("Snap", &m_isSnap);
			if (m_isSnap)
			{
				ImGui::SameLine();
				ImGui::SetNextItemWidth(80.0f);
				switch (m_gizmoOperation)
				{
				case ImGuizmo::TRANSLATE: ImGui::DragFloat("##SnapT", &m_snapTranslation, 1.0f, 0.1f, 1000.0f); break;
				case ImGuizmo::ROTATE:    ImGui::DragFloat("##SnapR", &m_snapRotation, 1.0f, 1.0f, 180.0f); break;
				case ImGuizmo::SCALE:     ImGui::DragFloat("##SnapS", &m_snapScale, 0.01f, 0.01f, 10.0f); break;
				default: break;
				}
			}
		}


		void StageEditor::DrawViewSetting()
		{
			if (ImGui::Checkbox("4 Views (Top / Front / Side / Perspective)", &m_isViewOpen))
			{
				// 閉じたら、視点を描くのをやめる(重くしない)
				if (!m_isViewOpen)
				{
					for (int i = 0; i < ViewCount; ++i)
					{
						RenderingEngine::GetInstance().DisableSubView(i);
					}
				}
			}

			// 消す・戻すは、4 Views を出していなくても使える
			ImGui::TextDisabled("Delete: remove selected / Ctrl+Z: undo");

			if (!m_isViewOpen) return;

			ImGui::TextDisabled("Top/Front/Side: Wheel zoom / Right or Middle drag pan");
			ImGui::TextDisabled("Perspective: Right drag look / Right + WASD fly (Q/E down/up, Shift fast)");
			ImGui::TextDisabled("             Alt + Right drag orbit / Middle drag pan / Wheel distance");
			ImGui::TextDisabled("Left click: select (or place with Place by click)");
			ImGui::TextDisabled("Cut: objects in front of it are hidden (Top: ceiling)");
		}


		void StageEditor::DrawPolygonStats()
		{
			// モデルごとに、1個のポリゴン数と、置いている数をまとめる
			struct ModelStats
			{
				std::string name;
				int triangleCount = 0;
				int objectCount = 0;
				long long GetTotal() const { return static_cast<long long>(triangleCount) * objectCount; }
			};
			std::map<std::string, ModelStats> statsMap;
			long long totalTriangleCount = 0;
			for (int i = 0; i < m_stage->GetObjectCount(); ++i)
			{
				const actor::StageObject* object = m_stage->GetStageObject(i);
				ModelStats& stats = statsMap[object->GetModelPath()];
				stats.name = object->GetName();
				stats.triangleCount = object->GetTriangleCount();
				stats.objectCount++;
				totalTriangleCount += object->GetTriangleCount();
			}

			ImGui::Text("Total triangles: %s  (objects: %d, models: %d)",
				FormatNumber(totalTriangleCount).c_str(), m_stage->GetObjectCount(), static_cast<int>(statsMap.size()));

			if (!ImGui::CollapsingHeader("Triangles by model")) return;

			// 合計の多い順(重いモデルが上)
			std::vector<ModelStats> statsList;
			statsList.reserve(statsMap.size());
			for (const auto& pair : statsMap)
			{
				statsList.push_back(pair.second);
			}
			std::sort(statsList.begin(), statsList.end(), [](const ModelStats& a, const ModelStats& b)
			{
				return a.GetTotal() > b.GetTotal();
			});

			const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
			if (!ImGui::BeginTable("TrianglesByModel", 5, flags, ImVec2(0.0f, POLYGON_TABLE_HEIGHT))) return;

			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("Model", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Tris");
			ImGui::TableSetupColumn("Count");
			ImGui::TableSetupColumn("Total");
			ImGui::TableSetupColumn("%");
			ImGui::TableHeadersRow();

			for (const ModelStats& stats : statsList)
			{
				ImGui::TableNextRow();

				// 名前をクリックすると、一覧をそのモデルで絞り込む
				ImGui::TableSetColumnIndex(0);
				if (ImGui::Selectable(stats.name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
				{
					strncpy_s(m_filter, stats.name.c_str(), _TRUNCATE);
				}
				ImGui::TableSetColumnIndex(1);
				ImGui::TextUnformatted(FormatNumber(stats.triangleCount).c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%d", stats.objectCount);
				ImGui::TableSetColumnIndex(3);
				ImGui::TextUnformatted(FormatNumber(stats.GetTotal()).c_str());
				ImGui::TableSetColumnIndex(4);
				const float percent = totalTriangleCount > 0 ? static_cast<float>(stats.GetTotal()) * 100.0f / static_cast<float>(totalTriangleCount) : 0.0f;
				ImGui::Text("%.1f", percent);
			}
			ImGui::EndTable();
		}


		void StageEditor::DrawObjectList()
		{
			ImGui::Text("Objects: %d", m_stage->GetObjectCount());
			ImGui::InputText("Filter", m_filter, sizeof(m_filter));

			ImGui::BeginChild("ObjectList", ImVec2(0.0f, OBJECT_LIST_HEIGHT), ImGuiChildFlags_Borders);
			for (int i = 0; i < m_stage->GetObjectCount(); ++i)
			{
				const actor::StageObject* object = m_stage->GetStageObject(i);
				if (!IsMatchFilter(object->GetName(), m_filter)) continue;

				// 同じ名前が並ぶので、番号も出す。後ろにポリゴンの数。## の後ろで ID を分ける
				const std::string label = std::to_string(i) + ": " + object->GetName()
					+ "  (" + FormatNumber(object->GetTriangleCount()) + ")##" + std::to_string(i);
				if (ImGui::Selectable(label.c_str(), i == m_selectedIndex))
				{
					m_selectedIndex = i;
				}
			}
			ImGui::EndChild();
		}


		void StageEditor::DrawInspector()
		{
			// 数値の欄を触っているか(元に戻す記録に使う)。毎フレーム調べ直す
			m_isInspectorEditing = false;

			actor::StageObject* object = m_stage->GetStageObject(m_selectedIndex);
			if (object == nullptr)
			{
				ImGui::TextDisabled("No object selected.");
				return;
			}

			ImGui::Text("%s", object->GetName().c_str());
			ImGui::TextDisabled("%s", object->GetModelPath().c_str());
			ImGui::Text("Triangles: %s", FormatNumber(object->GetTriangleCount()).c_str());

			// 値を変えている間は見た目だけ動かして、放した時にコリジョンを作り直す
			bool isEditEnd = false;

			Vector3 position = object->GetPosition();
			if (ImGui::DragFloat3("Position", position.v, 1.0f))
			{
				object->SetPosition(position);
				MarkDirty();
			}
			isEditEnd |= ImGui::IsItemDeactivatedAfterEdit();
			m_isInspectorEditing |= ImGui::IsItemActive();

			Vector3 rotationDeg = object->GetRotationDeg();
			if (ImGui::DragFloat3("Rotation", rotationDeg.v, 1.0f))
			{
				object->SetRotationDeg(rotationDeg);
				MarkDirty();
			}
			isEditEnd |= ImGui::IsItemDeactivatedAfterEdit();
			m_isInspectorEditing |= ImGui::IsItemActive();

			Vector3 scale = object->GetScale();
			if (ImGui::DragFloat3("Scale", scale.v, 0.01f))
			{
				object->SetScale(scale);
				// 反転の有無が変わったら、表と裏のどちらを描くかが変わるので、描画のグループを移す
				m_stage->RefreshObjectGroup(object);
				MarkDirty();
			}
			isEditEnd |= ImGui::IsItemDeactivatedAfterEdit();
			m_isInspectorEditing |= ImGui::IsItemActive();
			if (actor::StageObject::IsMirrored(object->GetScale()))
			{
				ImGui::TextDisabled("Mirrored (negative scale)");
			}

			bool isCollision = object->IsCollision();
			if (ImGui::Checkbox("Collision", &isCollision))
			{
				object->SetIsCollision(isCollision);
				MarkDirty();
				isEditEnd = true;
			}

			if (isEditEnd)
			{
				object->RebuildCollision();
			}

			if (ImGui::Button("Duplicate"))
			{
				// 重ならないように、少しずらした位置に作る
				Vector3 newPosition = object->GetPosition();
				newPosition.Add(DUPLICATE_OFFSET);
				const actor::StageObject* duplicated = m_stage->AddObject(object->GetName(), object->GetModelPath(), newPosition, object->GetRotationDeg(), object->GetScale(), isCollision);
				if (duplicated != nullptr)
				{
					OnObjectAdded(duplicated);
					m_message = "Duplicated.";
				}
				return;
			}

			ImGui::SameLine();
			if (ImGui::Button("Delete (Del)"))
			{
				DeleteSelected();
			}
		}


		void StageEditor::DrawAddObject()
		{
			// パレットは最初に開いた時に作る(フォルダを読むので、毎フレームは読まない)
			if (!m_isPaletteLoaded) RefreshPalette();

			ImGui::Text("Palette (%d)", static_cast<int>(m_paletteList.size()));
			ImGui::SameLine();
			if (ImGui::Button("Refresh")) RefreshPalette();
			ImGui::InputText("Palette Filter", m_paletteFilter, sizeof(m_paletteFilter));

			ImGui::BeginChild("Palette", ImVec2(0.0f, PALETTE_HEIGHT), ImGuiChildFlags_Borders);
			for (const std::string& modelPath : m_paletteList)
			{
				const std::string name = MakeNameFromPath(modelPath);
				if (!IsMatchFilter(name, m_paletteFilter)) continue;

				// 選んだモデルを、追加するモデルにする
				const bool isSelected = modelPath == m_addModelPath;
				if (ImGui::Selectable(name.c_str(), isSelected))
				{
					strncpy_s(m_addModelPath, modelPath.c_str(), _TRUNCATE);
				}
			}
			ImGui::EndChild();

			ImGui::InputText("Model", m_addModelPath, sizeof(m_addModelPath));
			if (ImGui::Button("Add"))
			{
				// カメラの注視点に置く(今見ている所に出てくる)
				const std::string modelPath = m_addModelPath;
				actor::StageObject* object = m_stage->AddObject(
					MakeNameFromPath(modelPath),
					modelPath,
					g_camera3D->GetTarget(),
					Vector3::Zero,
					Vector3::One,
					true
				);

				if (object != nullptr)
				{
					OnObjectAdded(object);
					m_message = "Added.";
				}
				else
				{
					m_message = "Model not found.";
				}
			}

			// 視点の中の左クリックで、選ぶか置くか
			ImGui::SameLine();
			ImGui::Checkbox("Place by click (4 Views)", &m_isPlaceMode);
		}


		void StageEditor::RefreshPalette()
		{
			m_paletteList.clear();
			m_isPaletteLoaded = true;

			// モデルのフォルダの tkm を集める(フォルダが無くても止まらないように、エラーは error_code で受ける)
			std::error_code errorCode;
			for (const auto& entry : std::filesystem::directory_iterator(m_modelDirectory, errorCode))
			{
				if (!entry.is_regular_file(errorCode)) continue;
				if (entry.path().extension() != ".tkm") continue;

				m_paletteList.push_back(m_modelDirectory + entry.path().filename().string());
			}
			std::sort(m_paletteList.begin(), m_paletteList.end());
		}


		void StageEditor::UpdateViews()
		{
			if (!m_isViewOpen) return;

			// 1 つのウィンドウに 4 つの画面を 2×2 で並べる(動かせる・大きさを変えられる)
			// ホイールはズームに使うので、スクロールしない
			const ImGuiIO& io = ImGui::GetIO();
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
			// 絵の上にマウスがある時や、絵の上でドラッグしている時は、ウィンドウを動かさない(ギズモ・選ぶ・カメラの操作に使う)
			// ※ Begin の前に決めるので、前のフレームの結果を使う
			const bool isGizmoDragging = m_gizmoViewIndex >= 0 && ImGuizmo::IsUsing();
			if (m_hoveredViewIndex >= 0 || m_dragViewIndex >= 0 || isGizmoDragging)
			{
				flags |= ImGuiWindowFlags_NoMove;
			}

			// 最初は画面の右上に出す(右上の角を基準に置く。pivot = 右上)
			ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - VIEW_WINDOW_MARGIN, VIEW_WINDOW_MARGIN), ImGuiCond_FirstUseEver, ImVec2(1.0f, 0.0f));
			ImGui::SetNextWindowSize(ImVec2(VIEW_WINDOW_DEFAULT_WIDTH, VIEW_WINDOW_DEFAULT_HEIGHT), ImGuiCond_FirstUseEver);

			// このフレームにマウスが乗っている画面を、調べ直す
			m_hoveredViewIndex = -1;

			// 畳んでいる時は描かない(重くしない)
			if (!ImGui::Begin("Stage Views###StageEditorViews", nullptr, flags))
			{
				for (int i = 0; i < ViewCount; ++i)
				{
					RenderingEngine::GetInstance().DisableSubView(i);
				}
				ImGui::End();
				return;
			}

			// ウィンドウの広さから、1 つの絵の大きさを決める(16:9 を保って、2×2 に入る一番大きな大きさ)
			const ImGuiStyle& style = ImGui::GetStyle();
			const ImVec2 available = ImGui::GetContentRegionAvail();
			const float headerHeight = ImGui::GetFrameHeightWithSpacing();
			float cellWidth = (available.x - style.ItemSpacing.x) / VIEW_COLUMN_COUNT;
			float cellHeight = (available.y - style.ItemSpacing.y - headerHeight * 2.0f) / 2.0f;
			if (cellWidth > cellHeight * VIEW_ASPECT)
			{
				cellWidth = cellHeight * VIEW_ASPECT;
			}
			cellWidth = (std::max)(cellWidth, VIEW_CELL_MIN_WIDTH);
			cellHeight = cellWidth / VIEW_ASPECT;

			for (int i = 0; i < ViewCount; ++i)
			{
				// 2 つごとに改行する(2×2)
				if (i % VIEW_COLUMN_COUNT != 0) ImGui::SameLine();

				ImGui::BeginGroup();
				ImGui::PushID(i);
				UpdateView(i, cellWidth, cellHeight);
				ImGui::PopID();
				ImGui::EndGroup();
			}

			ImGui::End();
		}


		void StageEditor::UpdateView(const int viewIndex, const float cellWidth, const float cellHeight)
		{
			ViewState& view = m_viewList[viewIndex];
			RenderingEngine& renderingEngine = RenderingEngine::GetInstance();
			const ImGuiIO& io = ImGui::GetIO();
			const bool isPerspective = viewIndex == ViewPerspective;

			// 描き先を作って(最初だけ)、ImGui に登録する(ゲーム中に 1 回だけ)
			renderingEngine.EnableSubView(viewIndex, VIEW_RENDER_WIDTH, VIEW_RENDER_HEIGHT);
			if (g_viewTextureIdList[viewIndex] == 0)
			{
				RenderTarget& renderTarget = renderingEngine.GetSubViewRenderTarget(viewIndex);
				g_viewTextureIdList[viewIndex] = RegisterImGuiTexture(renderTarget.GetRenderTargetTexture().Get(), renderTarget.GetColorBufferFormat());
			}

			// 見出しと設定(平行投影は断面とズーム、パースは距離とリセット)
			ImGui::TextUnformatted(VIEW_LABEL[viewIndex]);
			ImGui::SameLine();
			if (isPerspective)
			{
				ImGui::SetNextItemWidth(VIEW_ITEM_WIDTH);
				ImGui::DragFloat("Distance", &view.distance, 10.0f, PERSPECTIVE_DISTANCE_MIN, PERSPECTIVE_DISTANCE_MAX);
				ImGui::SameLine();
				if (ImGui::SmallButton("Reset"))
				{
					view.center = PERSPECTIVE_DEFAULT_TARGET;
					view.yawDeg = PERSPECTIVE_DEFAULT_YAW_DEG;
					view.pitchDeg = PERSPECTIVE_DEFAULT_PITCH_DEG;
					view.distance = PERSPECTIVE_DEFAULT_DISTANCE;
				}
			}
			else
			{
				ImGui::SetNextItemWidth(VIEW_ITEM_WIDTH);
				ImGui::DragFloat(VIEW_CUT_LABEL[viewIndex], &view.cut, 5.0f);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(VIEW_ITEM_WIDTH);
				ImGui::DragFloat("Zoom", &view.zoom, 0.05f, VIEW_ZOOM_MIN, VIEW_ZOOM_MAX);
			}

			// 絵の場所(大きさはウィンドウの広さから決めた 16:9)
			const ImVec2 imagePosition = ImGui::GetCursorScreenPos();
			const ImVec2 imageSize(cellWidth, cellHeight);
			const ImVec2 imageMax(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y);
			view.imageX = imagePosition.x;
			view.imageY = imagePosition.y;
			view.imageWidth = imageSize.x;
			view.imageHeight = imageSize.y;

			UpdateViewCamera(viewIndex);

			const bool isHovered = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(imagePosition, imageMax);
			if (isHovered) m_hoveredViewIndex = viewIndex;

			// ズーム・移動・回す(カメラが変わったら、その場で作り直す)
			if (isPerspective)
			{
				UpdatePerspectiveInput(isHovered);
			}
			else
			{
				UpdateOrthographicInput(viewIndex, isHovered);
			}

			// 視点の絵(このフレームの RenderingEngine で、このカメラで描かれる)
			ImGui::Image(ImTextureRef(static_cast<ImTextureID>(g_viewTextureIdList[viewIndex])), imageSize);

			// ギズモ(この視点に出している時だけ)
			const bool isGizmoHere = m_gizmoViewIndex == viewIndex;
			if (isGizmoHere)
			{
				Camera& camera = renderingEngine.GetSubViewCamera(viewIndex);
				ManipulateSelected(camera.GetViewMatrix(), camera.GetProjectionMatrix(), ImGui::GetWindowDrawList(),
					view.imageX, view.imageY, view.imageWidth, view.imageHeight, !isPerspective);
			}

			// 左クリックで、置く・選ぶ(ギズモを触っている時はしない)
			const bool isGizmoBusy = isGizmoHere && (ImGuizmo::IsOver() || ImGuizmo::IsUsing());
			if (isHovered && !isGizmoBusy && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				if (m_isPlaceMode)
				{
					PlaceObject(viewIndex, io.MousePos.x, io.MousePos.y);
				}
				else
				{
					PickObject(viewIndex, io.MousePos.x, io.MousePos.y);
				}
			}
		}


		void StageEditor::UpdateOrthographicInput(const int viewIndex, const bool isHovered)
		{
			ViewState& view = m_viewList[viewIndex];
			const ImGuiIO& io = ImGui::GetIO();

			// ホイールでズーム(マウスの下の位置が動かないように、中心もずらす)
			if (isHovered && io.MouseWheel != 0.0f)
			{
				Vector3 before;
				Vector3 unused;
				ScreenToRay(viewIndex, io.MousePos.x, io.MousePos.y, before, unused);
				const float rate = io.MouseWheel > 0.0f ? 1.0f / VIEW_ZOOM_STEP : VIEW_ZOOM_STEP;
				view.zoom = TClamp<float>::Clamp(view.zoom * rate, VIEW_ZOOM_MIN, VIEW_ZOOM_MAX);
				UpdateViewCamera(viewIndex);

				Vector3 after;
				ScreenToRay(viewIndex, io.MousePos.x, io.MousePos.y, after, unused);
				view.center += before - after;
				UpdateViewCamera(viewIndex);
			}

			// 右か中のドラッグで移動(絵の上で押した時だけ始める)
			if (isHovered && (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)))
			{
				m_dragViewIndex = viewIndex;
			}
			if (m_dragViewIndex != viewIndex) return;

			if (!ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGui::IsMouseDown(ImGuiMouseButton_Middle))
			{
				m_dragViewIndex = -1;
				return;
			}

			// 前のフレームのマウスの位置と、今の位置のワールドでの差だけ、逆に動かす
			Vector3 now;
			Vector3 last;
			Vector3 unused;
			ScreenToRay(viewIndex, io.MousePos.x, io.MousePos.y, now, unused);
			ScreenToRay(viewIndex, io.MousePos.x - io.MouseDelta.x, io.MousePos.y - io.MouseDelta.y, last, unused);
			view.center -= now - last;
			UpdateViewCamera(viewIndex);
		}


		void StageEditor::UpdatePerspectiveInput(const bool isHovered)
		{
			ViewState& view = m_viewList[ViewPerspective];
			const ImGuiIO& io = ImGui::GetIO();

			// ホイールで近づく・離れる
			if (isHovered && io.MouseWheel != 0.0f)
			{
				const float rate = io.MouseWheel > 0.0f ? 1.0f / VIEW_ZOOM_STEP : VIEW_ZOOM_STEP;
				view.distance = TClamp<float>::Clamp(view.distance * rate, PERSPECTIVE_DISTANCE_MIN, PERSPECTIVE_DISTANCE_MAX);
				UpdateViewCamera(ViewPerspective);
			}

			// 右・中のドラッグ(絵の上で押した時だけ始める)
			if (isHovered && (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)))
			{
				m_dragViewIndex = ViewPerspective;
			}
			if (m_dragViewIndex != ViewPerspective) return;

			if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				const float deltaYawDeg = io.MouseDelta.x * PERSPECTIVE_ROTATE_SPEED;
				const float deltaPitchDeg = io.MouseDelta.y * PERSPECTIVE_ROTATE_SPEED;
				if (io.KeyAlt)
				{
					// Alt + 右ドラッグで、注視点のまわりを回す
					view.yawDeg += deltaYawDeg;
					view.pitchDeg = TClamp<float>::Clamp(view.pitchDeg + deltaPitchDeg, PERSPECTIVE_PITCH_MIN, PERSPECTIVE_PITCH_MAX);
					UpdateViewCamera(ViewPerspective);
				}
				else
				{
					// 右ドラッグで、その場で見回す。押している間は WASD(Q/E)で飛べる
					LookAroundPerspective(deltaYawDeg, deltaPitchDeg);
					FlyPerspective();
				}
				return;
			}

			if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
			{
				// 中ドラッグで、画面の縦横に移動する
				// 注視点の距離で、1 ピクセルがワールドのどれだけの長さになるか(画角から求める)
				const float halfAngle = Math::DegToRad(PERSPECTIVE_VIEW_ANGLE_DEG) * 0.5f;
				const float worldPerPixel = 2.0f * view.distance * std::tan(halfAngle) / view.imageHeight;

				const Vector3 forward = MakeForward(view.yawDeg, view.pitchDeg);
				const Vector3& right = RenderingEngine::GetInstance().GetSubViewCamera(ViewPerspective).GetRight();
				// 画面の上の向き(見る向きと右の向きの両方に直角。上を向いている方を選ぶ)
				Vector3 up(
					forward.y * right.z - forward.z * right.y,
					forward.z * right.x - forward.x * right.z,
					forward.x * right.y - forward.y * right.x
				);
				if (up.y < 0.0f) up.Scale(-1.0f);

				// 絵をつかんで動かすように、カメラはマウスと逆に動かす
				view.center -= right * (io.MouseDelta.x * worldPerPixel);
				view.center += up * (io.MouseDelta.y * worldPerPixel);
				UpdateViewCamera(ViewPerspective);
				return;
			}

			m_dragViewIndex = -1;
		}


		void StageEditor::LookAroundPerspective(const float deltaYawDeg, const float deltaPitchDeg)
		{
			ViewState& view = m_viewList[ViewPerspective];

			// カメラの位置は変えずに向きだけ変えて、注視点を「カメラの位置 + 新しい向き × 距離」に置き直す
			const Vector3 position = view.center - MakeForward(view.yawDeg, view.pitchDeg) * view.distance;
			view.yawDeg += deltaYawDeg;
			view.pitchDeg = TClamp<float>::Clamp(view.pitchDeg + deltaPitchDeg, PERSPECTIVE_PITCH_MIN, PERSPECTIVE_PITCH_MAX);
			view.center = position + MakeForward(view.yawDeg, view.pitchDeg) * view.distance;
			UpdateViewCamera(ViewPerspective);
		}


		void StageEditor::FlyPerspective()
		{
			ViewState& view = m_viewList[ViewPerspective];
			const ImGuiIO& io = ImGui::GetIO();

			// 押しているキーで、進む向きを足し合わせる(W/S:前後 A/D:左右 E/Q:上下)
			const Vector3 forward = MakeForward(view.yawDeg, view.pitchDeg);
			const Vector3& right = RenderingEngine::GetInstance().GetSubViewCamera(ViewPerspective).GetRight();
			Vector3 move = Vector3::Zero;
			if (ImGui::IsKeyDown(ImGuiKey_W)) move += forward;
			if (ImGui::IsKeyDown(ImGuiKey_S)) move -= forward;
			if (ImGui::IsKeyDown(ImGuiKey_D)) move += right;
			if (ImGui::IsKeyDown(ImGuiKey_A)) move -= right;
			if (ImGui::IsKeyDown(ImGuiKey_E)) move += PERSPECTIVE_UP;
			if (ImGui::IsKeyDown(ImGuiKey_Q)) move -= PERSPECTIVE_UP;
			if (move.LengthSq() <= 0.0f) return;

			// 斜めに押しても速くならないように、長さを 1 にしてから速さを掛ける
			move.Normalize();
			const float speed = PERSPECTIVE_FLY_SPEED * (io.KeyShift ? PERSPECTIVE_FLY_FAST_RATE : 1.0f);
			// カメラと注視点を一緒に動かす(向きは変わらない)
			view.center += move * (speed * io.DeltaTime);
			UpdateViewCamera(ViewPerspective);
		}


		void StageEditor::UpdateViewCamera(const int viewIndex)
		{
			const ViewState& view = m_viewList[viewIndex];
			Camera& camera = RenderingEngine::GetInstance().GetSubViewCamera(viewIndex);

			// パース：注視点から、見る向きの逆へ距離だけ離れた所に置く
			if (viewIndex == ViewPerspective)
			{
				camera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Perspective);
				camera.SetViewAngle(Math::DegToRad(PERSPECTIVE_VIEW_ANGLE_DEG));
				camera.SetNear(PERSPECTIVE_NEAR);
				camera.SetFar(PERSPECTIVE_FAR);

				const Vector3 forward = MakeForward(view.yawDeg, view.pitchDeg);
				camera.SetTarget(view.center);
				camera.SetPosition(view.center - forward * view.distance);
				camera.SetUp(PERSPECTIVE_UP);
				camera.Update();
				return;
			}

			// 平行投影(遠くの物も同じ大きさに映る)。映る広さ = 画像の大きさ × ズーム
			// ※ 描き先の縦横の比と違っても、画像の縦横の比で出すので、ゆがまない
			camera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
			camera.SetWidth(view.imageWidth * view.zoom);
			camera.SetHeight(view.imageHeight * view.zoom);
			camera.SetNear(VIEW_NEAR);
			camera.SetFar(VIEW_FAR);

			// 断面の位置にカメラを置く(これより手前の物はカメラの後ろになるので映らない)
			Vector3 position = view.center;
			position.v[VIEW_AXIS[viewIndex]] = view.cut;
			camera.SetPosition(position);
			camera.SetTarget(position + VIEW_FORWARD[viewIndex]);
			camera.SetUp(VIEW_UP[viewIndex]);
			camera.Update();
		}


		void StageEditor::ScreenToRay(const int viewIndex, const float screenX, const float screenY, Vector3& start, Vector3& end)
		{
			const ViewState& view = m_viewList[viewIndex];
			Camera& camera = RenderingEngine::GetInstance().GetSubViewCamera(viewIndex);

			// 画像の中の割合(0 ~ 1)→ 画面の座標(-1 ~ 1。上がプラス)
			const float u = (screenX - view.imageX) / view.imageWidth;
			const float v = (screenY - view.imageY) / view.imageHeight;
			const float ndcX = u * 2.0f - 1.0f;
			const float ndcY = 1.0f - v * 2.0f;

			// ビュープロジェクション行列の逆で、ワールドに戻す(深度 0 = 近平面、深度 1 = 遠平面)
			// ※ 平行投影でもパースでも同じ式で使える(パースは遠いほど広がるので、2 点を結ぶとレイになる)
			const Matrix& inverse = camera.GetViewProjectionMatrixInv();
			Vector4 nearPoint(ndcX, ndcY, 0.0f, 1.0f);
			Vector4 farPoint(ndcX, ndcY, 1.0f, 1.0f);
			inverse.Apply(nearPoint);
			inverse.Apply(farPoint);
			start.Set(nearPoint.x / nearPoint.w, nearPoint.y / nearPoint.w, nearPoint.z / nearPoint.w);
			end.Set(farPoint.x / farPoint.w, farPoint.y / farPoint.w, farPoint.z / farPoint.w);
		}


		void StageEditor::PlaceObject(const int viewIndex, const float screenX, const float screenY)
		{
			const std::string modelPath = m_addModelPath;
			if (modelPath.empty())
			{
				m_message = "Select a model in the palette.";
				return;
			}

			// クリックした所を通るレイを飛ばして、最初に当たった面(床・壁)を探す
			Vector3 start;
			Vector3 end;
			ScreenToRay(viewIndex, screenX, screenY, start, end);
			const util::RayHit hit = util::RayCastClosest(start, end, nullptr, util::RayFilter::IgnoreCharacter);
			if (!hit.isHit)
			{
				m_message = "No surface to place on.";
				return;
			}

			// スナップする時は、見る向き以外の 2 つの軸だけそろえる(見る向きは、当たった面の位置のまま)
			// パースは見る向きが斜めなので、高さ(Y)を当たった面のままにして、横(X・Z)だけそろえる
			const int keepAxis = (viewIndex == ViewPerspective) ? 1 : VIEW_AXIS[viewIndex];
			Vector3 position = hit.position;
			if (m_isSnap && m_snapTranslation > 0.0f)
			{
				for (int axis = 0; axis < 3; ++axis)
				{
					if (axis == keepAxis) continue;
					position.v[axis] = std::round(position.v[axis] / m_snapTranslation) * m_snapTranslation;
				}
			}

			actor::StageObject* object = m_stage->AddObject(
				MakeNameFromPath(modelPath),
				modelPath,
				position,
				Vector3::Zero,
				Vector3::One,
				true
			);
			if (object == nullptr)
			{
				m_message = "Model not found.";
				return;
			}

			// 置いた物を選んで、すぐギズモで直せるようにする(元に戻せるように記録もする)
			OnObjectAdded(object);
			m_message = "Placed.";
		}


		void StageEditor::PickObject(const int viewIndex, const float screenX, const float screenY)
		{
			const ViewState& view = m_viewList[viewIndex];
			Camera& camera = RenderingEngine::GetInstance().GetSubViewCamera(viewIndex);
			const Matrix& viewProjection = camera.GetViewProjectionMatrix();
			// 半径を画面のピクセルにするのに使う(縦の拡大率。平行投影は 2 / 映る高さ、パースは 1 / tan(画角 / 2))
			const float projectionScaleY = camera.GetProjectionMatrix().m[1][1];

			int pickedIndex = -1;
			float pickedRadius = FLT_MAX;
			for (int i = 0; i < m_stage->GetObjectCount(); ++i)
			{
				const actor::StageObject* object = m_stage->GetStageObject(i);
				const Vector3& center = object->GetBoundingCenter();

				// 囲む球の中心を、画面の位置にする
				Vector4 point(center.x, center.y, center.z, 1.0f);
				viewProjection.Apply(point);
				if (point.w <= 0.0f) continue;
				const float ndcX = point.x / point.w;
				const float ndcY = point.y / point.w;
				const float ndcZ = point.z / point.w;
				// 断面より手前(カメラの後ろ)や、遠平面より奥の物は選ばない
				if (ndcZ < 0.0f || ndcZ > 1.0f) continue;

				const float objectScreenX = view.imageX + (ndcX * 0.5f + 0.5f) * view.imageWidth;
				const float objectScreenY = view.imageY + (0.5f - ndcY * 0.5f) * view.imageHeight;

				// 囲む球の半径も、画面のピクセルにする(パースは遠いほど小さくなるので w で割る。平行投影は w = 1)
				const float radius = object->GetBoundingRadius();
				const float screenRadius = radius * projectionScaleY * 0.5f * view.imageHeight / point.w;
				const float diffX = screenX - objectScreenX;
				const float diffY = screenY - objectScreenY;
				if (diffX * diffX + diffY * diffY > screenRadius * screenRadius) continue;

				// 重なっている中で一番小さい物(床や壁より、その上の小物を選びやすくする)
				if (radius < pickedRadius)
				{
					pickedRadius = radius;
					pickedIndex = i;
				}
			}

			if (pickedIndex < 0)
			{
				m_message = "Nothing here.";
				return;
			}
			m_selectedIndex = pickedIndex;
			m_message = "Selected.";
		}


		void StageEditor::UpdateGizmo()
		{
			const bool isUsing = ImGuizmo::IsUsing();

			// ギズモを放した瞬間に、コリジョンを作り直す
			actor::StageObject* object = m_stage->GetStageObject(m_selectedIndex);
			if (m_isGizmoUsingLastFrame && !isUsing && object != nullptr)
			{
				object->RebuildCollision();
			}
			m_isGizmoUsingLastFrame = isUsing;

			// 視点にギズモを出している時は、ゲームの画面には出さない(1 フレームに 1 つだけ)
			if (m_gizmoViewIndex >= 0) return;

			// ゲームの画面全体を使う。ウィンドウの後ろに描画して、ウィンドウを隠さないようにする
			const ImGuiIO& io = ImGui::GetIO();
			ManipulateSelected(g_camera3D->GetViewMatrix(), g_camera3D->GetProjectionMatrix(), ImGui::GetBackgroundDrawList(),
				0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y, false);
		}


		void StageEditor::ManipulateSelected(const Matrix& viewMatrix, const Matrix& projectionMatrix, ImDrawList* drawList,
			const float x, const float y, const float width, const float height, const bool isOrthographic)
		{
			actor::StageObject* object = m_stage->GetStageObject(m_selectedIndex);
			if (object == nullptr) return;

			ImGuizmo::SetOrthographic(isOrthographic);
			ImGuizmo::SetDrawlist(drawList);
			ImGuizmo::SetRect(x, y, width, height);

			// 位置・回転・スケールから、ワールド行列を作る
			// ※ ギズモは、行列を分解する時にスケールをプラスにしてしまうので、反転(マイナス)は外して渡して、戻す時に付け直す
			Vector3 position = object->GetPosition();
			Vector3 rotationDeg = object->GetRotationDeg();
			const Vector3& originalScale = object->GetScale();
			const Vector3 scaleSign(
				originalScale.x < 0.0f ? -1.0f : 1.0f,
				originalScale.y < 0.0f ? -1.0f : 1.0f,
				originalScale.z < 0.0f ? -1.0f : 1.0f
			);
			Vector3 scale(originalScale.x * scaleSign.x, originalScale.y * scaleSign.y, originalScale.z * scaleSign.z);
			Matrix worldMatrix;
			ImGuizmo::RecomposeMatrixFromComponents(position.v, rotationDeg.v, scale.v, &worldMatrix.m[0][0]);

			// スナップの値は、移動・スケールは x,y,z、回転は x だけ使われる
			float snap[3] = { 0.0f, 0.0f, 0.0f };
			switch (m_gizmoOperation)
			{
			case ImGuizmo::TRANSLATE: snap[0] = snap[1] = snap[2] = m_snapTranslation; break;
			case ImGuizmo::ROTATE:    snap[0] = snap[1] = snap[2] = m_snapRotation; break;
			case ImGuizmo::SCALE:     snap[0] = snap[1] = snap[2] = m_snapScale; break;
			default: break;
			}

			const ImGuizmo::OPERATION operation = static_cast<ImGuizmo::OPERATION>(m_gizmoOperation);
			// スケールはワールドではできないので、ローカルにする
			const ImGuizmo::MODE mode = (operation == ImGuizmo::SCALE) ? ImGuizmo::LOCAL : static_cast<ImGuizmo::MODE>(m_gizmoMode);

			if (ImGuizmo::Manipulate(&viewMatrix.m[0][0], &projectionMatrix.m[0][0], operation, mode, &worldMatrix.m[0][0], nullptr, m_isSnap ? snap : nullptr))
			{
				// ドラッグした結果を、位置・回転・スケールに戻す
				// ※ ImGuizmo::DecomposeMatrixToComponents は、Y の回転がちょうど ±90 度の時に X と Z の回転を捨ててしまうので、自分で分ける
				//    1~3行目 = 各軸(スケール込み)、4行目 = 位置
				Vector3 axisX(worldMatrix.m[0][0], worldMatrix.m[0][1], worldMatrix.m[0][2]);
				Vector3 axisY(worldMatrix.m[1][0], worldMatrix.m[1][1], worldMatrix.m[1][2]);
				Vector3 axisZ(worldMatrix.m[2][0], worldMatrix.m[2][1], worldMatrix.m[2][2]);
				position.Set(worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2]);
				scale.Set(axisX.Length() * scaleSign.x, axisY.Length() * scaleSign.y, axisZ.Length() * scaleSign.z);

				axisX.Normalize();
				axisY.Normalize();
				axisZ.Normalize();
				Matrix rotationMatrix = Matrix::Identity;
				rotationMatrix.m[0][0] = axisX.x; rotationMatrix.m[0][1] = axisX.y; rotationMatrix.m[0][2] = axisX.z;
				rotationMatrix.m[1][0] = axisY.x; rotationMatrix.m[1][1] = axisY.y; rotationMatrix.m[1][2] = axisY.z;
				rotationMatrix.m[2][0] = axisZ.x; rotationMatrix.m[2][1] = axisZ.y; rotationMatrix.m[2][2] = axisZ.z;
				rotationDeg = actor::StageObject::RotationMatrixToDeg(rotationMatrix);
				object->SetPosition(position);
				object->SetRotationDeg(rotationDeg);
				object->SetScale(scale);
				MarkDirty();
			}
		}


		void StageEditor::UpdateShortcut()
		{
			// 文字を入力している時は、切り替えない
			if (ImGui::GetIO().WantTextInput) return;

			if (ImGui::IsKeyPressed(ImGuiKey_1, false)) m_gizmoOperation = ImGuizmo::TRANSLATE;
			if (ImGui::IsKeyPressed(ImGuiKey_2, false)) m_gizmoOperation = ImGuizmo::ROTATE;
			if (ImGui::IsKeyPressed(ImGuiKey_3, false)) m_gizmoOperation = ImGuizmo::SCALE;

			// 動かしている途中は、消したり戻したりしない(動かす前の値の記録がずれるため)
			if (ImGuizmo::IsUsing() || m_isTransformEditing) return;

			// Delete キーで、選んでいる物を消す
			if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) DeleteSelected();
			// Ctrl + Z で、1 つ前に戻す
			if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z)) Undo();
		}


		void StageEditor::DeleteSelected()
		{
			actor::StageObject* object = m_stage->GetStageObject(m_selectedIndex);
			if (object == nullptr) return;

			// 消す前の中身を覚えておいて、戻す時に作り直す
			UndoRecord record;
			record.type = UndoType::Remove;
			record.object = object;
			record.before = MakeObjectData(*object);
			PushUndo(record);

			m_stage->RemoveObject(m_selectedIndex);
			m_selectedIndex = -1;
			m_transformBeforeObject = nullptr;
			m_message = "Deleted. (Ctrl+Z to undo)";
			MarkDirty();
		}


		void StageEditor::OnObjectAdded(const actor::StageObject* object)
		{
			UndoRecord record;
			record.type = UndoType::Add;
			record.object = object;
			record.before = MakeObjectData(*object);
			PushUndo(record);

			// 追加した物は一覧の最後に入る
			m_selectedIndex = m_stage->GetObjectCount() - 1;
			MarkDirty();
		}


		void StageEditor::Undo()
		{
			if (m_undoList.empty())
			{
				m_message = "Nothing to undo.";
				return;
			}

			const UndoRecord record = m_undoList.back();
			m_undoList.pop_back();

			switch (record.type)
			{
			case UndoType::Add:
			{
				// 置いた物を消す
				const int index = FindObjectIndex(record.object);
				if (index < 0) break;
				m_stage->RemoveObject(index);
				// 消した物より後ろの番号は 1 つずつ前にずれる
				if (m_selectedIndex == index) m_selectedIndex = -1;
				else if (m_selectedIndex > index) --m_selectedIndex;
				m_message = "Undo: add.";
				break;
			}
			case UndoType::Remove:
			{
				// 消した物を、消す前の中身で作り直す
				const ObjectData& data = record.before;
				const actor::StageObject* restored = m_stage->AddObject(
					data.name, data.modelPath, data.position, data.rotationDeg, data.scale, data.isCollision);
				if (restored == nullptr) break;
				// もっと前の記録が、消す前のアドレスを指しているので、新しいアドレスに置き換える
				for (UndoRecord& older : m_undoList)
				{
					if (older.object == record.object) older.object = restored;
				}
				m_selectedIndex = m_stage->GetObjectCount() - 1;
				m_message = "Undo: delete.";
				break;
			}
			case UndoType::Transform:
			{
				// 動かす前の位置・回転・スケールに戻す
				const int index = FindObjectIndex(record.object);
				if (index < 0) break;
				ApplyTransform(m_stage->GetStageObject(index), record.before);
				m_selectedIndex = index;
				m_message = "Undo: move.";
				break;
			}
			default:
				break;
			}

			// 戻した後の値を「動かす前」として覚え直す(戻したこと自体を、動かしたと記録しないように)
			actor::StageObject* selected = m_stage->GetStageObject(m_selectedIndex);
			m_transformBeforeObject = selected;
			if (selected != nullptr) m_transformBefore = MakeObjectData(*selected);
			MarkDirty();
		}


		void StageEditor::ClearUndo()
		{
			m_undoList.clear();
			m_transformBeforeObject = nullptr;
			m_isTransformEditing = false;
		}


		void StageEditor::PushUndo(const UndoRecord& record)
		{
			m_undoList.push_back(record);
			// 多すぎたら、一番古い物から消す
			if (static_cast<int>(m_undoList.size()) > MAX_UNDO_COUNT)
			{
				m_undoList.erase(m_undoList.begin());
			}
		}


		void StageEditor::UpdateTransformUndo()
		{
			const bool isEditing = ImGuizmo::IsUsing() || m_isInspectorEditing;
			actor::StageObject* selected = m_stage->GetStageObject(m_selectedIndex);

			// 動かし終わった瞬間：動かす前と違っていれば記録する
			if (m_isTransformEditing && !isEditing && m_transformBeforeObject != nullptr)
			{
				const int index = FindObjectIndex(m_transformBeforeObject);
				if (index >= 0)
				{
					const ObjectData after = MakeObjectData(*m_stage->GetStageObject(index));
					const Vector3 positionDiff = after.position - m_transformBefore.position;
					const Vector3 rotationDiff = after.rotationDeg - m_transformBefore.rotationDeg;
					const Vector3 scaleDiff = after.scale - m_transformBefore.scale;
					const bool isChanged = positionDiff.LengthSq() > 0.0f || rotationDiff.LengthSq() > 0.0f || scaleDiff.LengthSq() > 0.0f;
					if (isChanged)
					{
						UndoRecord record;
						record.type = UndoType::Transform;
						record.object = m_transformBeforeObject;
						record.before = m_transformBefore;
						PushUndo(record);
					}
				}
			}

			// 触っていない間は、今の値を「動かす前」として覚えておく(触り始めたフレームには、もう値が変わっているため)
			if (!isEditing)
			{
				m_transformBeforeObject = selected;
				if (selected != nullptr) m_transformBefore = MakeObjectData(*selected);
			}
			m_isTransformEditing = isEditing;
		}


		int StageEditor::FindObjectIndex(const actor::StageObject* object) const
		{
			if (object == nullptr) return -1;
			for (int i = 0; i < m_stage->GetObjectCount(); ++i)
			{
				if (m_stage->GetStageObject(i) == object) return i;
			}
			return -1;
		}


		StageEditor::ObjectData StageEditor::MakeObjectData(const actor::StageObject& object)
		{
			ObjectData data;
			data.name = object.GetName();
			data.modelPath = object.GetModelPath();
			data.position = object.GetPosition();
			data.rotationDeg = object.GetRotationDeg();
			data.scale = object.GetScale();
			data.isCollision = object.IsCollision();
			return data;
		}


		void StageEditor::ApplyTransform(actor::StageObject* object, const ObjectData& data)
		{
			if (object == nullptr) return;

			object->SetPosition(data.position);
			object->SetRotationDeg(data.rotationDeg);
			object->SetScale(data.scale);
			// 反転の有無が変わったら描画のグループを移して、コリジョンも今の位置で作り直す
			m_stage->RefreshObjectGroup(object);
			object->RebuildCollision();
		}
	}
}
