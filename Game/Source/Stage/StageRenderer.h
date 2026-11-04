/**
 * @file StageRenderer.h
 * @brief モデルの種類ごとにグループ分けして描く
 */
#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
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
		 * @brief モデルの種類ごとにグループ分けして描く(インスタンシング描画)
		 * @details 「tkm × 反転の有無」ごとに InstancedModelRender を1つ作り、同じグループの置物をまとめて描く。
		 *          反転したモデルはカリング(表と裏のどちらを描かないか)が違うので、別のグループにする
		 */
		class StageRenderer
		{
		public:
			StageRenderer();
			~StageRenderer();

			/**
			 * @brief 置物を何個登録する予定かを先に伝えて、グループを作っておく
			 * @param modelPath モデルのパス
			 * @param isMirrored 反転しているか
			 * @param count 登録する予定の数
			 * @details 先に伝えないと、Register で数が増えるたびにグループを作り直すことになる(読み込みが遅くなる)
			 */
			void Reserve(const std::string& modelPath, const bool isMirrored, const int count);
			/**
			 * @brief 置物を登録する(グループが無ければ作る)
			 * @param modelPath モデルのパス
			 * @param isMirrored 反転しているか
			 * @return 置物が入るグループの描画
			 * @details 最大数を超えたら、そのグループを大きく作り直す(返すアドレスは変わらない)
			 */
			InstancedModelRender& Register(const std::string& modelPath, const bool isMirrored);
			/**
			 * @brief 置物の登録を外す
			 * @param modelPath モデルのパス
			 * @param isMirrored 反転しているか
			 * @details 0個になってもグループは消さない(すぐにまた追加されることが多いため)
			 */
			void Unregister(const std::string& modelPath, const bool isMirrored);
			/**
			 * @brief 映っている置物を、グループごとにまとめて描く
			 * @param rc 描画コンテキスト
			 * @param objectList 置物の一覧
			 */
			void Render(RenderContext& rc, const std::vector<std::unique_ptr<actor::StageObject>>& objectList);
			/** グループを全部消す */
			void Clear();


		private:
			/** グループ */
			struct Group
			{
				/** まとめて描く描画 */
				InstancedModelRender render;
				/** 登録されている置物の数 */
				int count = 0;
				/** 最大数(行列リストの大きさ) */
				int capacity = 0;
			};
			/** グループのキー(tkm, 反転しているか) */
			using GroupKey = std::pair<std::string, bool>;

			/**
			 * @brief グループを探して、無ければ作る
			 * @param key キー
			 * @param capacity 作る時の最大数
			 * @return グループ
			 */
			Group& FindOrCreateGroup(const GroupKey& key, const int capacity);
			/**
			 * @brief グループの描画を、最大数を指定して作る(作り直す)
			 * @param key キー
			 * @param group グループ
			 * @param capacity 最大数
			 */
			static void InitGroup(const GroupKey& key, Group& group, const int capacity);


		private:
			/**
			 * キー:(tkm, 反転しているか)
			 * ※ 置物はグループの render のアドレスを持つので、map が並び替わってもアドレスが変わらないように unique_ptr で持つ
			 */
			std::map<GroupKey, std::unique_ptr<Group>> m_groupMap;
		};
	}
}
