/**
 * @file FinderParameter.h
 * @brief ファインダーのパラメーターを管理する
 */
#pragma once
#include "Source/Util/ParameterManager.h"


namespace app
{
	namespace camera
	{
		/**
		 * @brief ファインダーのパラメーター
		 */
		struct FinderParameter
		{
			/** ズームの速さ(1.0 がエンジンの標準の速さ) */
			float zoomSpeed = 1.0f;
			/** ピントを動かす速さ(1.0 がエンジンの標準の速さ) */
			float focusSpeed = 1.0f;
			/** 撮ってから、次に撮れるようになるまでの時間(秒) */
			float shotCoolTime = 1.0f;
			/** ピントが合う速さ(大きいほど早く合う) */
			float autoFocusSpeed = 8.0f;
			/** これより遠い被写体は無視する距離 */
			float autoFocusMaxDistance = 1000.0f;
			/**
			 * @brief 被写体の顔の半径(3D の大きさ)
			 * @details これを画面に写した大きさが、顔のまわりの丸の大きさになる(遠い・ズームで変わる)
			 */
			float subjectFaceRadius = 30.0f;
			/** 顔のまわりの丸の、半径の下限(ピクセル。遠すぎて点にならないように) */
			float subjectCircleMinRadius = 40.0f;
			/** 顔のまわりの丸の、半径の上限(ピクセル。近すぎて画面を覆わないように) */
			float subjectCircleMaxRadius = 300.0f;


			/**
			 * @brief JSON から読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
			{
				zoomSpeed = file.GetFloat("zoomSpeed", zoomSpeed);
				focusSpeed = file.GetFloat("focusSpeed", focusSpeed);
				shotCoolTime = file.GetFloat("shotCoolTime", shotCoolTime);
				autoFocusSpeed = file.GetFloat("autoFocusSpeed", autoFocusSpeed);
				autoFocusMaxDistance = file.GetFloat("autoFocusMaxDistance", autoFocusMaxDistance);
				subjectFaceRadius = file.GetFloat("subjectFaceRadius", subjectFaceRadius);
				subjectCircleMinRadius = file.GetFloat("subjectCircleMinRadius", subjectCircleMinRadius);
				subjectCircleMaxRadius = file.GetFloat("subjectCircleMaxRadius", subjectCircleMaxRadius);
			}
		};
	}
}
