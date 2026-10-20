/**
 * @file PhotoSubject.h
 * @brief 写真の被写体の情報
 */
#pragma once


namespace app
{
	namespace camera
	{
		/**
		 * @brief 写真の被写体の情報
		 * @details ピントを合わせる位置(キャラクターの体の中心や顔のあたり)
		 */
		class IPhotoSubject
		{
		public:
			virtual ~IPhotoSubject() = default;
			/** ピントを合わせる位置(体の中心または顔のあたり) */
			virtual Vector3 GetFocusPosition() const = 0;
		};
	}
}