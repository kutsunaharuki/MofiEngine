/**
 * @file EnemyBase.cpp
 * @brief 敵の親となるクラス
 */
#include "stdafx.h"
#include "EnemyBase.h"
#include "EnemyStateMachine.h"
#include "EnemyDestinationMode.h"
#include "EnemyAlertMode.h"
#include "Source/Mission/ShotMeasure.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 悪党の 1 歩の長さ(足音を鳴らす間かく) */
			constexpr float ENEMY_FOOTSTEP_STRIDE = 90.0f;
			/** 主人公とこの長さより近い時だけ、悪党の足音を鳴らす */
			constexpr float ENEMY_FOOTSTEP_HEAR_DISTANCE = 1200.0f;

			// 大男のアニメーションデータ
			const AnimationData BIG_MAN_ANIMATION_DATA[] =
			{
					{"Assets/animData/Enemy/BigMan/Idle.tka",  true} // 待機
				,	{"Assets/animData/Enemy/BigMan/Walk.tka",  true} // 歩行
				,	{"Assets/animData/Enemy/BigMan/Alert.tka", true} // 警戒
				,   {"Assets/animData/Enemy/BigMan/Steal.tka", true} // 盗む
				,   {"Assets/animData/Enemy/BigMan/Point.tka", false} // 指さす(1 回だけ。最後の形で止まる)
			};

			// BIG_MAN_ANIMATION_DATA の数と、EnemyAnimation の数があっているかをコンパイル時にチェック
			static_assert(sizeof(BIG_MAN_ANIMATION_DATA) / sizeof(BIG_MAN_ANIMATION_DATA[0]) == static_cast<size_t>(EnemyBase::EnemyAnimation::Max),
				"BIG_MAN_ANIMATION_DATA と EnemyAnimation の数が合っていません。");

			// 大男のモデルデータ
			const ModelData BIG_MAN_MODEL_DATA =
			{
				"Assets/modelData/Enemy/BigMan/BigMan.tkm",
				BIG_MAN_ANIMATION_DATA,
				enModelUpAxisZ,
				static_cast<uint8_t>(EnemyBase::EnemyAnimation::Max)
			};


			/** ねずみ男のアニメーションデータ */
			const AnimationData RAT_MAN_ANIMATION_DATA[] =
			{
					{"Assets/animData/Enemy/RatMan/Idle.tka",  true} // 待機
				,	{"Assets/animData/Enemy/RatMan/Walk.tka",  true} // 歩行
				,	{"Assets/animData/Enemy/RatMan/Alert.tka", true} // 警戒
				,	{"Assets/animData/Enemy/RatMan/Steal.tka", true} // 盗む
				,	{"Assets/animData/Enemy/RatMan/Point.tka", false} // 指さす(1 回だけ。最後の形で止まる)
			};

			// RAT_MAN_ANIMATION_DATA の数と、EnemyAnimation の数があっているかをコンパイル時にチェック
			static_assert(sizeof(RAT_MAN_ANIMATION_DATA) / sizeof(RAT_MAN_ANIMATION_DATA[0]) == static_cast<size_t>(EnemyBase::EnemyAnimation::Max),
				"RAT_MAN_ANIMATION_DATA と EnemyAnimation の数が合っていません。");

			/** ねずみ男のモデルデータ */
			const ModelData RAT_MAN_MODEL_DATA =
			{
				"Assets/modelData/Enemy/RatMan/RatMan.tkm",
				RAT_MAN_ANIMATION_DATA,
				enModelUpAxisZ,
				static_cast<uint8_t>(EnemyBase::EnemyAnimation::Max)
			};


			/** 敵の種類ごとのモデルデータのリスト */
			const ModelData* ENEMY_MODEL_DATA_LIST[] = { &BIG_MAN_MODEL_DATA, &RAT_MAN_MODEL_DATA};

			// ENEMY_MODEL_DATA_LIST の数と、EnemyType の数があっているかをコンパイル時にチェック
			static_assert(sizeof(ENEMY_MODEL_DATA_LIST) / sizeof(ENEMY_MODEL_DATA_LIST[0]) == static_cast<size_t>(EnemyBase::EnemyType::Max),
				"ENEMY_MODEL_DATA_LIST と EnemyType の数が合っていません。");


			/** 大男のパラメーターのJSONファイルパス */
			constexpr const char* BIG_MAN_PARAMETER_JSON = "Assets/parameter/Actor/Character/Enemy/BigMan/BigManParameter.json";
			/** ねずみ男のパラメーターのJSONファイルパス */
			constexpr const char* RAT_MAN_PARAMETER_JSON = "Assets/parameter/Actor/Character/Enemy/RatMan/RatManParameter.json";

			/** 敵の種類ごとのパラメーターのJSONリスト(項目は同じ EnemyParameter で、値だけ種類ごとに違う) */
			const char* PARAMETER_JSON_LIST[] = { BIG_MAN_PARAMETER_JSON, RAT_MAN_PARAMETER_JSON };


			// PARAMETER_JSON_LIST の数と、EnemyType の数が合っているかをコンパイル時にチェック
			static_assert(sizeof(PARAMETER_JSON_LIST) / sizeof(PARAMETER_JSON_LIST[0]) == static_cast<size_t>(EnemyBase::EnemyType::Max),
				"PARAMETER_JSON_LIST と EnemyType の数が合っていません。");
		}


		EnemyBase::EnemyBase(EnemyType type)
			: m_enemyType(type)
			, m_focusHeight(0.0f)
			, m_pathfinder(nullptr)
			, m_navGrid(nullptr)
			, m_player(nullptr)
			, m_headBoneNo(-1)
			, m_isFound(false)
		{}



		EnemyBase::~EnemyBase()
		{}


		bool EnemyBase::Start()
		{
			// 敵の種類をリスト番号に変換
			const int type = static_cast<int>(m_enemyType);

			// 敵のパラメーターを読み込む(種類ごとの JSON)
			m_enemyParameter.Initialize(PARAMETER_JSON_LIST[type]);
			// Get() を呼ぶと、JSONが変わっていれば読み直す(ホットリロード)
			const EnemyParameter& parameter = m_enemyParameter.Get();

			/** 初期位置と初期スケールを設定 */
			SetPosition(parameter.initialPosition);
			SetScale(parameter.initialScale);

			// モデルとアニメーションを読み込む
			Init(*ENEMY_MODEL_DATA_LIST[type]);


			// 頭のボーンの番号を探す(1回だけ見る)
			const std::wstring headBoneName(parameter.headBone.begin(), parameter.headBone.end());
			m_headBoneNo = m_modelRender.FindBoneID(headBoneName.c_str());


			// 敵のステートマシンを作成
			m_characterStateMachine = std::make_unique<EnemyStateMachine>(this);
			// 預かっていた経路探索を作ったステートマシンに渡す
			GetEnemyStateMachine()->SetNavigation(m_pathfinder, m_navGrid);

			// 目的地モードを作って、預かっていた美術品の一覧を渡す
			m_destinationMode = std::make_unique<EnemyDestinationMode>(GetEnemyStateMachine());
			m_destinationMode->SetTargetList(m_targetList);

			// 警戒モードを作って、預かっていた隠れ場所の一覧を渡す
			m_alertMode = std::make_unique<EnemyAlertMode>(GetEnemyStateMachine());
			m_alertMode->SetHidePointList(m_hidePointList);

			return CharacterBase::Start();
		}
		

		void EnemyBase::Update()
		{
			const EnemyParameter& parameter = m_enemyParameter.Get();

			// ピントの高さを覚えておく
			// ※ GetFocusPosition は const なので、Get()(JSON が変わっていれば読み直す)を呼べない。ここで読んでおく
			m_focusHeight = parameter.focusHeight;
			
			// 世界の時間で進める(スローモーションで遅くなる)
			const float deltaTime = util::WorldTime::GetDeltaTime();


			// 主人公を見つけた後は、目的地・警戒・道順は使わず、見つけたステートが主人公の方へ走らせる
			if (m_isFound)
			{
				if (m_player) GetEnemyStateMachine()->SetFoundTarget(m_player->GetTransform().GetWorldPosition());
				CharacterBase::Update();
				return;
			}

			// SetSetting は値をコピーしているだけなので目的地モードをやり直したりはしない
			m_destinationMode->SetSetting(parameter.destinationMode);
			// SetSetting は値をコピーしているだけなので警戒モードをやり直さない
			m_alertMode->SetSetting(parameter.alertMode);
			
			// 主人公が見えたら警戒モードに入る
			// 警戒モードの更新よりも先に呼ぶ
			UpdateVision(parameter.vision, deltaTime);
			
			
			// 行き先を決める(頭) → 道順をたどる(運転手) → ステートを選んで動く、の順番
			// 警戒モードが先。警戒が終わった後に目的地モードがすぐ続きを始められるため
			m_alertMode->Update(deltaTime);
			m_destinationMode->Update(deltaTime);

			// ステートマシンが Walk / Idle を選ぶ前に道順から移動方向と移動量を決めておく
			// ※ 順番を逆にすると、待機中は移動量が 0 のまま → 歩行に切り替わらない
			GetEnemyStateMachine()->UpdatePathFollow();

			CharacterBase::Update();
		}


		EnemyStateMachine* EnemyBase::GetEnemyStateMachine()
		{
			// Start() で m_characterStateMachine に EnemyStateMachine を作っているのでそのままキャストして返す
			return static_cast<EnemyStateMachine*>(m_characterStateMachine.get());
		}


		void EnemyBase::SetNavigation(const ai::Pathfinder* pathfinder, const ai::NavGrid* navGrid)
		{
			// ステートマシンができるまで預かっておく
			m_pathfinder = pathfinder;
			m_navGrid = navGrid;


			// すでにステートマシンができてればすぐ渡す
			if (m_characterStateMachine)
			{
				GetEnemyStateMachine()->SetNavigation(m_pathfinder, m_navGrid);
			}
		}


		void EnemyBase::SetTargetList(const std::vector<Vector3>& targetList)
		{
			// 目的地モードができるまで預かっておく
			m_targetList = targetList;

			// すでに目的地モードがあれば(Start の後に呼ばれた時)、すぐ渡す
			if (m_destinationMode)
			{
				m_destinationMode->SetTargetList(targetList);
			}
		}


		void EnemyBase::SetHidePointList(const std::vector<stage::HidePoint>& hidePointList)
		{
			// 警戒モードが出来るまで保持
			m_hidePointList = hidePointList;

			// すでに警戒モードがあれば(Start の後に呼ばれた時)、すぐ渡す
			if (m_alertMode)
			{
				m_alertMode->SetHidePointList(hidePointList);
			}
		}


		void EnemyBase::StartAlert()
		{
			if (m_alertMode == nullptr) return;

			m_alertMode->StartAlert();
		}


		void EnemyBase::EndAlert()
		{
			if (m_alertMode == nullptr) return;

			m_alertMode->EndAlert();
		}


		void EnemyBase::StartFound()
		{
			if (m_isFound || m_characterStateMachine == nullptr) return;

			m_isFound = true;
			// 警戒・盗み・道順は全部やめる(見つけたステートだけで動かす)
			EndAlert();
			EnemyStateMachine* stateMachine = GetEnemyStateMachine();
			stateMachine->SetStealing(false);
			stateMachine->StopPathFollow();
			if (m_player) stateMachine->SetFoundTarget(m_player->GetTransform().GetWorldPosition());
			stateMachine->StartFound();
		}


		bool EnemyBase::IsCaught()
		{
			if (!m_isFound || m_characterStateMachine == nullptr) return false;
			return GetEnemyStateMachine()->IsCaught();
		}


		bool EnemyBase::GetHead(Vector3& outPosition, Vector3& outForward)
		{
			return mission::CalcHead(m_modelRender, m_headBoneNo, GetParameter().headForwardLocal, outPosition, outForward);
		}


		const ModelData& EnemyBase::GetModelData(const EnemyType type)
		{
			return *ENEMY_MODEL_DATA_LIST[static_cast<int>(type)];
		}


		const char* EnemyBase::GetParameterPath(const EnemyType type)
		{
			return PARAMETER_JSON_LIST[static_cast<int>(type)];
		}


		void EnemyBase::UpdateVision(const ai::VisionSetting& setting, const float deltaTime)
		{
			// 主人公が nullptr なら処理しない
			if (m_player == nullptr) return;

			m_vision.SetSetting(setting);

			// 目の位置
			Vector3 eyePosition = m_transform.GetWorldPosition();
			eyePosition.y += setting.eyeHeight;

			// 前の向き(回転は Y 軸だけで、回転 0 の時 Z+ が正面)
			Vector3 forward = Vector3::Front;
			m_transform.GetWorldRotation().Apply(forward);

			// 主人公のどこを見るか
			Vector3 targetPosition = m_player->GetTransform().GetWorldPosition();
			// ※ 主人公がしゃがむと低くなる(低い物の陰に入ると、線がさえぎられて見えなくなる)
			targetPosition.y += m_player->GetVisionTargetHeight(setting.targetHeight);

			if (m_vision.Update(deltaTime, eyePosition, forward, targetPosition,
				GetCollisionObject(), m_player->GetCollisionObject(), m_player->GetNoticeScale()))
			{
				// 見えている時間が気づく時間を超えたら、見つけた(ゲームオーバーへ)
				// ※ 前は警戒モードに入っていた。警戒モードは、デバッグの窓からは今も入れる
				StartFound();
			}
		}


		Vector3 EnemyBase::GetFocusPosition() const
		{
			// ワールド座標を取得して、Y座標にピントの高さを足す
			const Vector3& worldPos = m_transform.GetWorldPosition();

			// ピントを合わせる位置を返す
			return Vector3(worldPos.x, worldPos.y + m_focusHeight, worldPos.z);
		}


		float EnemyBase::GetFootstepStride() const
		{
			return ENEMY_FOOTSTEP_STRIDE;
		}


		bool EnemyBase::CanPlayFootstep() const
		{
			if (m_player == nullptr) return false;

			const Vector3 diff = m_player->GetTransform().GetWorldPosition() - m_transform.GetWorldPosition();
			return diff.LengthSq() < ENEMY_FOOTSTEP_HEAR_DISTANCE * ENEMY_FOOTSTEP_HEAR_DISTANCE;
		}
	}
}
