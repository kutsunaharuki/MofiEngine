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
			/** 壁を、人形の真正面からカメラの方へ回す角度(JSON に無い時。45 度以下だとカメラが壁の裏に入りやすい) */
			constexpr float DEFAULT_WALL_TURN_DEG = 65.0f;
			/** 伸ばした手から、壁までの距離(JSON に無い時。額の厚みくらい) */
			constexpr float DEFAULT_WALL_GAP = 6.0f;
			/** 手のボーンが取れない時に、手を伸ばしている距離 */
			constexpr float DEFAULT_REACH = 50.0f;
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
			, m_wallTurnDeg(DEFAULT_WALL_TURN_DEG)
			, m_wallGap(DEFAULT_WALL_GAP)
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

			// 手のボーン(伸ばしている方の手の先に絵画を掛ける。悪党の JSON の leftHandBone / rightHandBone)
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

			// 3. セットを置く: 伸ばした手の先の壁に絵画を掛ける(壁はカメラの方へ斜めに回す)
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
			m_wallTurnDeg = util::JsonConverter::ToFloat(json, "wallTurnDeg", DEFAULT_WALL_TURN_DEG);
			m_wallGap = util::JsonConverter::ToFloat(json, "wallGap", DEFAULT_WALL_GAP);

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
				// "atHand":true は、伸ばした手の所に置く物(盗まれている絵画)
				piece.isAtHand = util::JsonConverter::ToBool(item, "atHand", false);

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

			// ① 伸ばしている手。取れなければ、顔の前の腰の高さに手があることにする
			Vector3 reach;
			if (!CalcReachHand(reach))
			{
				Vector3 forwardFlat(headForward.x, 0.0f, headForward.z);
				if (forwardFlat.LengthSq() < MIN_LENGTH_SQ) forwardFlat = Vector3::Front;
				forwardFlat.Normalize();
				reach = DUMMY_POSITION + forwardFlat * DEFAULT_REACH;
				reach.y = DUMMY_POSITION.y + DEFAULT_HEIGHT * HALF;
			}

			// ② 体の前 = 足元 → 伸ばした手(水平)。体の右 = 上 × 前
			Vector3 bodyForward = reach - DUMMY_POSITION;
			bodyForward.y = 0.0f;
			if (bodyForward.LengthSq() < MIN_LENGTH_SQ) bodyForward = Vector3::Front;
			bodyForward.Normalize();
			Vector3 bodyRight;
			bodyRight.Cross(Vector3::Up, bodyForward);
			bodyRight.Normalize();

			// ③ カメラが体の右・左のどちらにいるか
			Vector3 toCamera = m_camera.GetPosition() - DUMMY_POSITION;
			toCamera.y = 0.0f;
			const float cameraSide = toCamera.Dot(bodyRight) >= 0.0f ? 1.0f : -1.0f;

			// ④ 壁の正面(部屋の方) = 人形の方を向いた正面を、カメラの方へ wallTurnDeg だけ回す
			// ※ 真正面(0 度)だと、顔が写る角度のカメラは壁の裏に入る。回すほど、カメラから絵画が正面に近く見える
			const float turn = Math::DegToRad(m_wallTurnDeg);
			Vector3 wallFront = bodyForward * -std::cos(turn) + bodyRight * (cameraSide * std::sin(turn));
			wallFront.Normalize();

			// ⑤ 基準: 床・壁は、伸ばした手の前の壁の根元。手の所の物(絵画)は、壁の手の高さ
			Vector3 handOnWall = reach - wallFront * m_wallGap;
			Vector3 wallOrigin = handOnWall;
			wallOrigin.y = DUMMY_POSITION.y;

			// ⑥ JSON の +X がカメラの方になるように、X の符号を決める(壁ぞいの向きは、壁の正面の向きで変わる)
			Vector3 wallSide;
			wallSide.Cross(Vector3::Up, wallFront);
			const float signX = wallSide.Dot(toCamera) >= 0.0f ? 1.0f : -1.0f;

			for (SetPiece& piece : m_setPieceList)
			{
				PlacePiece(piece, piece.isAtHand ? handOnWall : wallOrigin, wallFront, signX);
			}
			PlacePeople(wallOrigin, wallFront);

			// ⑦ 壁がカメラ・顔をふさいでいないか(ふさいでいたら知らせるだけ。JSON を直して保存すれば描き直される)
			if ((m_camera.GetPosition() - wallOrigin).Dot(wallFront) <= 0.0f)
			{
				OutputDebugStringA("[PhotoSample] camera is behind the wall. Increase wallTurnDeg.\n");
			}
			if ((headPosition - wallOrigin).Dot(wallFront) <= 0.0f)
			{
				OutputDebugStringA("[PhotoSample] head is inside the wall. Flip the sign of yawDeg, or decrease wallTurnDeg / increase wallGap.\n");
			}
		}


		void PhotoSampleRenderer::PlacePiece(SetPiece& piece, const Vector3& origin, const Vector3& frontZ, const float signX)
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
			// ※ X の符号だけ変える(回転の行列の X を逆にすると裏返しになるので、位置の方で変える)
			Vector3 position = piece.position;
			position.x *= signX;
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


		bool PhotoSampleRenderer::CalcReachHand(Vector3& outPosition) const
		{
			bool isFound = false;
			float farthestSq = -1.0f;
			const int handBoneList[] = { m_leftHandBoneNo, m_rightHandBoneNo };
			for (const int boneNo : handBoneList)
			{
				if (boneNo < 0) continue;
				const Matrix& hand = m_dummy.GetBone(boneNo)->GetWorldMatrix();
				const Vector3 handPosition(hand.m[3][0], hand.m[3][1], hand.m[3][2]);

				// 足元から水平に一番離れている手 = 盗もうとして伸ばしている手
				Vector3 fromFoot = handPosition - DUMMY_POSITION;
				fromFoot.y = 0.0f;
				const float distanceSq = fromFoot.LengthSq();
				if (distanceSq <= farthestSq) continue;

				farthestSq = distanceSq;
				outPosition = handPosition;
				isFound = true;
			}
			return isFound;
		}
	}
}
