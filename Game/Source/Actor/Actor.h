/**
 * @file Actor.h
 * @brief 簡単なアクタークラス
 */
#pragma once
#include "Source/Core/Transform.h"


namespace app
{
	namespace nsK2Engine
	{
		class ModelRender;
	}


	namespace actor
	{
		class Actor : public IGameObject
		{
		public:
			Actor();
			virtual ~Actor();

			/**
			 * @brief トランスフォームを取得する
			 * @return トランスフォーム
			 */
			inline const core::Transform& GetTransform() const { return m_transform; }
			/**
			 * @brief モデルレンダーを取得する
			 * @return モデルレンダー
			 */
			inline nsK2Engine::ModelRender* GetModelRender() const { return m_modelRender; }


		protected:
			//--------------------------------------------------------------
			// TODO: 純粋仮想関数の為Actorクラスは抽象クラス
			// --------------------------------------------------------------
			/**
			 * @brief 初期化処理
			 */
			virtual bool Start() override = 0;
			/**
			 * @brief 更新処理
			 */
			virtual void Update() override = 0;
			/**
			 * @brief 描画処理
			 */
			virtual void Render(RenderContext& rc) override = 0;


		protected:
			/** モデルレンダー */
			nsK2Engine::ModelRender* m_modelRender;
			/** トランスフォーム */
			core::Transform m_transform;
		};
	}
}

