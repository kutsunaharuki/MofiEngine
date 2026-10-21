/**
 * @file FinderController.h
 * @brief ファインダー(写真撮影)の操作
 */
#pragma once
#include "Source/Util/ParameterManager.h"
#include "Source/UI/UILayout.h"
#include "PhotoSubject.h"
#include <vector>
#include "Object/FinderParameter.h"


namespace app
{
	/** 前方宣言 */
	namespace ui
	{
		class FinderMenu;
	}


	namespace camera
	{
		/**
		 * @brief ファインダー(写真撮影)の操作
		 * @details 操作 : Y で覗く / やめる、RB1 / LB1 でズーム、B で撮影。
		 *          ピントはオートフォーカス。顔が真ん中のグリッドの中にある被写体のうち、一番真ん中に近いものに合わせる。
		 *          被写体の顔のまわりに丸を出し、真ん中の十字が丸の中に入っていれば「範囲内」になる。
		 *          構えている間だけ、ファインダーの UI(FinderMenu)を出す。撮影には、クールタイムがある
		 *
		 * シーンからの呼び方(1フレーム)
		 *   1. Update()     … 入力(覗く・ズーム・撮影)。シーンの最初の方で呼ぶ
		 *   2. (シーンがファインダーのカメラを、ゲームカメラに合わせる)
		 *   3. LateUpdate() … オートフォーカスと UI。ファインダーのカメラを合わせた後に呼ぶ
		 *   4. Render(rc)   … UI の描画
		 */
		class FinderController : public Noncopyable
		{
		public:
			FinderController();
			~FinderController();

			/**
			 * @brief 初期化
			 * @param photoCapture 操作する写真撮影クラス
			 */
			void Initialize(PhotoCapture* photoCapture);

			/**
			 * @brief 更新処理(入力を受け付ける)
			 * @return このフレームで写真を撮ったら true
			 */
			bool Update();
			/**
			 * @brief オートフォーカスと UI の更新
			 * @details 被写体の顔を画面の位置に変えて計算するので、ファインダーのカメラを合わせた後に呼ぶ
			 *          (先に呼ぶと、カメラを振った時に丸が1フレーム遅れてずれる)
			 */
			void LateUpdate();
			/**
			 * @brief UI の描画
			 * @param rc レンダリングコンテキスト
			 * @details 構えていない時は、何も描画されない
			 */
			void Render(RenderContext& rc);


		public:
			/**
			 * @brief 今、撮影できるか(クールタイムが終わっているか)
			 * @return 撮影できれば true
			 */
			bool CanShot() const { return m_coolTimer <= 0.0f; }
			/**
			 * @brief クールタイムの残りを取得
			 * @return 残りの秒数(0 なら撮影できる)
			 */
			float GetCoolTimeRemaining() const { return m_coolTimer; }
			/**
			 * @brief ピントを合わせている被写体を取得
			 * @return IPhotoSubjectのポインタ
			 */
			const IPhotoSubject* GetFocusSubject() const { return m_focusSubject; }
			/**
			 * @brief ピントを合わせている被写体が「範囲内」か
			 * @return 真ん中の十字が、被写体の顔のまわりの丸の中に入っていれば true
			 */
			bool IsSubjectInRange() const { return m_isSubjectInRange; }


		public:
			/**
			 * @brief 被写体を登録する
			 * @param subject 被写体の情報を登録
			 */
			void RegisterSubject(const IPhotoSubject* subject);
			/**
			 * @brief 被写体の登録を解除する
			 * @param subject 被写体の情報を解除
			 */
			void UnregisterSubject(const IPhotoSubject* subject);


		private:
			/**
			 * @brief 画面に写った被写体の情報
			 */
			struct SubjectOnScreen
			{
				/** 被写体 */
				const IPhotoSubject* subject = nullptr;
				/** 顔の画面の位置(画面の真ん中が原点、上が +Y) */
				Vector2 screenPosition = Vector2(0.0f, 0.0f);
				/** 顔のまわりの丸の半径(ピクセル) */
				float circleRadius = 0.0f;
				/** カメラから顔までの距離(ピントの目標) */
				float distance = 0.0f;
				/** 画面の真ん中から顔までの距離(ピクセル) */
				float screenDistance = 0.0f;
			};


		private:
			/**
			 * @brief 顔が真ん中のグリッドの中にある被写体のうち、一番真ん中に近いものを選ぶ
			 * @param out 選んだ被写体の情報
			 * @return 見つかったら true
			 */
			bool FindFocusSubject(SubjectOnScreen& out);
			/**
			 * @brief 選んだ被写体にピントを合わせて、範囲内かを判定し、丸を出す
			 */
			void AutoFocus();
			/**
			 * @brief FinderMenu の開く・閉じるを、ファインダーを構えているかに合わせる
			 */
			void SyncMenuOpen();


		private:
			/** 操作する写真撮影クラス */
			PhotoCapture* m_photoCapture;
			/** ファインダーの UI のレイアウト */
			ui::UILayout m_finderLayout;
			/** ファインダーの UI(m_finderLayout が持っている) */
			ui::FinderMenu* m_finderMenu;
			/** ピントを合わせている被写体 */
			const IPhotoSubject* m_focusSubject;
			/** ピントを合わせている被写体が「範囲内」か */
			bool m_isSubjectInRange;
			/** 調整用パラメーター */
			util::Parameter<FinderParameter> m_parameter;
			/** 被写体のリスト */
			std::vector<const IPhotoSubject*> m_subjectList;
			/** クールタイムの残り(秒) */
			float m_coolTimer;
		};
	}
}
