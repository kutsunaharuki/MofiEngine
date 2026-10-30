/**
 * @file EnemyBase.cpp
 * @brief 敵の親となるクラス
 */
#include "stdafx.h"
#include "EnemyBase.h"
#include "EnemyStateMachine.h"
#include "EnemyDestinationMode.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			// 大男のアニメーションデータ
			const AnimationData BIG_MAN_ANIMATION_DATA[] =
			{
					{"Assets/animData/Enemy/BigMan/Idle.tka",  true} // 待機
				,	{"Assets/animData/Enemy/BigMan/Walk.tka",  true} // 歩行
				,	{"Assets/animData/Enemy/BigMan/Alert.tka", true} // 警戒
				,   {"Assets/animData/Enemy/BigMan/Steal.tka", true} // 盗む
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

			// 敵のステートマシンを作成
			m_characterStateMachine = std::make_unique<EnemyStateMachine>(this);
			// 預かっていた経路探索を作ったステートマシンに渡す
			GetEnemyStateMachine()->SetNavigation(m_pathfinder, m_navGrid);

			// 目的地モードを作って、預かっていた美術品の一覧を渡す
			m_destinationMode = std::make_unique<EnemyDestinationMode>(GetEnemyStateMachine());
			m_destinationMode->SetTargetList(m_targetList);

			return CharacterBase::Start();
		}

		void EnemyBase::Update()
		{
			// ピントの高さを覚えておく
			// ※ GetFocusPosition は const なので、Get()(JSON が変わっていれば読み直す)を呼べない。ここで読んでおく
			m_focusHeight = m_enemyParameter.Get().focusHeight;

			// 行き先を決める(頭) → 道順をたどる(運転手) → ステートを選んで動く、の順番
			m_destinationMode->Update(g_gameTime->GetFrameDeltaTime());

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


		Vector3 EnemyBase::GetFocusPosition() const
		{
			// ワールド座標を取得して、Y座標にピントの高さを足す
			const Vector3& worldPos = m_transform.GetWorldPosition();

			// ピントを合わせる位置を返す
			return Vector3(worldPos.x, worldPos.y + m_focusHeight, worldPos.z);
		}
	}
}