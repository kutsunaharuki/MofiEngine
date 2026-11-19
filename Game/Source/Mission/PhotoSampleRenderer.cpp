/**
 * @file PhotoSampleRenderer.cpp
 * @brief ミッションの見本の絵をオフスクリーンで1回だけ描く
 */
#include "stdafx.h"
#include "PhotoSampleRenderer.h"
#include "Source/Actor/Character/Enemy/EnemyBase.h"
#include "Source/Actor/Character/Enemy/Object/EnemyParameter.h"
#include "Source/Util/ParameterManager.h"
#include "Source/Util/JsonConverter.h"
#include "Source/Actor/StageObject/StageObject.h"
#include <cmath>
#include <string>


namespace app
{
	namespace mission
	{
		namespace
		{
			/**
			 * @brief 人形を置く位置
			 * @details ステージの影や明かりが入らないように、影の範囲の外(ずっと下)に置く
			 */
			const Vector3 DUMMY_POSITION = Vector3(0.0f, -10000.0f, 0.0f);

			/** 描き先の横幅(16:9。エンジンのカメラは画面全体の比で描くので、比を合わせる) */
			constexpr int SAMPLE_RT_WIDTH = 640;
			/** 描き先の縦幅 */
			constexpr int SAMPLE_RT_HEIGHT = 360;
			/** 背景の色(α は 1 = 透けない) */
			const float SAMPLE_CLEAR_COLOR[4] = { 0.18f, 0.18f, 0.2f, 1.0f };

			/** 画角(ファインダーの広角とそろえる) */
			constexpr float SAMPLE_VIEW_ANGLE_DEG = 60.0f;
			/** カメラの近平面 */
			constexpr float SAMPLE_CAMERA_NEAR = 1.0f;
			/** カメラの遠平面 */
			constexpr float SAMPLE_CAMERA_FAR = 10000.0f;

			/** 測り直してカメラを直す回数 */
			constexpr int REFINE_COUNT = 4;
			/** 大きさが 0 以下の見本の時に使う大きさ */
			constexpr float DEFAULT_SIZE = 0.4f;
			/** 人形の高さが取れない時に使う高さ */
			constexpr float DEFAULT_HEIGHT = 150.0f;
			/** 上下の角度の限界(真上・真下だとカメラの上の向きが決まらない) */
			constexpr float PITCH_LIMIT_DEG = 80.0f;
			/** 半分 */
			constexpr float HALF = 0.5f;

			/** セット(床・壁・絵画)の JSON */
			constexpr const char* SAMPLE_SET_PATH = "Assets/parameter/Mission/PhotoSampleSet.json";
			/** 人形の足元から、背中側の壁までの距離(JSON に無い時) */
			constexpr float DEFAULT_WALL_DISTANCE = 60.0f;
			/** 向きが決まらないくらい短い長さ(の 2 乗) */
			constexpr float MIN_LENGTH_SQ = 0.0001f;
			/** 客と壁の間を、少なくともこれだけ空ける(壁にめり込まない) */
			constexpr float PERSON_WALL_MIN_DISTANCE = 40.0f;
			/** 客の奥行きの最小(カメラの近くに置いて、画面をふさがない) */
			constexpr float PERSON_MIN_DEPTH_RATE = 0.5f;


			/**
			 * @brief ボーンの名前からボーンの番号を探す
			 * @param modelRender モデル
			 * @param boneName ボーンの名前(JSON の文字。英数字だけ)
			 * @return ボーンの番号(名前が空・見つからなければ -1)
			 */
			int FindBone(const ModelRender& modelRender, const std::string& boneName)
			{
				if (boneName.empty()) return -1;
				const std::wstring name(boneName.begin(), boneName.end());
				return modelRender.FindBoneID(name.c_str());
			}
		}


		PhotoSampleRenderer::PhotoSampleRenderer()
			: m_headBoneNo(-1)
			, m_headForwardLocal(0.0f, 0.0f, 1.0f)
			, m_leftHandBoneNo(-1)
			, m_rightHandBoneNo(-1)
			, m_setFile(nullptr)
			, m_setVersion(0)
			, m_wallDistance(DEFAULT_WALL_DISTANCE)
			, m_poseTimer(0.0f)
			, m_isInitialized(false)
			, m_isPosed(false)
			, m_isRendered(false)
		{}


