/**
 * @file StageObject.h
 * @brief ステージのオブジェクトの基底クラス
 */
#pragma once
#include "Source/Actor/Actor.h"
#include <memory>
#include <string>


namespace app
{
	namespace actor
	{
		/**
		 * @brief ステージのオブジェクトの基底クラス
		 * @details 回転は度数(オイラー角)で持って、Quaternion はそこから作る。
		 *          エディタとギズモ(ImGuizmo)が度数で扱うため
		 */
		class StageObject : public Actor
		{
		public:
			StageObject();
			~StageObject() override;

			/** 初期化処理(コリジョンを作る) */
			bool Start() override;
			/** 更新処理 */
			void Update() override;
			/** 描画処理 */
			void Render(RenderContext& rc) override;

			/**
			 * @brief 初期化(Stage が JSON を読んだ後に呼ぶ)
			 * @param name 名前
			 * @param modelPath モデルのパス
			 * @param position 位置
			 * @param rotationDeg 回転(度数)
			 * @param scale スケール
			 * @param isCollision コリジョンを付けるか
			 */
			void Initialize(
				const std::string& name,
				const std::string& modelPath,
				const Vector3& position,
				const Vector3& rotationDeg,
				const Vector3& scale,
				const bool isCollision
			);


		public:
			/**
			 * @brief コリジョンを、今の位置・回転・スケールで作り直す
			 * @details エディタで動かし終わった時に呼ぶ(毎フレーム呼ぶと重い)
			 */
			void RebuildCollision();

			/**
			 * @brief 名前を設定
			 * @param name 名前
			 */
			void SetName(const std::string& name) { m_name = name; }
			/**
			 * @brief 名前を取得
			 * @return 名前
			 */
			const std::string& GetName() const { return m_name; }

			/**
			 * @brief モデルのパスを取得
			 * @return モデルのパス
			 */
			const std::string& GetModelPath() const { return m_modelPath; }

			/**
			 * @brief 位置を取得
			 * @return 位置
			 * @details 親がいないので、ワールドの位置 = ローカルの位置
			 */
			const Vector3& GetPosition() const { return m_transform.GetWorldPosition(); }
			/**
			 * @brief スケールを取得
			 * @return スケール
			 */
			const Vector3& GetScale() const { return m_transform.GetWorldScale(); }

			/**
			 * @brief 回転を度数で設定
			 * @param rotationDeg 回転(度数)
			 */
			void SetRotationDeg(const Vector3& rotationDeg);
			/**
			 * @brief 回転を度数で取得
			 * @return 回転(度数)
			 */
			const Vector3& GetRotationDeg() const { return m_rotationDeg; }

			/**
			 * @brief コリジョンを付けるかを設定
			 * @param isCollision 付けるか
			 * @details 反映するには RebuildCollision を呼ぶ
			 */
			void SetIsCollision(const bool isCollision) { m_isCollision = isCollision; }
			/**
			 * @brief コリジョンを付けるか
			 * @return 付けるなら true
			 */
			bool IsCollision() const { return m_isCollision; }

			/**
			 * @brief モデルのポリゴン(三角形)の数を取得
			 * @return 三角形の数
			 * @details 読み込んだ時に1回だけ数えて覚えておく
			 */
			int GetTriangleCount() const { return m_triangleCount; }


		public:
			/**
			 * @brief スケールが反転しているか
			 * @param scale スケール
			 * @return 反転していれば true
			 * @details 3ds Max のミラーで置いた物は、スケールがマイナスになる
			 */
			static bool IsMirrored(const Vector3& scale);
			/**
			 * @brief 度数の回転から Quaternion を作る
			 * @param rotationDeg 回転(度数)
			 * @return 回転
			 * @details ギズモと同じ計算(ImGuizmo)を使うので、ギズモで回した結果と必ず一致する
			 */
			static Quaternion ToQuaternion(const Vector3& rotationDeg);
			/**
			 * @brief Quaternion を度数の回転にする
			 * @param rotation 回転
			 * @return 回転(度数)
			 * @details tkl から読み込んだ回転を、度数にするのに使う
			 */
			static Vector3 ToRotationDeg(const Quaternion& rotation);
			/**
			 * @brief 回転の行列を度数の回転にする
			 * @param rotationMatrix 回転だけの行列(スケールと位置は入れない)
			 * @return 回転(度数)
			 * @details ToQuaternion と同じ順番(X → Y → Z)で分ける。
			 *          Y がちょうど ±90 度の時(ジンバルロック)も、正しい向きになるように求める
			 */
			static Vector3 RotationMatrixToDeg(const Matrix& rotationMatrix);
			/**
			 * @brief モデルを囲む球の中心(ワールド)を取得
			 * @return 球の中心(ワールド)
			 */
			const Vector3& GetBoundingCenter() const { return m_boundingCenter; }
			/**
			 * @brief モデルを囲む球の半径を取得
			 * @return 球の半径
			 */
			float GetBoundingRadius() const { return m_boundingRadius; }


		private:
			/**
			 * @brief トランスフォームを更新して、モデルに反映する
			 * @details コリジョンはモデルのワールド行列から作るので、作る前にも呼ぶ
			 */
			void ApplyTransform();
			/**
			 * @brief 囲む球を今のワールド行列で計算し直す
			 */
			void UpdateBoundingSphere();


		private:
			/** モデルを囲むボックス(モデルの座標系。読み込み時に1回だけ作る) */
			AABB m_localAABB;
			/** 囲む球の中心(ワールド) */
			Vector3 m_boundingCenter;
			/** 囲む球の半径 */
			float m_boundingRadius;

			/** 名前(一覧の表示と保存用) */
			std::string m_name;
			/** モデルのパス(保存用。ModelRender からは取れない) */
			std::string m_modelPath;
			/** 回転(度数) */
			Vector3 m_rotationDeg;
			/**
			 * 当たり判定
			 * ※ エンジンの MeshCollider は、作り直すと前のデータが残るので、作り直す時は丸ごと作り直す
			 */
			std::unique_ptr<PhysicsStaticObject> m_physicsObject;
			/** コリジョンを付けるか */
			bool m_isCollision;
			/** モデルのポリゴン(三角形)の数 */
			int m_triangleCount;
		};
	}
}
