/**
 * @file EnemyController.h
 * @brief 敵の入力クラス
 */
#pragma once
#include <string>
#include <vector>


namespace app
{
	namespace stage
	{
		/** 前方宣言 */
		class Stage;
		class StageLayout;
	}


	namespace actor
	{
		/** 前方宣言 */
		class EnemyBase;


		/**
		 * @brief 敵の入力クラス
		 * @details ステージから美術品を探して、敵がいる階の美術品だけを、盗みに行く一覧として敵に渡す。
		 *          一覧の中からどれに向かうかは、敵ごとの目的地モード(EnemyDestinationMode)が決める
		 */
		class EnemyController
		{
		public:
			EnemyController();
			~EnemyController();

			/**
			 * @brief 更新処理
			 * @details 敵がいる階が変わったら、その階の美術品の一覧を渡し直す。敵の Update より先に呼ぶ
			 */
			void Update();


		public:
			/**
			 * @brief ステージの構造を設定(階を決めるのに使う)
			 * @param layout ステージの構造(シーンが持っているものを借りる。nullptr なら全部 1F 扱い)
			 * @details CollectArtTargets と RegisterEnemy より先に呼ぶ
			 */
			void SetLayout(const stage::StageLayout* layout) { m_layout = layout; }

			/**
			 * @brief 敵を登録する(今いる階の美術品の一覧をすぐ渡す)
			 * @param enemy 敵(シーンが持っているものを借りる)
			 * @details 敵の Start より前に呼んでも、後に呼んでもよい(Start で位置が変わっても、Update で渡し直す)
			 */
			void RegisterEnemy(EnemyBase* enemy);

			/**
			 * @brief ステージの置物から美術品を探して、一覧を作り直す
			 * @param stage ステージ
			 * @return 見つかった美術品の数(全部の階)
			 * @details エディタで置物を動かした後にも呼ぶ
			 */
			int CollectArtTargets(const stage::Stage& stage);
			/**
			 * @brief 美術品の位置を1つ足す(デバッグ用)
			 * @param position 美術品の位置(高さで階を決める)
			 */
			void AddTarget(const Vector3& position);
			/**
			 * @brief 美術品の一覧を空にする(デバッグ用)
			 */
			void ClearTargets();

			/**
			 * @brief 美術品の数を取得
			 * @return 美術品の数(全部の階)
			 */
			int GetTargetCount() const { return static_cast<int>(m_targetList.size()); }
			/**
			 * @brief 指定した階の美術品の位置の一覧を作る
			 * @param floor 階の番号(1F が 0、2F が 1)
			 * @return その階の美術品の位置の一覧
			 */
			std::vector<Vector3> MakeFloorTargetList(const int floor) const;

			/**
			 * @brief 高さから階の番号を求める
			 * @param height 高さ(y)
			 * @return 階の番号(1F が 0、2F が 1)
			 * @details ステージの構造の、階ごとの床の高さで決める
			 */
			int ToFloorIndex(const float height) const;


		private:
			/** 美術品 */
			struct ArtTarget
			{
				/** 位置(行き先にする) */
				Vector3 position;
				/** 階の番号 */
				int floor;
			};

			/** 登録された敵 */
			struct EnemyEntry
			{
				/** 敵(シーンのものを借りる) */
				EnemyBase* enemy;
				/** 最後に一覧を渡した時の階の番号 */
				int floor;
			};


		private:
			/**
			 * @brief 置物の名前が美術品かどうか
			 * @param name 置物の名前
			 * @return 美術品なら true
			 */
			static bool IsArt(const std::string& name);
			/**
			 * @brief 敵に、今いる階の美術品の一覧を渡す
			 * @param entry 登録された敵
			 */
			void ApplyTargetList(EnemyEntry& entry);
			/**
			 * @brief 登録された敵全員に、今いる階の美術品の一覧を渡す
			 */
			void ApplyAllTargetList();


		private:
			/** ステージの構造(シーンのものを借りる) */
			const stage::StageLayout* m_layout;
			/** 登録された敵 */
			std::vector<EnemyEntry> m_enemyList;
			/** 美術品の一覧(全部の階) */
			std::vector<ArtTarget> m_targetList;
		};
	}
}
