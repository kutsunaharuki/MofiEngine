#pragma once


namespace app {
	namespace core {


		/**
		 * @brief Transformクラス
		 */
		class Transform : public Noncopyable
		{
		public:
			/** 位置 */
			Vector3 position_ = Vector3::Zero;
			/** 回転 */
			Quaternion rotation_ = Quaternion::Identity;
			/** スケール */
			Vector3 scale_ = Vector3::One;
		};
	}
}