		PhotoSampleRenderer::~PhotoSampleRenderer()
		{}


		void PhotoSampleRenderer::Initialize(const MissionData& mission)
		{
			// 見本が無い・誰を撮るか決まっていないミッションは描かない
			if (!mission.hasSample) return;
			if (mission.target == actor::EnemyBase::EnemyType::Max) return;

			m_mission = mission;

			// 悪党の JSON から、頭のボーンの名前・顔の前の向き・スケールを読む(悪党と同じ値にする)
			util::Parameter<actor::EnemyParameter> parameter;
			parameter.Initialize(actor::EnemyBase::GetParameterPath(mission.target));
			const actor::EnemyParameter& enemyParameter = parameter.Get();

			// 悪党と同じモデルとアニメーション
			const actor::ModelData& modelData = actor::EnemyBase::GetModelData(mission.target);
			m_animationClips = std::make_unique<AnimationClip[]>(modelData.clipNum);
			for (int i = 0; i < modelData.clipNum; ++i)
			{
				m_animationClips[i].Load(modelData.animationData[i].fileName);
				m_animationClips[i].SetLoopFlag(modelData.animationData[i].isLoop);
			}

			// 影は落とさない(本編の影に入らない)。影を受けるにしないと、フォワード用のモデルが作られない
			m_dummy.Init(modelData.fileName, m_animationClips.get(), modelData.upAxis, modelData.clipNum, false, true);
			m_dummy.SetPosition(DUMMY_POSITION);
			m_dummy.SetScale(enemyParameter.initialScale);
			m_dummy.PlayAnimation(static_cast<int>(actor::EnemyBase::EnemyAnimation::Steal));

			// 頭のボーン(悪党の Start と同じ探し方)
			const std::wstring headBoneName(enemyParameter.headBone.begin(), enemyParameter.headBone.end());
			m_headBoneNo = m_dummy.FindBoneID(headBoneName.c_str());
			m_headForwardLocal = enemyParameter.headForwardLocal;

			// 手のボーン(盗んだ絵画を持たせる所。悪党の JSON の leftHandBone / rightHandBone)
			m_leftHandBoneNo = FindBone(m_dummy, enemyParameter.leftHandBone);
			m_rightHandBoneNo = FindBone(m_dummy, enemyParameter.rightHandBone);

			// 描き先
			// ※ Create は const でない float[4] を受け取るので、定数をコピーしてから渡す
			float clearColor[4] = { SAMPLE_CLEAR_COLOR[0], SAMPLE_CLEAR_COLOR[1], SAMPLE_CLEAR_COLOR[2], SAMPLE_CLEAR_COLOR[3] };
			m_renderTarget.Create(
				SAMPLE_RT_WIDTH,
				SAMPLE_RT_HEIGHT,
				1,
				1,
				DXGI_FORMAT_R16G16B16A16_FLOAT,		// フォワード用のモデル(model.fx)の描き先と同じ
				DXGI_FORMAT_D32_FLOAT,
				clearColor
			);

			// カメラ(位置と向きは、ポーズを止めてから決める)
			m_camera.SetViewAngle(Math::DegToRad(SAMPLE_VIEW_ANGLE_DEG));
			m_camera.SetNear(SAMPLE_CAMERA_NEAR);
			m_camera.SetFar(SAMPLE_CAMERA_FAR);
			m_camera.SetWidth(static_cast<float>(SAMPLE_RT_WIDTH));
			m_camera.SetHeight(static_cast<float>(SAMPLE_RT_HEIGHT));
			m_camera.SetUp(Vector3::Up);

			// セット(床・壁・絵画)。置くのは、ポーズを止めて顔の向きが決まってから
			InitializeSet();

			m_isInitialized = true;
		}


