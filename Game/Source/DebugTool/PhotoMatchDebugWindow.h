/**
 * @file PhotoMatchDebugWindow.h
 * @brief 照合のデバッグ表示(今の写り方を出して、見本の数値を決める道具)
 */
#pragma once


namespace app
{
	/** 前方宣言 */
	namespace actor
	{
		/** 敵の基盤 */
		class EnemyBase;
	}
	namespace mission
	{
		/** ミッションの管理(最後に撮った写真の照合結果を見る) */
		class MissionManager;
	}


	namespace debug
	{
		/**
		 * @brief 照合のデバッグ表示
		 * @details Debug の時だけ出す。ピントの被写体の悪党を毎フレーム測って、値と顔の向きの矢印を出す。
		 *          「Copy as sample」で、今の値を MissionList.json の "sample" の形でクリップボードに入れる(貼るだけで見本になる)
		 *          表示するものは InGameScene が持っていて、ここは借りて見せるだけ(作る・消すはしない)
		 */
		class PhotoMatchDebugWindow : public Noncopyable
		{
		public:
			PhotoMatchDebugWindow() = default;
			~PhotoMatchDebugWindow() = default;

			/**
			 * @brief 描画(ImGui のウィンドウと、ゲームの画面に顔の向きの矢印)
			 * @param focusEnemy ピントの被写体の悪党(悪党でなければ nullptr)
			 * @param finderCamera 撮るカメラ(ファインダーのカメラ)
			 * @param photographer 撮る人の体(レイで無視する)
			 * @param missionManager ミッションの管理(最後に撮った写真の照合結果を出す)
			 * @details ImGui::NewFrame と ImGui::Render の間で呼ぶ(Update の中なら大丈夫)
			 */
			void Draw(actor::EnemyBase* focusEnemy, Camera& finderCamera, const btCollisionObject* photographer, const mission::MissionManager& missionManager);


		private:
			/**
			 * @brief 最後に撮った写真の照合結果を出す(照合結果の UI ができるまでの仮の表示)
			 * @param missionManager ミッションの管理
			 */
			void DrawLastShot(const mission::MissionManager& missionManager);
			/**
			 * @brief 顔の向きの矢印を、ゲームの画面に描く
			 * @param enemy 悪党
			 * @details 頭の位置から、顔の前の向きへ線を引く。鼻の向きに出なければ、敵の JSON の headForwardLocal を直す
			 */
			void DrawFaceArrow(actor::EnemyBase& enemy);
		};
	}
}
