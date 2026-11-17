/**
 * @file ScreenPosition.h
 * @brief 3Dの位置を画面(UI)の位置に変える
 */
#pragma once


namespace app
{
	namespace util
	{
		/**
		 * @brief 3D の位置を、画面の位置に変える
		 * @param camera カメラ
		 * @param worldPosition 3D の位置
		 * @param out 画面の位置(画面の真ん中が原点、上が +Y。UI と同じ座標)
		 * @return カメラの前にあれば true(後ろにある物は、画面に写らない)
		 * @details Camera::CalcScreenPositionFromWorldPosition と同じ計算に、カメラの後ろかの判定を足したもの
		 */
		bool CalcScreenPosition(Camera& camera, const Vector3& worldPosition, Vector2& out);
	}
}


