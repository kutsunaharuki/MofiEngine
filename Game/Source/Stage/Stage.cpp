/**
 * @file Stage.cpp
 * @brief ステージ(置物の一覧と、その読み込み・保存)
 */
#include "stdafx.h"
#include "Stage.h"
#include "Source/Actor/StageObject/StageObject.h"
#include "Source/Util/JsonConverter.h"
#include "level3D/Level.h"
#include <fstream>


namespace app
{
	namespace stage
	{
		namespace
		{
			/** 保存する JSON の字下げ */
			constexpr int JSON_INDENT = 2;


			/**
			 * @brief ファイルがあるか
			 * @param filePath ファイルパス
			 * @return あれば true
			 */
			bool IsFileExist(const std::string& filePath)
			{
				std::ifstream file(filePath);
				return file.is_open();
			}


			/**
			 * @brief ファイルを置くフォルダを作る(1段だけ)
			 * @param filePath ファイルパス
			 * @details 無いフォルダに保存すると失敗するので、保存する前に作る
			 */
			void CreateParentDirectory(const std::string& filePath)
			{
				const size_t slash = filePath.find_last_of("/\\");
				if (slash == std::string::npos) return;

				// すでにあるときは失敗するが、問題ない
				CreateDirectoryA(filePath.substr(0, slash).c_str(), nullptr);
			}


			/**
			 * @brief Vector3 を JSON の配列 [x, y, z] にする
			 * @param value 値
			 * @return JSON の配列
			 */
			nlohmann::json ToJsonArray(const Vector3& value)
			{
				return nlohmann::json::array({ value.x, value.y, value.z });
			}
		}


		Stage::Stage()
		{}


		Stage::~Stage()
		{}


		void Stage::Update()
		{
			for (auto& object : m_objectList)
			{
				object->Update();
			}
		}


		void Stage::Render(RenderContext& rc)
		{
			for (auto& object : m_objectList)
			{
				object->Render(rc);
			}
		}


		bool Stage::Load(const std::string& filePath)
		{
			nlohmann::json json;
			if (!util::JsonConverter::LoadFile(filePath, json)) return false;

			Clear();

			if (!json.contains("objects") || !json["objects"].is_array()) return true;

			for (const nlohmann::json& item : json["objects"])
			{
				AddObject(
					util::JsonConverter::ToString(item, "name"),
					util::JsonConverter::ToString(item, "model"),
					util::JsonConverter::ToVector3(item, "position", Vector3::Zero),
					util::JsonConverter::ToVector3(item, "rotation", Vector3::Zero),
					util::JsonConverter::ToVector3(item, "scale", Vector3::One),
					util::JsonConverter::ToBool(item, "collision", true)
				);
			}
			return true;
		}


		bool Stage::Save(const std::string& filePath) const
		{
			nlohmann::json objects = nlohmann::json::array();
			for (const auto& object : m_objectList)
			{
				nlohmann::json item;
				item["name"] = object->GetName();
				item["model"] = object->GetModelPath();
				item["position"] = ToJsonArray(object->GetPosition());
				item["rotation"] = ToJsonArray(object->GetRotationDeg());
				item["scale"] = ToJsonArray(object->GetScale());
				item["collision"] = object->IsCollision();
				objects.push_back(item);
			}

			nlohmann::json json;
			json["objects"] = objects;

			CreateParentDirectory(filePath);
			std::ofstream file(filePath);
			if (!file.is_open()) return false;

			file << json.dump(JSON_INDENT) << std::endl;
			return true;
		}


		bool Stage::ImportTkl(const char* tklFilePath, const std::string& modelDirectory)
		{
			if (!IsFileExist(tklFilePath)) return false;

			Clear();

			// Level が、3ds Max の Z-up から Y-up への変換をしてくれるので、それを使う
			Level level;
			level.Init(tklFilePath, [&](LevelObjectData& objectData)
			{
				// 名前はワイド文字なので、普通の文字列にする(モデル名は英数字だけ)
				std::string name;
				for (const wchar_t* c = objectData.name; *c != L'\0'; ++c)
				{
					name += static_cast<char>(*c);
				}

				AddObject(
					name,
					modelDirectory + name + ".tkm",
					objectData.position,
					actor::StageObject::ToRotationDeg(objectData.rotation),
					objectData.scale,
					true
				);

				// true を返すと、Level は自分でモデル(MapChip)を作らない
				return true;
			});

			return !m_objectList.empty();
		}


		actor::StageObject* Stage::AddObject(
			const std::string& name,
			const std::string& modelPath,
			const Vector3& position,
			const Vector3& rotationDeg,
			const Vector3& scale,
			const bool isCollision)
		{
			// モデルが無いと、読み込みで止まってしまうので追加しない
			if (!IsFileExist(modelPath))
			{
				OutputDebugStringA(("[Stage] model not found: " + modelPath + "\n").c_str());
				return nullptr;
			}

			auto object = std::make_unique<actor::StageObject>();
			object->Initialize(name, modelPath, position, rotationDeg, scale, isCollision);
			object->Start();

			actor::StageObject* rawPtr = object.get();
			m_objectList.push_back(std::move(object));
			return rawPtr;
		}


		void Stage::RemoveObject(const int index)
		{
			if (index < 0 || index >= GetObjectCount()) return;
			m_objectList.erase(m_objectList.begin() + index);
		}


		void Stage::Clear()
		{
			m_objectList.clear();
		}


		actor::StageObject* Stage::GetStageObject(const int index)
		{
			if (index < 0 || index >= GetObjectCount()) return nullptr;
			return m_objectList[index].get();
		}
	}
}
