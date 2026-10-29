/**
 * @file Stage.h
 * @brief ステージ(置物の一覧と、その読み込み・保存)
 */
#pragma once
#include <memory>
#include <string>
#include <vector>


namespace app
{
	/** 前方宣言 */
	namespace actor
	{
		class StageObject;
	}


	namespace stage
	{
		/**
		 * @brief ステージ
		 * @details 置物(StageObject)をまとめて持って、JSON から読み込む・JSON に保存する。
		 *          3ds Max で作った tkl から、最初の配置を読み込むこともできる
		 *
		 * JSON の形
		 * {
		 *   "objects": [
		 *     { "name": "SM_Bench", "model": "Assets/modelData/Stage/SM_Bench.tkm",
		 *       "position": [0, 0, 0], "rotation": [0, 0, 0], "scale": [1, 1, 1], "collision": true }
		 *   ]
		 * }
		 */
		class Stage
		{
		public:
			Stage();
			~Stage();

			/** 更新処理 */
			void Update();
			/** 描画処理 */
			void Render(RenderContext& rc);


		public:
			/**
			 * @brief JSON から読み込む(今の置物は全部消す)
			 * @param filePath JSON のパス
			 * @return 読み込めたか(ファイルが無ければ false)
			 */
			bool Load(const std::string& filePath);
			/**
			 * @brief JSON に保存する
			 * @param filePath JSON のパス
			 * @return 保存できたか
			 */
			bool Save(const std::string& filePath) const;
			/**
			 * @brief 3ds Max で作った tkl から読み込む(今の置物は全部消す)
			 * @param tklFilePath tkl のパス
			 * @param modelDirectory モデルのフォルダ(最後に / を付ける)
			 * @return 1つでも読み込めたか
			 * @details tkl のオブジェクトの名前 + ".tkm" をモデルのファイル名にする
			 */
			bool ImportTkl(const char* tklFilePath, const std::string& modelDirectory);

			/**
			 * @brief 置物を追加する
			 * @param name 名前
			 * @param modelPath モデルのパス
			 * @param position 位置
			 * @param rotationDeg 回転(度数)
			 * @param scale スケール
			 * @param isCollision コリジョンを付けるか
			 * @return 追加した置物(モデルのファイルが無ければ nullptr)
			 */
			actor::StageObject* AddObject(
				const std::string& name,
				const std::string& modelPath,
				const Vector3& position,
				const Vector3& rotationDeg,
				const Vector3& scale,
				const bool isCollision
			);
			/**
			 * @brief 置物を消す
			 * @param index 番号
			 */
			void RemoveObject(const int index);
			/** 置物を全部消す */
			void Clear();

			/**
			 * @brief 置物の数を取得
			 * @return 置物の数
			 */
			int GetObjectCount() const { return static_cast<int>(m_objectList.size()); }
			/**
			 * @brief 置物を取得
			 * @param index 番号
			 * @return 置物(範囲外なら nullptr)
			 */
			actor::StageObject* GetStageObject(const int index);
			/**
			 * @brief 置物を取得(const版)
			 * @param index 番号
			 * @return 置物(範囲外なら nullptr)
			 */
			const actor::StageObject* GetStageObject(const int index) const;


		private:
			/** 置物の一覧 */
			std::vector<std::unique_ptr<actor::StageObject>> m_objectList;
		};
	}
}
