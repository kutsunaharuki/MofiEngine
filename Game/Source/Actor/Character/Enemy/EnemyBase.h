/**
 * @file EnemyBase.h
 * @brief 敵の親となるクラス
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "Source/Camera/PhotoSubject.h"
#include "Source/Actor/Character/CharacterBase.h"
#include <cstdint>


namespace app
{
	namespace actor
	{
		/**
		 * @brief 敵の親となるクラス
		 * @details 敵の種類でモデルとパラメーターを切り替える
		 * @details 今回は多重継承でCharacterBase と IPhotoSubject を継承している(IPhotoSubjectがインターフェースなので安全)
		 */
		class EnemyBase : public CharacterBase, public camera::IPhotoSubject
		{
		public:
			// 敵の種類
			enum class EnemyType : uint8_t
			{
				BigMan,   // 大男
				RatMan,   // ねずみ男
				Max       // 最大数
			};


			// 敵のアニメーション
			enum class EnemyAnimation : uint8_t
			{
				Idle,      // 待機
				Walk,      // 歩く
				Alert,     // 警戒
				Max        // 最大数
			};


		public:
			explicit EnemyBase(EnemyType type);
			~EnemyBase() override;

			/** 初期化処理 */
			bool Start() override;
			/** 更新処理 */
			void Update() override;


		public:
			/**
			 * @brief 敵の種類を取得
			 * @return 敵の種類
			 */
			EnemyType GetEnemyType() const { return m_enemyType; }
			
			/**
			 * @brief 敵に被写体のピントを合わせる位置を取得
			 * @return 被写体のピントを合わせる位置(定義した場所で体の中心や顔のあたりの位置が変わる)
			 */
			Vector3 GetFocusPosition() const override;


		private:
			/** 敵の種類 */
			EnemyType m_enemyType;
			/** アニメーションクリップ */
			std::unique_ptr<AnimationClip> m_animationClip;
			/** 敵のアニメーションのリスト */
			std::vector<std::unique_ptr<AnimationClip[]>> m_enemyAnimationClipList;
		};
	}
}