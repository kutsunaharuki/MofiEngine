/**
 * @file Stage.cpp
 * @brief ステージ(置物の一覧と、その読み込み・保存)
 */
#include "stdafx.h"
#include "Stage.h"
#include "Source/Actor/StageObject/StageObject.h"
#include "Source/Util/JsonConverter.h"
#include "level3D/TklFile.h"
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
			/** 2.視錐台の範囲内にあるオブジェクトのみを描画 */
			// このフレームのカメラの映る範囲(視錐台)
			Frustum frustum;
			frustum.Update(g_camera3D->GetViewProjectionMatrix());

			int visibleCount = 0;
			for (auto& object : m_objectList)
			{
				// 映る範囲外なら描画に登録しない(G-Buffer にもファインダーにも描かれない)
				if (!frustum.IsInside(object->GetBoundingCenter(), object->GetBoundingRadius())) continue;

				object->Render(rc);
				visibleCount++;
			}

			// Profiler に描いた数を出す
			Profiler::GetInstance().SetCounter("Stage visible", visibleCount);
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

			// ※ エンジンの Level は、スケールを行列の長さで求めるので、3ds Max のミラーで置いた物の
			//    反転(マイナスのスケール)が消えて、向きも壊れる。なので、行列を自分で分解する
			TklFile tklFile;
			tklFile.Load(tklFilePath);
			tklFile.QuaryObject([&](TklFile::SObject& object)
			{
				// Level と同じく、ルート(Scene Root)の直下のオブジェクトだけを置く
				if (object.parentNo != 0) return;

				// 3ds Max のワールド行列(Z-up)。1~3行目が各軸(スケール込み)、4行目が位置
				Vector3 axisX(object.bindPose[0][0], object.bindPose[0][1], object.bindPose[0][2]);
				Vector3 axisY(object.bindPose[1][0], object.bindPose[1][1], object.bindPose[1][2]);
				Vector3 axisZ(object.bindPose[2][0], object.bindPose[2][1], object.bindPose[2][2]);
				const Vector3 maxPosition(object.bindPose[3][0], object.bindPose[3][1], object.bindPose[3][2]);

				Vector3 maxScale(axisX.Length(), axisY.Length(), axisZ.Length());
				axisX.Normalize();
				axisY.Normalize();
				axisZ.Normalize();

				// X軸 × Y軸 が Z軸と逆を向いていたら、反転している
				// X のスケールをマイナスにして、X軸を戻すと、残りは正しい回転になる
				Vector3 crossXY;
				crossXY.Cross(axisX, axisY);
				if (crossXY.Dot(axisZ) < 0.0f)
				{
					maxScale.x = -maxScale.x;
					axisX.Scale(-1.0f);
				}

				Matrix rotationMatrix = Matrix::Identity;
				rotationMatrix.m[0][0] = axisX.x; rotationMatrix.m[0][1] = axisX.y; rotationMatrix.m[0][2] = axisX.z;
				rotationMatrix.m[1][0] = axisY.x; rotationMatrix.m[1][1] = axisY.y; rotationMatrix.m[1][2] = axisY.z;
				rotationMatrix.m[2][0] = axisZ.x; rotationMatrix.m[2][1] = axisZ.y; rotationMatrix.m[2][2] = axisZ.z;
				Quaternion maxRotation;
				maxRotation.SetRotation(rotationMatrix);

				// Z-up から Y-up に変換する(Level と同じ変換。X軸まわりに90度回すのと同じ)
				const Vector3 position(maxPosition.x, maxPosition.z, -maxPosition.y);
				Quaternion rotation = maxRotation;
				rotation.y = maxRotation.z;
				rotation.z = -maxRotation.y;
				const Vector3 scale(maxScale.x, maxScale.z, maxScale.y);

				const std::string name = object.name.get();
				AddObject(
					name,
					modelDirectory + name + ".tkm",
					position,
					actor::StageObject::ToRotationDeg(rotation),
					scale,
					true
				);
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


		const actor::StageObject* Stage::GetStageObject(const int index) const
		{
			if (index < 0 || index >= GetObjectCount()) return nullptr;
			return m_objectList[index].get();
		}
	}
}