		void PhotoSampleRenderer::Update()
		{
			if (!m_isInitialized) return;

			// 描き終わった後は、セットの JSON が書き換わった時だけ描き直す(ホットリロード。ポーズはそのまま)
			if (m_isRendered)
			{
				if (m_setFile == nullptr || m_setFile->GetVersion() == m_setVersion) return;
				InitializeSet();
				m_isRendered = false;
			}

			// 1. ポーズ: 盗むアニメーションを poseTime まで進めて止める
			// ※ アニメーションは「経過時間 × 速さ」で進むだけ(時間を指定して飛ぶ関数は無い)
			if (!m_isPosed)
			{
				m_poseTimer += g_gameTime->GetFrameDeltaTime();
				if (m_poseTimer >= m_mission.sample.poseTime)
				{
					m_dummy.SetAnimationSpeed(0.0f);
					m_isPosed = true;
				}
				m_dummy.Update();
				// ※ 止めたフレームは、スケルトンがまだ 1 つ前のポーズなので、次のフレームで描く
				return;
			}

			// 止めたポーズでボーンを更新(描く時に GPU に送られるボーン行列もこの値)
			m_dummy.Update();

			// 2. 見本の数値 → カメラ
			PhotoCamera();

			// 3. セットを置く: 背中側に絵画の掛かった壁、両手に盗んだ絵画、画面の端に客
			PlaceSet();

			// 4. 見本そのものを照合にかけて確かめる
			CheckSample(MeasureDummy());

			// 5. このフレームの描画で 1 回だけ描く(以後は描かない。絵は描き先に残る)
			std::vector<Model*> modelList;
			modelList.push_back(&m_dummy.GetFinderModel());
			for (SetPiece& piece : m_setPieceList)
			{
				modelList.push_back(&piece.render->GetFinderModel());
			}
			for (SetPerson& person : m_setPersonList)
			{
				modelList.push_back(&person.render->GetFinderModel());
			}
			RenderingEngine::GetInstance().RequestSnapshot(m_renderTarget, m_camera, modelList);
			m_isRendered = true;
		}


		void PhotoSampleRenderer::PhotoCamera()
		{
			Vector3 headPosition;
			Vector3 headForward;
			if (!CalcHead(m_dummy, m_headBoneNo, m_headForwardLocal, headPosition, headForward)) return;

			const PhotoSample& sample = m_mission.sample;

			// ① 顔 → カメラの向き(水平): 顔の正面を yawDeg だけ回す(測り方の atan2 と同じ向きになる回し方)
			Vector3 forwardFlat(headForward.x, 0.0f, headForward.z);
			forwardFlat.Normalize();
			const float yaw = Math::DegToRad(sample.yawDeg);
			const float sinYaw = std::sin(yaw);
			const float cosYaw = std::cos(yaw);
			const Vector3 toCameraFlat(
				forwardFlat.x * cosYaw + forwardFlat.z * sinYaw,
				0.0f,
				-forwardFlat.x * sinYaw + forwardFlat.z * cosYaw);

			// ② 上下: pitchDeg だけ持ち上げる(真上・真下にならないように止める)
			const float pitch = Math::DegToRad(TClamp<float>::Clamp(sample.pitchDeg, -PITCH_LIMIT_DEG, PITCH_LIMIT_DEG));
			const Vector3 toCamera(
				toCameraFlat.x * std::cos(pitch),
				std::sin(pitch),
				toCameraFlat.z * std::cos(pitch));

			// ③ 距離: 画面の高さに写る割合 = 実際の高さ ÷ (2・距離・tan(画角/2))
			// ※ カメラは頭から toCamera の向きに置く → 左右・上下の角度は距離や注視点を変えてもずれない
			const float tanHalf = std::tan(Math::DegToRad(SAMPLE_VIEW_ANGLE_DEG) * HALF);
			float height = headPosition.y - DUMMY_POSITION.y;
			if (height <= 0.0f) height = DEFAULT_HEIGHT;
			const float size = sample.size > 0.0f ? sample.size : DEFAULT_SIZE;
			float distance = height / (2.0f * tanHalf * size);

			// ④ 注視点: 頭と足元の真ん中
			Vector3 target = (headPosition + DUMMY_POSITION) * HALF;
			SetCamera(target, headPosition, toCamera, distance);

			// ⑥ 測り直し: 近似のずれを、実際に測って直す(大きさ → 距離、位置 → 注視点)
			for (int i = 0; i < REFINE_COUNT; ++i)
			{
				const ShotMeasure measure = MeasureDummy();
				if (!measure.isMeasured) return;

				// 大きく写りすぎていれば離れる(大きさは距離に反比例)
				if (measure.size > 0.0f) distance *= measure.size / size;

				// ⑤ 位置: 見本より右に写っていれば、注視点を右へ(画面の中で左へ動く)。下に写っていれば注視点を下へ
				Vector3 toTarget = target - m_camera.GetPosition();
				const float targetDistance = toTarget.Length();
				const float visibleHeight = 2.0f * targetDistance * tanHalf;
				const float visibleWidth = visibleHeight * m_camera.GetAspect();
				target += m_camera.GetRight() * ((measure.position.x - sample.position.x) * visibleWidth);
				target -= m_camera.GetUp() * ((measure.position.y - sample.position.y) * visibleHeight);

				SetCamera(target, headPosition, toCamera, distance);
			}
		}


