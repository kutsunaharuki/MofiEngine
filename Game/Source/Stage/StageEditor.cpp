/**
 * @file StageEditor.cpp
 * @brief ステージエディタ(Debug の時だけ使う)
 */
#include "stdafx.h"
#include "StageEditor.h"
#include "Stage.h"
#include "Source/Actor/StageObject/StageObject.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include <cstring>


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
		{
			m_filter[0] = '\0';
			strncpy_s(m_addModelPath, DEFAULT_ADD_MODEL_PATH, _TRUNCATE);
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

			// 名前が変わってもウィンドウが別物にならないように、### の後ろを ID にする
			const char* title = m_isDirty ? "Stage Editor *###StageEditor" : "Stage Editor###StageEditor";
			ImGui::Begin(title);

			DrawFileMenu();
			ImGui::Separator();
			DrawGizmoSetting();
			ImGui::Separator();
			DrawObjectList();
			ImGui::Separator();
			DrawInspector();
			ImGui::Separator();
			DrawAddObject();

			ImGui::End();

			// ギズモはウィンドウの外(ゲームの画面)に出す
			UpdateGizmo();
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
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

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


		void StageEditor::DrawObjectList()
		{
			ImGui::Text("Objects: %d", m_stage->GetObjectCount());
			ImGui::InputText("Filter", m_filter, sizeof(m_filter));

			ImGui::BeginChild("ObjectList", ImVec2(0.0f, OBJECT_LIST_HEIGHT), ImGuiChildFlags_Borders);
			for (int i = 0; i < m_stage->GetObjectCount(); ++i)
			{
				const actor::StageObject* object = m_stage->GetStageObject(i);
				if (!IsMatchFilter(object->GetName(), m_filter)) continue;

				// 同じ名前が並ぶので、番号も出す。## の後ろで ID を分ける
				const std::string label = std::to_string(i) + ": " + object->GetName() + "##" + std::to_string(i);
				if (ImGui::Selectable(label.c_str(), i == m_selectedIndex))
				{
					m_selectedIndex = i;
				}
			}
			ImGui::EndChild();
		}


		void StageEditor::DrawInspector()
		{
			actor::StageObject* object = m_stage->GetStageObject(m_selectedIndex);
			if (object == nullptr)
			{
				ImGui::TextDisabled("No object selected.");
				return;
			}

			ImGui::Text("%s", object->GetName().c_str());
			ImGui::TextDisabled("%s", object->GetModelPath().c_str());

			// 値を変えている間は見た目だけ動かして、放した時にコリジョンを作り直す
			bool isEditEnd = false;

			Vector3 position = object->GetPosition();
			if (ImGui::DragFloat3("Position", position.v, 1.0f))
			{
				object->SetPosition(position);
				MarkDirty();
			}
			isEditEnd |= ImGui::IsItemDeactivatedAfterEdit();

			Vector3 rotationDeg = object->GetRotationDeg();
			if (ImGui::DragFloat3("Rotation", rotationDeg.v, 1.0f))
			{
				object->SetRotationDeg(rotationDeg);
				MarkDirty();
			}
			isEditEnd |= ImGui::IsItemDeactivatedAfterEdit();

			Vector3 scale = object->GetScale();
			if (ImGui::DragFloat3("Scale", scale.v, 0.01f))
			{
				object->SetScale(scale);
				MarkDirty();
			}
			isEditEnd |= ImGui::IsItemDeactivatedAfterEdit();

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
				if (m_stage->AddObject(object->GetName(), object->GetModelPath(), newPosition, object->GetRotationDeg(), object->GetScale(), isCollision) != nullptr)
				{
					m_selectedIndex = m_stage->GetObjectCount() - 1;
					MarkDirty();
				}
				return;
			}

			ImGui::SameLine();
			if (ImGui::Button("Delete"))
			{
				m_stage->RemoveObject(m_selectedIndex);
				m_selectedIndex = -1;
				MarkDirty();
			}
		}


		void StageEditor::DrawAddObject()
		{
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
					m_selectedIndex = m_stage->GetObjectCount() - 1;
					m_message = "Added.";
					MarkDirty();
				}
				else
				{
					m_message = "Model not found.";
				}
			}
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

			if (object == nullptr) return;

			// ゲームの画面全体を使う。ウィンドウの後ろに描画して、ウィンドウを隠さないようにする
			const ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
			ImGuizmo::SetRect(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y);

			// 位置・回転・スケールから、ワールド行列を作る
			Vector3 position = object->GetPosition();
			Vector3 rotationDeg = object->GetRotationDeg();
			Vector3 scale = object->GetScale();
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

			const Matrix& viewMatrix = g_camera3D->GetViewMatrix();
			const Matrix& projectionMatrix = g_camera3D->GetProjectionMatrix();
			if (ImGuizmo::Manipulate(&viewMatrix.m[0][0], &projectionMatrix.m[0][0], operation, mode, &worldMatrix.m[0][0], nullptr, m_isSnap ? snap : nullptr))
			{
				// ドラッグした結果を、位置・回転・スケールに戻す
				ImGuizmo::DecomposeMatrixToComponents(&worldMatrix.m[0][0], position.v, rotationDeg.v, scale.v);
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
		}
	}
}
