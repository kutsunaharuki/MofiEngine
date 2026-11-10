/**
 * @file LightParameter.h
 * @brief 明かりの調整パラメーター
 */
#pragma once
#include "Source/Util/ParameterManager.h"


namespace app
{
	namespace stage
	{
		/**
		 * @brief 明かりの調整パラメーター
		 */
		struct LightParameter
		{
			/** ディレクションライトの色 */
			Vector3 directionColor;
			/** 環境光の色(下げるほど影がはっきりする) */
			Vector3 ambient;
			/** ディレクションライトの向き */
			Vector3 directionDir;


			LightParameter()
				: directionColor(Vector3(1.0f,0.9f,0.9f))
				, ambient(Vector3(0.2f, 0.2f, 0.2f))
				, directionDir(Vector3(0.1f,-1.0f,0.1f))
			{}


			/**
			 * @brief JSON から読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
			{
				directionColor = file.GetVector3("directionColor", directionColor);
				ambient = file.GetVector3("ambient", ambient);
				directionDir = file.GetVector3("directionDir", directionDir);
			}
		};
	}
}