		void PhotoSampleRenderer::SetCamera(const Vector3& target, const Vector3& headPosition, const Vector3& toCamera, const float distance)
		{
			m_camera.SetPosition(headPosition + toCamera * distance);
			m_camera.SetTarget(target);
			m_camera.Update();
		}


		ShotMeasure PhotoSampleRenderer::MeasureDummy()
		{
			Vector3 headPosition;
			Vector3 headForward;
			if (!CalcHead(m_dummy, m_headBoneNo, m_headForwardLocal, headPosition, headForward)) return ShotMeasure();

			// 足元は人形の位置(悪党の GetTransform().GetWorldPosition() と同じ)
			return MeasureShape(headPosition, headForward, DUMMY_POSITION, m_camera);
		}


		void PhotoSampleRenderer::CheckSample(const ShotMeasure& measure)
		{
			const MatchTolerance& tolerance = m_mission.tolerance;
			const float halfWidth = static_cast<float>(UI_SPACE_WIDTH) * HALF - tolerance.screenMargin;
			const float halfHeight = static_cast<float>(UI_SPACE_HEIGHT) * HALF - tolerance.screenMargin;

			// 判定と同じ条件(顔が見えている・枠の中)を、見本の絵が満たしているか
			const char* reason = nullptr;
			if (!measure.isMeasured)                                    reason = "head bone not found";
			else if (measure.faceAngleDeg > tolerance.faceAngleMax)     reason = "face not visible (yawDeg/pitchDeg)";
			else if (!measure.isFaceInFront)                            reason = "face behind camera";
			else if (std::abs(measure.faceScreenPosition.x) > halfWidth
				  || std::abs(measure.faceScreenPosition.y) > halfHeight) reason = "face out of frame (size/position)";

			if (reason == nullptr) return;

			// ※ K2_ASSERT のメッセージは 256 バイトまでなので、短くする
			std::string message = "[PhotoSample] ";
			message += reason;
			message += "\n";
			OutputDebugStringA(message.c_str());
			K2_ASSERT(false, message.c_str());
		}


