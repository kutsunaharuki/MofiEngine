/*
 * @file CharacterBase.h
 * @brief キャラクターの基底クラス
 */
#pragma once
#include "Source/Actor/Actor.h"


namespace app
{
	namespace actor
	{
		/** アニメーションデータ */
		struct AnimationData
		{
			/** ファイルネーム */
			const char* fileName;
			/** ループするかのフラグ */
			const bool isLoop;
		};




		/****************************************************/


		/** モデルデータ */
		struct ModelData
		{
			/** ファイルネーム */
			const char* fileName;
			/** アニメーションデータ */
			const AnimationData* animationData;
			/** モデルの上方向 */
			const EnModelUpAxis upAxis;
			/** アニメーションクリップの数 */
			const uint8_t clipNum;
		};




		/***************************************************/
		
		
		/** 前方宣言 */
		class CharacterStateMachine;


		/**
		 * @brief キャラクターの基底クラス
		 */
		class CharacterBase : public Actor
		{
		public:
			CharacterBase();
			virtual ~CharacterBase() override;


		protected:
			/**
			 * @brief 初期化処理
			 * @param modelData モデルデータ
			 * @details モデルデータは共通して使うものなので用意している。
			 *          アニメーションクリップの読み込みと、モデルの初期化を行う
			 */
			void Init(const ModelData& modelData);


		public:
			/** 初期化処理 */
			virtual bool Start() override;
			/** 更新処理 */
			virtual void Update() override;
			/** 描画処理 */
			virtual void Render(RenderContext& rc) override;


		public:
			/**
			 * @brief 移動速度を設定
			 * @param speed 移動速度
			 * @details キャラクターコントローラーで動かすために移動速度を設定させる関数を用意
			 */
			void SetMoveSpeed(const Vector3& speed) { m_moveSpeed = speed; }
			/**
			 * @brief 移動速度を取得
			 * @return 移動速度
			 * @details 縦(y)の速度は、ジャンプや重力で毎フレーム積み重ねるので、取得してから変える
			 */
			const Vector3& GetMoveSpeed() const { return m_moveSpeed; }
			/**
			 * @brief キャラクターが地面についているかどうかを判定
			 * @return 地面に付いていたらtrue
			 */
			bool IsOnGround() const { return m_characterController.IsOnGround(); }
			/**
			 * @brief 指定した位置に移動させる(ワープ)
			 * @param position 移動先
			 * @details キャラクターコントローラーの位置も合わせて変えて、移動速度は 0 に戻す。
			 *          SetPosition だけだと、次の更新でキャラクターコントローラーの位置に戻されるので、こちらを使う
			 */
			void Warp(const Vector3& position)
			{
				m_characterController.SetPosition(position);
				m_transform.SetLocalPosition(position);
				m_moveSpeed = Vector3(0.0f, 0.0f, 0.0f);
			}


		protected:
			/** キャラクターステートマシン */
			std::unique_ptr<CharacterStateMachine> m_characterStateMachine;
			/** アニメーションクリップ */
			std::unique_ptr<AnimationClip[]> m_animationClips;
			/** キャラクターコントローラー */
			CharacterController m_characterController;
			/** アニメーションクリップ数 */
			int m_clipNum;
			/** モデルの上方向 */
			EnModelUpAxis m_upAxis;

			/** 移動速度 */
			Vector3 m_moveSpeed;
		};
	}
}

