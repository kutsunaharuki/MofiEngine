/**
 * @file EnemyBase.cpp
 * @brief 敵の親となるクラス
 */
#include "stdafx.h"
#include "EnemyBase.h"


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
			};

			// BIG_MAN_ANIMATION_DATA の数と、EnemyAnimation の数があっているかをコンパイル時にチェックする
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

			/** 敵の種類ごとのモデルデータのリスト */
			const ModelData* ENEMY_MODEL_DATA_LIST[] = { &BIG_MAN_MODEL_DATA,  };
			/** 大男のパラメーターのJSONファイルパス */
			constexpr const char* BIG_MAN_PARAMETER_JSON = { "Assets/parameter/Actor/Character/Enemy/BigMan/BigMan.json" };
			/** ねずみ男のパラメーターのJSONファイルパス */
			constexpr const char* RAT_MAN_PARAMETER_JSON = { "Assets/parameter/Actor/Character/Enemy/RatMan/RatMan.json" };

			/** 敵の種類ごとのパラメーターのJSONリスト */
			const char* PARAMETER_JSON_LIST[] = { BIG_MAN_PARAMETER_JSON, RAT_MAN_PARAMETER_JSON };





			/*******************************************************************************************************/


			/** 敵のアニメーションクリップ数(インデックスで使用) */
			const int ENEMY_ANIMATION_CLIP_NUM = static_cast<int>(EnemyBase::EnemyAnimation::Max);
			/** ピントを合わせる高さ(足元から。体の中心 ~ 顔のあたり) */
			constexpr float ENEMY_FOCUS_POSITION_Y = 80.0f;
		}


		EnemyBase::EnemyBase(EnemyType type)
			: m_enemyType(type)
		{}



		EnemyBase::~EnemyBase()
		{}


		bool EnemyBase::Start()
		{
			// 大男を初期化
			Init(BIG_MAN_MODEL_DATA);

			return CharacterBase::Start();
		}

		void EnemyBase::Update()
		{
			CharacterBase::Update();
		}


		Vector3 EnemyBase::GetFocusPosition() const
		{
			// ワールド座標を取得して、Y座標に ENEMY_FOCUS_POSITION_Y を足す
			const Vector3& worldPos = m_transform.GetWorldPosition();

			// ピントを合わせる位置を返す
			return Vector3(worldPos.x,worldPos.y + ENEMY_FOCUS_POSITION_Y,worldPos.z);
		}
	}
}