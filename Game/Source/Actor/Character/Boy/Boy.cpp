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
			// ステートがちゃんと切り替わるかの確認材料
			/** アニメーションデータ(EnBoyAnimation の順番と合わせる) */
			const AnimationData BOY_ANIMATION_DATA[] =
			{
				{ "Assets/animData/idle.tka", true },	// 待機
				{ "Assets/animData/walk.tka", true },	// 歩き
				{ "Assets/animData/run.tka", true },	// 走り
				{ "Assets/animData/jump.tka", false },	// ジャンプ
			};
			// 少年のアニメーションデータの数と、EnBoyAnimation の数があっているかをコンパイル時にチェックする
			static_assert(sizeof(BOY_ANIMATION_DATA) / sizeof(BOY_ANIMATION_DATA[0]) == static_cast<size_t>(EnBoyAnimation::Num),
				"BOY_ANIMATION_DATA と EnBoyAnimation の数が合っていません。");


			/** モデルデータ(主人公のモデルができるまでは、ユニティちゃんを仮で使う) */
			const ModelData BOY_MODEL_DATA =
			{
				"Assets/modelData/unityChan.tkm",
				BOY_ANIMATION_DATA,
				enModelUpAxisY,
				static_cast<uint8_t>(EnBoyAnimation::Num)
			};

			/** 調整用パラメーターの JSON */
			constexpr const char* BOY_PARAMETER_PATH = "Assets/parameter/Actor/Boy.json";
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
