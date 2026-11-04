/**
 * @file Stage.cpp
 * @brief ステージ(置物の一覧と、その読み込み・保存)
 */
#include "stdafx.h"
#include "Stage.h"
#include "StageRenderer.h"
#include "Source/Actor/StageObject/StageObject.h"
#include "Source/Util/JsonConverter.h"
#include "level3D/TklFile.h"
#include <fstream>
#include <map>


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
			: m_stageRenderer(std::make_unique<StageRenderer>())
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
			// 視錐台の範囲内にある置物だけを、モデルの種類ごとにまとめて描く(インスタンシング描画)
			m_stageRenderer->Render(rc, m_objectList);
		}


		bool Stage::Load(const std::string& filePath)
		{
			nlohmann::json json;
			if (!util::JsonConverter::LoadFile(filePath, json)) return false;

			Clear();

			if (json.contains("hidePoints") && json["hidePoints"].is_array())
			{
				// 隠れ場所の読み込み
				for (const nlohmann::json& item : json["hidePoints"])
				{
					HidePoint hidePoint;
					hidePoint.position = util::JsonConverter::ToVector3(item, "position", Vector3::Zero);
					hidePoint.weight = util::JsonConverter::ToFloat(item, "weight", 1.0f);
					m_hidePointList.push_back(hidePoint);
				}
			}

			if (json.contains("objects") && json["objects"].is_array())
			{
				// 先に「モデル × 反転の有無」ごとの数を数えて、描画のグループをその大きさで作っておく
				// ※ しないと、追加するたびにグループが大きく作り直されて、読み込みが遅くなる
				std::map<std::pair<std::string, bool>, int> groupCountMap;
				for (const nlohmann::json& item : json["objects"])
				{
					const std::string modelPath = util::JsonConverter::ToString(item, "model");
					const Vector3 scale = util::JsonConverter::ToVector3(item, "scale", Vector3::One);
					groupCountMap[std::make_pair(modelPath, actor::StageObject::IsMirrored(scale))]++;
				}
				for (const auto& pair : groupCountMap)
				{
					// モデルが無いと、読み込みで止まってしまうので作らない(AddObject でも追加しない)
					if (!IsFileExist(pair.first.first)) continue;
					m_stageRenderer->Reserve(pair.first.first, pair.first.second, pair.second);
				}

				// 置物の読み込み
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
			}
			return true;
		}


		bool Stage::Save(const std::string& filePath) const
		{
			nlohmann::json hidePoints = nlohmann::json::array();
			// 隠れ場所の保存
			for (const auto& hidePoint : m_hidePointList)
			{
				nlohmann::json item;
				item["position"] = ToJsonArray(hidePoint.position);
				item["weight"] = hidePoint.weight;
				hidePoints.push_back(item);
			}


			nlohmann::json objects = nlohmann::json::array();
			// 置物の保存
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
			json["hidePoints"] = hidePoints;
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

			// 「モデル × 反転の有無」のグループに登録して、グループの描画をもらう(Model はグループに1つだけ)
			InstancedModelRender& instancedRender = m_stageRenderer->Register(modelPath, actor::StageObject::IsMirrored(scale));

			auto object = std::make_unique<actor::StageObject>();
			object->Initialize(name, modelPath, position, rotationDeg, scale, isCollision, instancedRender);
			object->Start();

			actor::StageObject* rawPtr = object.get();
			m_objectList.push_back(std::move(object));
			return rawPtr;
		}


		void Stage::RemoveObject(const int index)
		{
			if (index < 0 || index >= GetObjectCount()) return;

			// グループの登録を外してから消す
			const actor::StageObject& object = *m_objectList[index];
			m_stageRenderer->Unregister(object.GetModelPath(), object.IsInMirroredGroup());

			m_objectList.erase(m_objectList.begin() + index);
		}


		void Stage::Clear()
		{
			// 置物と隠れ場所の一覧を消す
			m_objectList.clear();
			m_hidePointList.clear();
			// 置物がグループのアドレスを持っているので、置物を消した後に消す
			m_stageRenderer->Clear();
		}


		void Stage::RefreshObjectGroup(actor::StageObject* object)
		{
			if (object == nullptr) return;

			const bool isMirrored = actor::StageObject::IsMirrored(object->GetScale());
			// 反転の有無が変わっていなければ、何もしない
			if (isMirrored == object->IsInMirroredGroup()) return;

			// 今のグループから外して、反転の有無が合うグループに入れ直す
			m_stageRenderer->Unregister(object->GetModelPath(), object->IsInMirroredGroup());
			InstancedModelRender& instancedRender = m_stageRenderer->Register(object->GetModelPath(), isMirrored);
			object->SetInstancedRender(instancedRender, isMirrored);
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
