/**
 * @file Frustum.h
 * @brief フラスタム
 */
#pragma once


namespace nsK2EngineLow
{
	/**
	 * @brief フラスタム
	 */
	class Frustum
	{
	public:
		/**
		 * @brief ビュープロジェクション行列から6枚の面を作る
		 * @param viewProj ビュープロジェクション行列
		 */
		void Update(const Matrix& viewProj);

		/**
		 * @brief 球がフラスタム内にあるか
		 * @param center 中心
		 * @param radius 半径
		 * @return 少しでも入っていれば true
		 */
		bool IsInside(const Vector3& center, float radius) const;


	private:
		/** 平面の数(左・右・上・下・前・後) */
		static constexpr int PLANE_COUNT = 6;
		/** 面(x,y,z が法線、w が原点からの距離)。法線は内側を向いている */
		Vector4 m_planes[PLANE_COUNT];
	};
}