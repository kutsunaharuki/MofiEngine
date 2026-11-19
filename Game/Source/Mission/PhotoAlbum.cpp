/**
 * @file PhotoAlbum.cpp
 * @brief 撮った写真を残しておくアルバム
 */
#include "stdafx.h"
#include "PhotoAlbum.h"


namespace app
{
	namespace mission
	{
		PhotoAlbum::PhotoAlbum()
			: m_shotCount(0)
			, m_isInitialized(false)
		{}


		PhotoAlbum::~PhotoAlbum()
		{}


		void PhotoAlbum::Initialize(const int width, const int height)
		{
			// 描き先は使い回す(リトライのたびに作り直さない)
			if (m_isInitialized) return;

			// ※ 写真用のRT(PhotoCapture)と同じ大きさ・形式にする。違うと写す時に絵がずれる
			float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
			for (RenderTarget& photo : m_photoList)
			{
				photo.Create(
					width, height,
					1, 1,
					DXGI_FORMAT_R16G16B16A16_FLOAT,
					DXGI_FORMAT_UNKNOWN,
					clearColor
				);
			}
			m_isInitialized = true;
		}


		void PhotoAlbum::Clear()
		{
			// 絵は残っているが、番号で見えなくなる(次に撮った写真で上書きされる)
			m_shotCount = 0;
		}


		RenderTarget* PhotoAlbum::Reserve(int& photoIndex)
		{
			photoIndex = -1;
			if (!m_isInitialized) return nullptr;

			photoIndex = m_shotCount;
			++m_shotCount;
			// いっぱいになったら、一番古い写真の場所に入れる
			return &m_photoList[photoIndex % CAPACITY];
		}


		Texture* PhotoAlbum::GetTexture(const int photoIndex)
		{
			if (!m_isInitialized) return nullptr;
			// まだ撮っていない番号
			if (photoIndex < 0 || photoIndex >= m_shotCount) return nullptr;
			// もう新しい写真で上書きされた番号
			if (photoIndex < m_shotCount - CAPACITY) return nullptr;

			return &m_photoList[photoIndex % CAPACITY].GetRenderTargetTexture();
		}
	}
}
