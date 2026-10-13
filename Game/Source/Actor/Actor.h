/**
 * @file Actor.h
 * @brief 見た目が存在するオブジェクトの基底クラス
 */
#pragma once
#include "Source/Core/TransformNode.h"


namespace app
{
    namespace actor
    {
        /**
		 * @brief 見た目が存在するオブジェクトの基底クラス
         */
        class Actor
        {
        public:
            /**
             * @brief トランスフォームノードを取得
             * @return トランスフォームノード
             */
            inline const app::core::TransformNode& GetTransform() const { return m_transform; }
            /**
             * @brief トランスフォームノードを取得(const無しバージョン)
			 * @return トランスフォームノード
             * @details const無しバージョンは親子付けが必要になった時の為に用意している
             */
            inline const app::core::TransformNode& GetTransform() { return m_transform; }
            /**
             * @brief モデルレンダーを取得
             * @return モデルレンダー
             */
            inline ModelRender* GetModelRender() { return &m_modelRender; }

            /**
             * @brief 位置を設定
             * @param pos 位置
             */
            void SetPosition(const Vector3& pos)
            {
                m_transform.SetLocalPosition(pos);
            }
            /**
             * @brief 回転を設定
             * @param rot 回転
             */
			void SetRotation(const Quaternion& rot)
			{
				m_transform.SetLocalRotation(rot);
			}
            /**
             * @brief スケールを設定
             * @param scale スケール
             */
			void SetScale(const Vector3& scale)
			{
				m_transform.SetLocalScale(scale);
			}
            /**
             * @brief アニメーションの再生速度の設定
             * @param speed 再生速度(1.0 = 通常速度)
             */
            void SetAnimationSpeed(float speed) { m_modelRender.SetAnimationSpeed(speed); }


        public:
            Actor() = default;
            virtual ~Actor() = default;


        public:
            /** 初期化処理 */
            virtual bool Start() = 0;
            /** 更新処理 */
            virtual void Update()  = 0;
			/** 描画処理 */
			virtual void Render(RenderContext& rc) = 0;


        protected:
            /** トランスフォーム */
            app::core::TransformNode m_transform;
            /** モデルレンダー */
            ModelRender m_modelRender;
        };
    }
}

