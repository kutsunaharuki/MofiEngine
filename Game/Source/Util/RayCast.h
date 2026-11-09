/**
 * @file RayCast.h
 * @brief 一番近くで当たったものを返すレイ
 */
#pragma once


namespace app
{
	namespace util
	{
		/**
		 * @brief レイの当たり判定の結果
		 */
		struct RayHit
		{
			/** 何かに当たったか */
			bool isHit;
			/** 当たった位置 */
			Vector3 position;
			/** 当たった物(キャラクターは CharacterController) */
			const btCollisionObject* hitObject;


			RayHit()
				:isHit(false)
				, position(Vector3::Zero)
				, hitObject(nullptr)
			{}
		};


		/**
		 * @brief レイが何に当たるかのフィルター
		 */
		enum class RayFilter
		{
			/** 全てに当たる */
			All,
			/** キャラクターには当たらない */
			IgnoreCharacter
		};


		/**
	 	 * @brief 一番近くで当たったものを返すレイ
		 * @param start 撃ち始め
		 * @param end 撃ち終わり
		 * @param ignoreObject 当たらないようにする物(撃つ人の体など)
		 * @param filter レイが何に当たるか
		 */
		RayHit RayCastClosest(
			const Vector3& start,
			const Vector3& end, 
			const btCollisionObject* ignoreObject = nullptr,
			const RayFilter filter = RayFilter::All
		);
	}
}

