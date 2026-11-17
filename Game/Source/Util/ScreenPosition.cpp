/**
 * @file ScreenPosition.cpp
 * @brief 3Dの位置を画面(UI)の位置に変える
 */
#include "stdafx.h"
#include "ScreenPosition.h"


namespace app
{
	namespace util
	{
		bool CalcScreenPosition(Camera& camera, const Vector3& worldPosition, Vector2& out)
		{
			{
				Vector4 position(worldPosition.x, worldPosition.y, worldPosition.z, 1.0f);
				camera.GetViewProjectionMatrix().Apply(position);

				// w がマイナス(または 0)なら、カメラの後ろ
				if (position.w <= 0.0001f) return false;

				// -1 ~ 1 の範囲を、画面のピクセルにする(ファインダーの画面は画面全体に出ている)
				out.x = (position.x / position.w) * UI_SPACE_WIDTH * 0.5f;
				out.y = (position.y / position.w) * UI_SPACE_HEIGHT * 0.5f;
				return true;
			}
		}
	}
}