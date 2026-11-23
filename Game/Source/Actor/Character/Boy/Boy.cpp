/**
 * @file Boy.cpp
 * @brief 主人公(男の子)
 */
#include "stdafx.h"
#include "Boy.h"
#include "BoyStateMachine.h"
#include "BoyState.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 主人公の 1 歩の長さ(足音を鳴らす間かく) */
			constexpr float BOY_FOOTSTEP_STRIDE = 80.0f;

			// 主人公のアニメーションデータ
			const AnimationData BOY_ANIMATION_DATA[] =
			{
					{"Assets/animData/Boy/Idle.tka",             true} // ファインダーを覗いていない時の待機
				,	{"Assets/animData/Boy/Walk.tka" ,            true} // ファインダーを覗いていない時の歩行
				,	{"Assets/animData/Boy/Run.tka",              true} // ファインダーを覗いていない時の走行
				,	{"Assets/animData/Boy/PhotoIdle.tka",        true} // ファインダーを覗いている時の待機
				,	{"Assets/animData/Boy/PhotoWalkForward.tka", true} // ファインダーを覗いている時の前歩き
				,	{"Assets/animData/Boy/PhotoWalkBack.tka",    true} // ファインダーを覗いている時の後ろ歩き
				,	{"Assets/animData/Boy/PhotoWalkLeft.tka",    true} // ファインダーを覗いている時の左歩き
				,	{"Assets/animData/Boy/PhotoWalkRight.tka",   true} // ファインダーを覗いている時の右歩き
				,	{"Assets/animData/Boy/LowHide.tka",          true} // 小さい物陰に隠れる
				,	{"Assets/animData/Boy/HightHide.tka",        true} // 高い物陰に隠れる
			};

			// 少年のアニメーションデータの数と、EnBoyAnimation の数があっているかをコンパイル時にチェックする
			static_assert(sizeof(BOY_ANIMATION_DATA) / sizeof(BOY_ANIMATION_DATA[0]) == static_cast<size_t>(EnBoyAnimation::Max),
				"BOY_ANIMATION_DATA と EnBoyAnimation の数が合っていません。");


			/** モデルデータ(主人公のモデルができるまでは、ユニティちゃんを仮で使う) */
			const ModelData BOY_MODEL_DATA =
			{
				"Assets/modelData/Boy/Boy.tkm",
				BOY_ANIMATION_DATA,
				enModelUpAxisZ,
				static_cast<uint8_t>(EnBoyAnimation::Max)
			};

			/** 手に持つカメラのモデル */
			constexpr const char* HAND_CAMERA_MODEL_PATH = "Assets/modelData/Camera/Camera.tkm";
			/**
			 * 手に持つカメラのモデルの、底から真ん中までの高さ(モデルのままの大きさ)
			 * ※ モデルの底が基準の点になっているので、この分だけ下げて、カメラの真ん中を手のひらに合わせる
			 */
			constexpr float HAND_CAMERA_MODEL_HALF_HEIGHT = 90.0f;

			/**
			 * @brief ボーンの名前からボーンの番号を探す
			 * @param modelRender モデル
			 * @param boneName ボーンの名前
			 * @return ボーンの番号(名前が空・見つからなければ -1)
			 */
			int FindBoneNo(const ModelRender& modelRender, const std::string& boneName)
			{
				if (boneName.empty()) return -1;
				const std::wstring name(boneName.begin(), boneName.end());
				return modelRender.FindBoneID(name.c_str());
			}

			/**
			 * @brief ボーンのワールドの位置を取る
			 */
			Vector3 GetBonePosition(const ModelRender& modelRender, const int boneNo)
			{
				const Matrix& matrix = modelRender.GetBone(boneNo)->GetWorldMatrix();
				return Vector3(matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]);
			}

			/** 調整用パラメーターの JSON */
			constexpr const char* BOY_PARAMETER_PATH = "Assets/parameter/Actor/Character/Boy/Boy.json";
		}


		Boy::Boy()
			: m_gameCamera(nullptr)
			, m_isFinderActive(false)
			, m_noticeScale(1.0f)
			, m_handBoneNo(-1)
			, m_palmBoneNo(-1)
		{}


		Boy::~Boy()
		{}


		bool Boy::Start()
		{
			// 調整用パラメーターの読み込み
			m_parameter.Initialize(BOY_PARAMETER_PATH);

			// 調整パラメーターを取得
			const BoyParameter& parameter = m_parameter.Get();
			// 初期位置だけ設定
			SetPosition(parameter.initialPosition);

			// モデルとアニメーションの読み込み
			Init(BOY_MODEL_DATA);

			// 手に持つカメラ(ボーンは Update で探す)
			m_handCamera.Init(HAND_CAMERA_MODEL_PATH);

			// 行動を決めるステートマシンを作る
			m_characterStateMachine = std::make_unique<BoyStateMachine>(this);

			// キャラクターコントローラーの初期化(今の位置で作るので、位置を決めてから Start を呼ぶ)
			return CharacterBase::Start();
		}


		void Boy::Update()
		{
			// 調整パラメーターを取得
			// ※ Get() を呼ぶと、JSONが変わっていれば読み直す
			const BoyParameter& parameter = m_parameter.Get();

			SetScale(parameter.updateScale);

			// 重力
			// ※ ステートの Move / StopMove は横(XZ)しか変えないので、縦(y)はここで決める
			// ※ 地面にいる時は落ちる速さを 0 に戻す。戻さないと、たまり続けて、段差から落ちた瞬間に一気に落ちる
			Vector3 moveSpeed = GetMoveSpeed();
			if (IsOnGround()) moveSpeed.y = 0.0f;
			
			moveSpeed.y -= parameter.gravity * g_gameTime->GetFrameDeltaTime();
			// 移動速度を設定
			SetMoveSpeed(moveSpeed);

			// ステートマシン → キャラクターコントローラー(床に当たったら止まる)→ モデルの順に更新する
			CharacterBase::Update();
			
			// 走っていると目立つ(悪党が遠くから・早く気付く)
			m_noticeScale = IsState(BoyRunState::ID()) ? parameter.runNoticeScale : 1.0f;

			// 手に持つカメラを、手に合わせる(モデルを動かした後に呼ぶ)
			UpdateHandCamera(parameter);
		}


		void Boy::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);

			// 覗いている時は、目の前にカメラが来て画面をふさぐので描かない
			if (!m_isFinderActive && m_handBoneNo >= 0)
			{
				m_handCamera.Draw(rc);
			}
		}


		void Boy::UpdateHandCamera(const BoyParameter& parameter)
		{
			// ボーンの名前が変わった時(最初も)だけ、探し直す
			if (m_handBoneName != parameter.cameraHandBone)
			{
				m_handBoneName = parameter.cameraHandBone;
				m_handBoneNo = FindBoneNo(m_modelRender, m_handBoneName);
			}
			if (m_palmBoneName != parameter.cameraPalmBone)
			{
				m_palmBoneName = parameter.cameraPalmBone;
				m_palmBoneNo = FindBoneNo(m_modelRender, m_palmBoneName);
			}
			if (m_handBoneNo < 0) return;

			// 場所は手のひらの真ん中(手首と中指の付け根の間)。先の方のボーンが無ければ手首
			Vector3 handPosition = GetBonePosition(m_modelRender, m_handBoneNo);
			if (m_palmBoneNo >= 0)
			{
				handPosition = (handPosition + GetBonePosition(m_modelRender, m_palmBoneNo)) * 0.5f;
			}

			// 向きは主人公の向きに合わせる(レンズが主人公の前を向いて、カメラがまっすぐ立つ)
			// ※ 手のボーンの向きに合わせると、ボーンの軸の向きがモデルごとにちがうので、カメラが変な向きになる
			const Quaternion& bodyRotation = m_transform.GetWorldRotation();
			Vector3 right = Vector3::AxisX;
			Vector3 up = Vector3::AxisY;
			Vector3 forward = Vector3::AxisZ;
			bodyRotation.Apply(right);
			bodyRotation.Apply(up);
			bodyRotation.Apply(forward);

			// 位置: 手の位置から、主人公の向きで見たずれの分だけ動かす
			const Vector3 position = handPosition
				+ right * parameter.cameraOffset.x
				+ up * parameter.cameraOffset.y
				+ forward * parameter.cameraOffset.z;

			// 向き: 主人公の向きに、JSON の回転(x → y → z)を足す
			Quaternion localRotation = Quaternion::Identity;
			Quaternion addRotation;
			addRotation.SetRotationDegX(parameter.cameraRotation.x);
			localRotation.Multiply(addRotation);
			addRotation.SetRotationDegY(parameter.cameraRotation.y);
			localRotation.Multiply(addRotation);
			addRotation.SetRotationDegZ(parameter.cameraRotation.z);
			localRotation.Multiply(addRotation);
			Quaternion rotation = localRotation;
			rotation.Multiply(bodyRotation);

			// カメラのモデルは底が基準なので、真ん中が手のひらに来るように、カメラの下の方へ下げる
			const float scale = parameter.cameraScale;
			Vector3 cameraUp = Vector3::AxisY;
			rotation.Apply(cameraUp);
			const Vector3 cameraPosition = position - cameraUp * (HAND_CAMERA_MODEL_HALF_HEIGHT * scale);

			m_handCamera.SetTRS(cameraPosition, rotation, Vector3(scale, scale, scale));
			m_handCamera.Update();
		}


		bool Boy::IsState(const uint32_t stateId) const
		{
			if (m_characterStateMachine == nullptr) return false;
			return m_characterStateMachine->IsEqualState(stateId);
		}


		float Boy::GetFootstepStride() const
		{
			return BOY_FOOTSTEP_STRIDE;
		}
	}
}
