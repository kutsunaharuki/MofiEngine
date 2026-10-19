/**
 * @file ParameterManager.cpp
 * @brief 調整用のパラメーター(JSON)の管理とホットリロード
 */
#include "stdafx.h"
#include "ParameterManager.h"
#include "JsonConverter.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <fstream>


namespace app
{
	namespace util
	{
		namespace
		{
			/** ファイルの更新を確認する間隔(秒)。毎フレーム確認すると無駄が多いので、間をあける */
			constexpr float CHECK_INTERVAL_SEC = 0.25f;
			/** JSON を保存するときの字下げの数 */
			constexpr int JSON_INDENT = 2;
		}


		/*************************************/
		// ParameterFile
		/*************************************/


		ParameterFile::ParameterFile(const std::string& filePath)
			: m_filePath(filePath)
			, m_json(nlohmann::json::object())
			, m_lastWriteTime(0)
			, m_version(0)
			, m_isDirty(false)
		{}


		ParameterFile::~ParameterFile()
		{}


		bool ParameterFile::Load()
		{
			m_lastWriteTime = JsonConverter::GetFileLastWriteTime(m_filePath);

			nlohmann::json json;
			if (!JsonConverter::LoadFile(m_filePath, json) || !json.is_object())
			{
				return false;
			}

			m_json = std::move(json);
			m_isDirty = false;
			m_version++;
			return true;
		}


		bool ParameterFile::Save()
		{
			std::ofstream file(m_filePath);
			if (!file.is_open()) return false;

			file << m_json.dump(JSON_INDENT) << std::endl;
			file.close();

			// 自分で保存した分は、ホットリロードで読み直さないようにする
			m_lastWriteTime = JsonConverter::GetFileLastWriteTime(m_filePath);
			m_isDirty = false;
			return true;
		}


		bool ParameterFile::ReloadIfModified()
		{
			const time_t writeTime = JsonConverter::GetFileLastWriteTime(m_filePath);
			if (writeTime == 0 || writeTime == m_lastWriteTime) return false;

			// 書きかけで読めなかったときは、次に保存されたときにもう一度読む
			return Load();
		}


		void ParameterFile::MarkChanged()
		{
			m_isDirty = true;
			m_version++;
		}


		float ParameterFile::GetFloat(const char* key, const float defaultValue) const
		{
			return JsonConverter::ToFloat(m_json, key, defaultValue);
		}


		int ParameterFile::GetInt(const char* key, const int defaultValue) const
		{
			return JsonConverter::ToInt(m_json, key, defaultValue);
		}


		bool ParameterFile::GetBool(const char* key, const bool defaultValue) const
		{
			return JsonConverter::ToBool(m_json, key, defaultValue);
		}


		Vector3 ParameterFile::GetVector3(const char* key, const Vector3& defaultValue) const
		{
			return JsonConverter::ToVector3(m_json, key, defaultValue);
		}




		/*************************************/
		// ParameterManager
		/*************************************/


		/** シングルトンインスタンス */
		ParameterManager* ParameterManager::m_instance = nullptr;


		ParameterManager::ParameterManager()
			: m_checkTimer(0.0f)
		{}


		ParameterManager::~ParameterManager()
		{}


		void ParameterManager::Update()
		{
#ifdef APP_ENABLE_PARAMETER_HOTRELOAD
			m_checkTimer -= g_gameTime->GetFrameDeltaTime();
			if (m_checkTimer > 0.0f) return;
			m_checkTimer = CHECK_INTERVAL_SEC;

			for (auto& file : m_fileList)
			{
				file->ReloadIfModified();
			}
#endif
		}


		ParameterFile* ParameterManager::Load(const char* filePath)
		{
			// 読み込み済みなら、それを使い回す
			for (auto& file : m_fileList)
			{
				if (file->GetFilePath() == filePath)
				{
					return file.get();
				}
			}

			auto file = std::make_unique<ParameterFile>(filePath);
			file->Load();
			ParameterFile* rawPtr = file.get();
			m_fileList.push_back(std::move(file));
			return rawPtr;
		}


		void ParameterManager::DrawImGui()
		{
			ImGui::Begin("Parameters");
			ImGui::TextDisabled("Edit values here, or edit the JSON file (hot reload).");

			for (auto& file : m_fileList)
			{
				// 保存していない変更があるときは、名前の後ろに * を付ける
				const std::string header = file->GetFilePath() + (file->IsDirty() ? " *" : "");
				ImGui::PushID(file.get());

				if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					bool isChanged = false;
					for (auto& item : file->GetJson().items())
					{
						const char* key = item.key().c_str();
						nlohmann::json& value = item.value();

						if (value.is_number_float())
						{
							float number = value.get<float>();
							// 値の大きさに合わせて、ドラッグの細かさを変える
							// ※ Windows.h の max マクロとぶつからないように、括弧で囲む
							const float speed = (std::max)(0.001f, std::fabs(number) * 0.01f);
							if (ImGui::DragFloat(key, &number, speed))
							{
								value = number;
								isChanged = true;
							}
						}
						else if (value.is_number_integer())
						{
							int number = value.get<int>();
							if (ImGui::DragInt(key, &number))
							{
								value = number;
								isChanged = true;
							}
						}
						else if (value.is_boolean())
						{
							bool flag = value.get<bool>();
							if (ImGui::Checkbox(key, &flag))
							{
								value = flag;
								isChanged = true;
							}
						}
						else if (value.is_array() && value.size() >= 2 && value.size() <= 4)
						{
							float numbers[4] = {};
							const int count = static_cast<int>(value.size());
							bool isAllNumber = true;
							for (int i = 0; i < count; ++i)
							{
								if (!value[i].is_number()) { isAllNumber = false; break; }
								numbers[i] = value[i].get<float>();
							}
							if (!isAllNumber)
							{
								ImGui::TextDisabled("%s : (not editable)", key);
								continue;
							}

							if (ImGui::DragScalarN(key, ImGuiDataType_Float, numbers, count, 0.1f))
							{
								for (int i = 0; i < count; ++i)
								{
									value[i] = numbers[i];
								}
								isChanged = true;
							}
						}
						else
						{
							ImGui::TextDisabled("%s : (not editable)", key);
						}
					}

					// 使う側に、値が変わったことを知らせる
					if (isChanged)
					{
						file->MarkChanged();
					}

					if (ImGui::Button("Save"))
					{
						file->Save();
					}
					ImGui::SameLine();
					if (ImGui::Button("Reload"))
					{
						file->Load();
					}
				}

				ImGui::PopID();
			}

			ImGui::End();
		}
	}
}