		void PhotoSampleRenderer::InitializeSet()
		{
			// ホットリロードで呼び直した時は、前のセットを消してから作り直す
			m_setPieceList.clear();
			m_setPersonList.clear();

			m_setFile = util::ParameterManager::GetInstance()->Load(SAMPLE_SET_PATH);
			if (m_setFile == nullptr) return;
			m_setVersion = m_setFile->GetVersion();

			const nlohmann::json& json = m_setFile->GetJson();
			m_wallDistance = util::JsonConverter::ToFloat(json, "wallDistance", DEFAULT_WALL_DISTANCE);

			// 客(画面の端で見切れている人)
			if (json.contains("people"))
			{
				for (const nlohmann::json& item : json["people"])
				{
					const std::string modelPath = util::JsonConverter::ToString(item, "model");
					const std::string animationPath = util::JsonConverter::ToString(item, "animation");
					if (modelPath.empty() || animationPath.empty()) continue;

					SetPerson person;
					person.screenX = util::JsonConverter::ToFloat(item, "screenX", 0.0f);
					person.depthRate = util::JsonConverter::ToFloat(item, "depthRate", 1.0f);
					person.turnDeg = util::JsonConverter::ToFloat(item, "turnDeg", 0.0f);
					person.scale = util::JsonConverter::ToVector3(item, "scale", Vector3::One);

					// 1 つのアニメーションを流しておく(描く時のポーズになる)
					person.animationClips = std::make_unique<AnimationClip[]>(1);
					person.animationClips[0].Load(animationPath.c_str());
					person.animationClips[0].SetLoopFlag(true);

					// 訪問者と同じ上の向き(Z)。影は落とさない。影を受けるにしないと、フォワード用のモデルが作られない
					person.render = std::make_unique<ModelRender>();
					person.render->Init(modelPath.c_str(), person.animationClips.get(), enModelUpAxisZ, 1, false, true);
					person.render->PlayAnimation(0);
					m_setPersonList.push_back(std::move(person));
				}
			}

			if (!json.contains("pieces")) return;

			for (const nlohmann::json& item : json["pieces"])
			{
				const std::string modelPath = util::JsonConverter::ToString(item, "model");
				if (modelPath.empty()) continue;

				SetPiece piece;
				piece.position = util::JsonConverter::ToVector3(item, "position", Vector3::Zero);
				// 回転はステージ(Museum.json)と同じ形(X → Y → Z の度)なので、置物と同じ関数で変える
				piece.rotation = actor::StageObject::ToQuaternion(util::JsonConverter::ToVector3(item, "rotation", Vector3::Zero));
				piece.scale = util::JsonConverter::ToVector3(item, "scale", Vector3::One);
				// "hold":true は、人形の両手に持たせる物(盗んだ絵画)
				piece.isHeld = util::JsonConverter::ToBool(item, "hold", false);

				// ステージと同じ上の向き(Z)。影は落とさない(本編の影に入らない)。影を受けるにしないと、フォワード用のモデルが作られない
				piece.render = std::make_unique<ModelRender>();
				piece.render->Init(modelPath.c_str(), nullptr, enModelUpAxisZ, 0, false, true);
				m_setPieceList.push_back(std::move(piece));
			}
		}


		void PhotoSampleRenderer::PlaceSet()
		{
			if (m_setPieceList.empty() && m_setPersonList.empty()) return;

			Vector3 headPosition;
			Vector3 headForward;
			if (!CalcHead(m_dummy, m_headBoneNo, m_headForwardLocal, headPosition, headForward)) return;

			// 部屋の方 = 振り向いた人形の正面(水平)
			Vector3 roomFront(headForward.x, 0.0f, headForward.z);
			if (roomFront.LengthSq() < MIN_LENGTH_SQ) roomFront = Vector3::Front;
			roomFront.Normalize();

			// 壁は、人形の背中側(盗んだ壁)に、部屋の方を向けて立てる
			// ※ 顔が写る角度のカメラは人形の前(部屋の側)にいるので、壁がカメラをふさぐことはない
			Vector3 wallOrigin = DUMMY_POSITION - roomFront * m_wallDistance;

			// 盗んだ絵画は、両手の真ん中に持たせる。手が取れなければ、人形の腰の高さの前くらい
			Vector3 handCenter;
			if (!CalcHandCenter(handCenter))
			{
				handCenter = DUMMY_POSITION + Vector3(0.0f, DEFAULT_HEIGHT * HALF, 0.0f);
			}

			for (SetPiece& piece : m_setPieceList)
			{
				PlacePiece(piece, piece.isHeld ? handCenter : wallOrigin, roomFront);
			}
			PlacePeople(wallOrigin, roomFront);
		}


