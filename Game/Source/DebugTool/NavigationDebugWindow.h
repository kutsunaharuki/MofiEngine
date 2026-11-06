/**
 * @file NavigationDebugWindow.h
 * @brief 経路探索のデバッグ表示
 */
#pragma once
#include <memory>
#include <string>
#include <vector>


namespace app
{
	/** 前方宣言 */
	namespace ai
	{
		/** 経路探索の地図 */
		class NavGrid;
	}

	namespace actor
	{
		/** プレイヤー(少年) */
		class Boy;
		/** 敵の基盤 */
		class EnemyBase;
		/** 敵の入力 */
		class EnemyController;
	}

	namespace stage
	{
		/** ステージ */
		class Stage;
		/** ステージレイアウト */
		class StageLayout;
	}


	namespace debug
	{
		/**
		 * @brief 経路探索のデバッグ表示
		 * @details NavGridのミニマップ(赤:通れない)と敵の道順を描く。
		 *          ボタンで敵を主人公の位置へ向かわせたり、地図を作り直したりできる。
		 *          表示するものは InGameScene が持っていて、ここは借りて見せるだけ(作る・消すはしない)
		 */
		class NavigationDebugWindow : public Noncopyable
		{
		public:
			/**
			 * @brief 表示に使うもの(InGameScene が持っているものを借りる)
			 * @details 引数で並べると順番を間違えやすいので、名前で入れられるように構造体にする
			 */
			struct Target
			{
				/** 主人公 */
				actor::Boy* boy;
				/** 敵の一覧 */
				std::vector<std::unique_ptr<actor::EnemyBase>>* enemyList;
				/** 経路探索の地図 */
				ai::NavGrid* navGrid;
				/** ステージ(置物) */
				stage::Stage* stage;
				/** ステージの構造(階・区画・階段) */
				stage::StageLayout* stageLayout;
				/** 敵の入力(美術品・隠れ場所の一覧) */
				actor::EnemyController* enemyController;


				Target()
					: boy(nullptr)
					, enemyList(nullptr)
					, navGrid(nullptr)
					, stage(nullptr)
					, stageLayout(nullptr)
					, enemyController(nullptr)
				{}
			};


		public:
			NavigationDebugWindow();
			~NavigationDebugWindow();

			/**
			 * @brief 初期化処理
			 * @param target 表示に使うもの(全部作り終わってから渡す)
			 */
			void Initialize(const Target& target);

			/**
			 * @brief ウィンドウを表示する
			 * @details Debug の時だけ表示する
			 */
			void Draw();


		private:
			/** 表示に使うもの */
			Target m_target;
			/** 最後の操作の結果 */
			std::string m_message;
		};
	}
}