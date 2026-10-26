/**
 * @file Frustum.cpp
 * @brief フラスタム
 */
#include "k2EngineLowPreCompile.h"
#include "Frustum.h"


namespace nsK2EngineLow
{
	namespace
	{
		/**
		 * @brief 行列の column 列目を取り出す(列はColumn、行はRow)
		 * @param mt 行列
		 * @param column 列番号(0 ~ 3)
		 * @return 取り出した列
		 * @details DirectX::XMMATRIX は列優先なので、m[0][column] が 1 列目の column 行目になる
		 */
		Vector4 GetColumn(const Matrix& mt, const int column)
		{
			return Vector4(mt.m[0][column], mt.m[1][column], mt.m[2][column], mt.m[3][column]);
		}

		/**
		 * @brief 視錐台の面を1枚作る(奥行きの列 + 向き * 軸の列)
		 * @param depthColumn 奥行き(w)の列。いつも3列
		 * @param axisColumn 軸の列(列0 = 横、列1 = 縦、列2 = 奥)
		 * @param side どちら側の端か(+1 = 左・下、-1 = 右・上・奥)
		 */
		Vector4 MakeFrustumPlane(const Vector4& depthColumn, const Vector4& axisColumn, const float side)
		{
			return Vector4(
				depthColumn.x + axisColumn.x * side,
				depthColumn.y + axisColumn.y * side,
				depthColumn.z + axisColumn.z * side,
				depthColumn.w + axisColumn.w * side
			);
		}
	}




	/*********************************************/


	void Frustum::Update(const Matrix& viewProj)
	{
		// エンジンの行列は DirectX の形(行ベクトル)なので、「列」を使って面を作る(Gribb-Hartman 法)
		const Vector4 c0 = GetColumn(viewProj, 0);
		const Vector4 c1 = GetColumn(viewProj, 1);
		const Vector4 c2 = GetColumn(viewProj, 2);
		const Vector4 c3 = GetColumn(viewProj, 3);

		m_planes[0] = MakeFrustumPlane(c3, c0, 1.0f);  // 奥行き + 横 → 左の端
		m_planes[1] = MakeFrustumPlane(c3, c0, -1.0f); // 奥行き- 横 → 右の端
		m_planes[2] = MakeFrustumPlane(c3, c1, 1.0f);  // 奥行き + 縦 → 下の端
		m_planes[3] = MakeFrustumPlane(c3, c1, -1.0f); // 奥行き- 縦 → 上の端
		m_planes[4] = c2;                              // 奥行き方向だけ → 手前の端
		m_planes[5] = MakeFrustumPlane(c3, c2, -1.0f); // 奥行き - 奥行き方向 → 奥の端


		// 法線の長さで割って、 w を「距離」として使えるようにする
		for (Vector4& plane : m_planes)
		{
			const float length = std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
			if (length <= 0.0f) continue;

			plane.x /= length;
			plane.y /= length;
			plane.z /= length;
			plane.w /= length;
		}
	}


	bool Frustum::IsInside(const Vector3& center, float radius) const
	{
		for (const Vector4& plane : m_planes)
		{
			// 面からの距離(プラスなら内側)
			const float distance = plane.x * center.x + plane.y * center.y + plane.z * center.z + plane.w;

			// 半径の分よりも外側なら、球は丸ごと外にある
			if (distance < -radius) return false;
		}
		return true;
	}
}