		void PhotoSampleRenderer::PlacePiece(SetPiece& piece, const Vector3& origin, const Vector3& frontZ)
		{
			Vector3 axisX;
			axisX.Cross(Vector3::Up, frontZ);
			axisX.Normalize();

			// 行に、基準の X・Y・Z の向きを並べた回転の行列(基準の中の向き → ワールドの向き)
			Matrix baseRotation = Matrix::Identity;
			baseRotation.m[0][0] = axisX.x;   baseRotation.m[0][1] = axisX.y;   baseRotation.m[0][2] = axisX.z;
			baseRotation.m[1][0] = 0.0f;      baseRotation.m[1][1] = 1.0f;      baseRotation.m[1][2] = 0.0f;
			baseRotation.m[2][0] = frontZ.x;  baseRotation.m[2][1] = frontZ.y;  baseRotation.m[2][2] = frontZ.z;

			// 位置: 基準の中の位置を回してから、基準の位置に足す
			Vector3 position = piece.position;
			baseRotation.Apply(position);
			position += origin;

			// 回転: 置物の回転 → 基準の回転、の順にかける(行ベクトルなので左から順に効く)
			Matrix pieceRotation;
			pieceRotation.MakeRotationFromQuaternion(piece.rotation);
			const Matrix worldRotation = pieceRotation * baseRotation;
			Quaternion rotation;
			rotation.SetRotation(worldRotation);

			piece.render->SetTRS(position, rotation, piece.scale);
			piece.render->Update();
		}


		void PhotoSampleRenderer::PlacePeople(const Vector3& wallOrigin, const Vector3& wallFront)
		{
			if (m_setPersonList.empty()) return;

			// カメラの前・右(水平)
			const Vector3 cameraPosition = m_camera.GetPosition();
			Vector3 cameraForward = m_camera.GetForward();
			cameraForward.y = 0.0f;
			if (cameraForward.LengthSq() < MIN_LENGTH_SQ) return;
			cameraForward.Normalize();
			Vector3 cameraRight = m_camera.GetRight();
			cameraRight.y = 0.0f;
			cameraRight.Normalize();

			// カメラから人形までの奥行き(客の奥行きは、これに対する割合で決める → 見本の大きさを変えても同じ写り方)
			const float dummyDepth = (DUMMY_POSITION - cameraPosition).Dot(cameraForward);
			const float tanHalf = std::tan(Math::DegToRad(SAMPLE_VIEW_ANGLE_DEG) * HALF);

			for (SetPerson& person : m_setPersonList)
			{
				// 画面の横の位置 → その奥行きでの左右の位置(画面の横幅の半分 = 奥行き × tan(画角/2) × 横縦比)
				const float depthRate = person.depthRate > PERSON_MIN_DEPTH_RATE ? person.depthRate : PERSON_MIN_DEPTH_RATE;
				const float depth = dummyDepth * depthRate;
				const float halfWidth = depth * tanHalf * m_camera.GetAspect();
				Vector3 position = cameraPosition + cameraForward * depth + cameraRight * ((person.screenX - HALF) * 2.0f * halfWidth);
				position.y = DUMMY_POSITION.y;

				// 壁にめり込むなら、壁から離す
				const float wallDistance = (position - wallOrigin).Dot(wallFront);
				if (wallDistance < PERSON_WALL_MIN_DISTANCE)
				{
					position += wallFront * (PERSON_WALL_MIN_DISTANCE - wallDistance);
				}

				// 向き: 壁(絵画)の方を向いて、turnDeg だけ回す
				Quaternion rotation;
				rotation.SetRotationY(std::atan2(-wallFront.x, -wallFront.z) + Math::DegToRad(person.turnDeg));

				person.render->SetTRS(position, rotation, person.scale);
				person.render->Update();
			}
		}


		bool PhotoSampleRenderer::CalcHandCenter(Vector3& outPosition) const
		{
			Vector3 sum = Vector3::Zero;
			int count = 0;
			const int handBoneList[] = { m_leftHandBoneNo, m_rightHandBoneNo };
			for (const int boneNo : handBoneList)
			{
				if (boneNo < 0) continue;
				const Matrix& hand = m_dummy.GetBone(boneNo)->GetWorldMatrix();
				sum += Vector3(hand.m[3][0], hand.m[3][1], hand.m[3][2]);
				++count;
			}
			if (count == 0) return false;

			outPosition = sum * (1.0f / static_cast<float>(count));
			return true;
		}
	}
}
