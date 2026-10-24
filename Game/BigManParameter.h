/**
 * @file BigManParameter.h
 * @brief BigManのパラメーター
 */
#pragma once
#include "Source/Util/ParameterFile.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief BigManのパラメーター
		 */
		struct BigManParameter
		{
			/** 歩く速度 */
			float walkSpeed = 1.0f;
			/** 更新される位置 */
			Vector3 updatePosition = Vector3::Zero;
			/** 更新される回転 */
			Quaternion updateRotation = Quaternion::Identity;
			/** 更新されるスケール */
			Vector3 updateScale = Vector3::One;


			//void Read(const util::ParameterFile& file)
			//{

			//}
		};
	}
}

