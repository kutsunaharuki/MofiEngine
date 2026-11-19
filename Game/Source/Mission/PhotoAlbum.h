/**
 * @file PhotoAlbum.h
 * @brief 撮った写真を残しておくアルバム
 */
#pragma once
#include <array>


namespace app
{
	namespace mission
	{
		/**
		 * @brief 撮った写真を残しておくアルバム
		 * @details PhotoCapture の写真用のRTは、撮るたびに上書きされて、インゲームと一緒に消える。
		 *          リザルトで写真を並べるために、写真の描き先をここに何枚分か持っておく。
		 *          いっぱいになったら、一番古い写真から上書きする
		 */
		class PhotoAlbum : public Noncopyable
		{
		public:
			/** 残せる写真の枚数 */
			static constexpr int CAPACITY = 12;


		public:
			PhotoAlbum();
			~PhotoAlbum();

			/**
			 * @brief 写真の描き先を作る(最初の 1 回だけ作る)
			 * @param width 写真の横幅(PhotoCapture::GetPhotoWidth)
			 * @param height 写真の縦幅(PhotoCapture::GetPhotoHeight)
			 */
			void Initialize(const int width, const int height);
			/**
			 * @brief 写真を空にする(描き先は残して、使い回す)
			 */
			void Clear();
			/**
			 * @brief 次の写真の描き先を取る
			 * @param photoIndex 写真の番号(撮った順に 0, 1, 2...)
			 * @return 描き先(作っていなければ nullptr)
			 * @details 返した描き先を PhotoCapture::SetSaveTarget に渡す
			 */
			RenderTarget* Reserve(int& photoIndex);
			/**
			 * @brief 写真の絵を取得
			 * @param photoIndex 写真の番号
			 * @return 写真の絵(番号が無い・もう上書きされた時は nullptr)
			 */
			Texture* GetTexture(const int photoIndex);


		public:
			/**
			 * @brief 今までに撮った枚数を取得
			 * @return 撮った枚数(上書きされた写真も数える)
			 */
			int GetShotCount() const { return m_shotCount; }


		private:
			/** 写真の描き先(撮った番号 % CAPACITY の場所に入れる) */
			std::array<RenderTarget, CAPACITY> m_photoList;
			/** 今までに撮った枚数 */
			int m_shotCount;
			/** 描き先を作ったか */
			bool m_isInitialized;
		};
	}
}
