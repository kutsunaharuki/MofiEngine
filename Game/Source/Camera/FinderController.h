/**
 * @file FinderController.h
 * @brief ファインダー(写真撮影)の操作
 */
#pragma once
#include "Source/Util/ParameterManager.h"
#include "PhotoSubject.h"
#include <vector>
#include "Object/FinderParameter.h"


namespace app
{
	namespace camera
	{
		/**
		 * @brief ファインダー(写真撮影)の操作
		 * @details 操作 : Y で覗く / やめる、RB1 / LB1 でズーム、B で撮影。
		 *          ピントはオートフォーカス(画面の真ん中に近い被写体に、少しずつ合わせる)。
		 *          撮影には、クールタイムがある
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
			 * @brief ファインダーの中心に近い被写体を選ぶ
			 * @return 選ばれた被写体(無ければ nullptr)
			 */
			const IPhotoSubject* FinderFocusSubject();

			/**
			 * @brief ファインダーの中心に近い被写体にピントを合わせる
			 */
			void AutoFocus();


		private:
			/** 操作する写真撮影クラス */
			PhotoCapture* m_photoCapture;
			/** ピントを合わせている被写体 */
			const IPhotoSubject* m_focusSubject;
			/** 調整用パラメーター */
			util::Parameter<FinderParameter> m_parameter;
			/** 被写体のリスト */
			std::vector<const IPhotoSubject*> m_subjectList;
			/** クールタイムの残り(秒) */
			float m_coolTimer;
		};
	}
}
