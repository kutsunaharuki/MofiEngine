/**
 * @file BoyParameter.h
 * @brief 主人公の調整パラメータ
 */
#pragma once
#include "Source/Util/ParameterManager.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 主人公の調整パラメーター
		 */
		struct BoyParameter
		{
			/** 歩く速さ */
			float walkSpeed;
			/** 走る速さ */
			float runSpeed;
			/** 向きを変える速さ(大きいほど早く向く) */
			float rotateSpeed;
			/** 重力 */
			float gravity;
			/** ファインダーを覗いている時の歩行速度 */
			float photoWalkSpeed;
			/** しゃがんで歩く速さ */
			float crouchWalkSpeed;
			/** しゃがんだ時に、目(カメラ)を下げる長さ */
			float crouchEyeDrop;
			/** しゃがんだ時に、悪党が見る高さ(足元から。低い物の陰に入ると見えなくなる) */
			float crouchVisionHeight;
			/** 柱のモデルの名前(モデルのパスにこの名前が入っている置物を、のぞける柱にする。ゲームを始めた時だけ読む) */
			std::string leanPillarName;
			/** 柱の表面から、これより近ければのぞける */
			float leanCheckDistance;
			/** 柱のはしが、画面の真ん中からこれより外に出ていたら、そちらからはのぞかない(画面の半分を 1 とした長さ。0.5 = 1/4) */
			float leanEdgeLimit;
			/** のぞいた時に、目(カメラ)を横にずらす長さ */
			float leanEyeOffset;
			/** 目(カメラ)を、しゃがんだ・のぞいた位置まで動かす速さ(大きいほど早い) */
			float eyeMoveSpeed;
			/** 初期位置 */
			Vector3 initialPosition;
			/** 更新されるスケール */
			Vector3 updateScale;
			/** 走っている時の目立ちやすさの倍率 */
			float runNoticeScale;
			/** 手に持つカメラをつけるボーンの名前(手首) */
			std::string cameraHandBone;
			/** 手のひらの先の方のボーンの名前(手首とこのボーンの真ん中を、手のひらの真ん中にする) */
			std::string cameraPalmBone;
			/** 手に持つカメラの、手からのずれ(主人公の向きで見る。x = 横、y = 上、z = 前) */
			Vector3 cameraOffset;
			/** 手に持つカメラの回転(度。主人公の向きに足す。x → y → z の順に回す) */
			Vector3 cameraRotation;
			/** 手に持つカメラの大きさ */
			float cameraScale;


			BoyParameter()
				: walkSpeed(150.0f)
				, runSpeed(400.0f)
				, rotateSpeed(15.0f)
				, gravity(1500.0f)
				, photoWalkSpeed(100.0f)
				, crouchWalkSpeed(80.0f)
				, crouchEyeDrop(50.0f)
				, crouchVisionHeight(35.0f)
				, leanPillarName("SM_Column01")
				, leanCheckDistance(60.0f)
				, leanEdgeLimit(0.5f)
				, leanEyeOffset(40.0f)
				, eyeMoveSpeed(8.0f)
				, initialPosition(Vector3::Zero)
				, updateScale(Vector3::One)
				, runNoticeScale(1.0f)
				, cameraHandBone("mixamorig:RightHand")
				, cameraPalmBone("mixamorig:RightHandMiddle1")
				, cameraOffset(Vector3::Zero)
				, cameraRotation(Vector3::Zero)
				, cameraScale(1.0f)
			{}


			/**
			 * @brief JSON から読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
			{
				walkSpeed = file.GetFloat("walkSpeed", walkSpeed);
				runSpeed = file.GetFloat("runSpeed", runSpeed);
				rotateSpeed = file.GetFloat("rotateSpeed", rotateSpeed);
				gravity = file.GetFloat("gravity", gravity);
				photoWalkSpeed = file.GetFloat("photoWalkSpeed", photoWalkSpeed);
				crouchWalkSpeed = file.GetFloat("crouchWalkSpeed", crouchWalkSpeed);
				crouchEyeDrop = file.GetFloat("crouchEyeDrop", crouchEyeDrop);
				crouchVisionHeight = file.GetFloat("crouchVisionHeight", crouchVisionHeight);
				leanPillarName = file.GetString("leanPillarName", leanPillarName);
				leanCheckDistance = file.GetFloat("leanCheckDistance", leanCheckDistance);
				leanEdgeLimit = file.GetFloat("leanEdgeLimit", leanEdgeLimit);
				leanEyeOffset = file.GetFloat("leanEyeOffset", leanEyeOffset);
				eyeMoveSpeed = file.GetFloat("eyeMoveSpeed", eyeMoveSpeed);
				initialPosition = file.GetVector3("initialPosition", initialPosition);
				updateScale = file.GetVector3("updateScale", updateScale);
				runNoticeScale = file.GetFloat("runNoticeScale", runNoticeScale);
				cameraHandBone = file.GetString("cameraHandBone", cameraHandBone);
				cameraPalmBone = file.GetString("cameraPalmBone", cameraPalmBone);
				cameraOffset = file.GetVector3("cameraOffset", cameraOffset);
				cameraRotation = file.GetVector3("cameraRotation", cameraRotation);
				cameraScale = file.GetFloat("cameraScale", cameraScale);
			}
		};
	}
}