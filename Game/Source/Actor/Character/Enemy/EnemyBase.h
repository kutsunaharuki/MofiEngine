/**
 * @file EnemyBase.h
 * @brief 敵の親となるクラス
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "Source/Camera/PhotoSubject.h"
#include "Source/Actor/Character/CharacterBase.h"
#include <cstdint>
#include "Source/Util/ParameterManager.h"
#include "Object/EnemyParameter.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class EnemyStateMachine;


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
				Steal,     // 盗む
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

			/**
			 * @brief 敵のステートマシンを取得
			 * @return 敵のステートマシン
			 * @detalis 敵のステートマシンは CharacterStateMachine が所持しているので変換して返す
			 */
			EnemyStateMachine* GetEnemyStateMachine();

			/**
			 * @brief 調整パラメーターを取得
			 * @return 調整パラメーター(JSON が変わっていれば、読み直してから返す)
			 * @details Get() が読み直しで中身を書き換えるので、const 関数にはできない
			 */
			const EnemyParameter& GetParameter() { return m_enemyParameter.Get(); }


		private:
			/** 敵の種類 */
			EnemyType m_enemyType;
			/** 敵のパラメーター情報 */
			util::Parameter<EnemyParameter> m_enemyParameter;
			/** ピントを合わせる高さ */
			float m_focusHeight;
		};
	}
}