/**
 * @file Boy.cpp
 * @brief 主人公(男の子)
 */
#include "stdafx.h"
#include "Boy.h"
#include "BoyStateMachine.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			// 主人公のアニメーションデータ
			const AnimationData BOY_ANIMATION_DATA[] =
			{
					{"Assets/animData/Boy/Idle.tka",      true} // ファインダーを覗いていない時の待機
				,	{"Assets/animData/Boy/Walk.tka" ,     true} // ファインダーを覗いていない時の歩行
				,	{"Assets/animData/Boy/Run.tka"  ,     true} // ファインダーを覗いていない時の走行
				,	{"Assets/animData/Boy/PhotoIdle.tka", true} // ファインダーを覗いている時の待機
				,	{"Assets/animData/Boy/PhotoWalk.tka", true} // ファインダーを覗いている時の歩行
				,	{"Assets/animData/Boy/LowHide.tka",   true} // 小さい物陰に隠れる
				,	{"Assets/animData/Boy/HighHide.tka",  true} // 高い物陰に隠れる
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

			/** 調整用パラメーターの JSON */
			constexpr const char* BOY_PARAMETER_PATH = "Assets/parameter/Actor/Boy/Boy.json";
		}


		Boy::Boy()
			: m_gameCamera(nullptr)
			, m_isFinderActive(false)
		{}


		Boy::~Boy()
		{}


		bool Boy::Start()
		{
			// 調整用パラメーターの読み込み
			m_parameter.Initialize(BOY_PARAMETER_PATH);

			// Boyのパラメーターを取得してきて位置とスケールを設定(Startのため1回だけ読み込まれる)
			const BoyParameter& parameter = m_parameter.Get();
			
			SetPosition(parameter.initialPosition);
			SetScale(parameter.initialScale);

			// モデルとアニメーションの読み込み
			Init(BOY_MODEL_DATA);

			// 行動を決めるステートマシンを作る
			m_characterStateMachine = std::make_unique<BoyStateMachine>(this);

			// キャラクターコントローラーの初期化(今の位置で作るので、位置を決めてから Start を呼ぶ)
			return CharacterBase::Start();
		}


		bool Boy::IsState(const uint32_t stateId) const
		{
			if (m_characterStateMachine == nullptr) return false;
			return m_characterStateMachine->IsEqualState(stateId);
		}
	}
}
