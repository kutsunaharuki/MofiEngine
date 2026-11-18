/**
 * @file GameCamera.h
 * @brief ゲームカメラクラス
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "Source/Util/ParameterManager.h"


namespace app
{
	namespace camera
	{
		/**
		 * @brief ゲームカメラの調整用パラメーター
		 * @details JSON : Assets/parameter/Camera/GameCamera.json(保存すると、ゲームを動かしたまま反映される)
		 */
		struct GameCameraParameter
		{
			/** 3人称のときの、回る速さ(1秒に回る角度。度) */
			float rotateSpeed = 102.0f;
			/** ファインダーを覗いているときの、回る速さ(1秒に回る角度。度) */
			float finderRotateSpeed = 48.0f;
			/** 3人称のときの、目の位置からカメラまでの距離 */
			float thirdPersonDistance = 250.0f;
			/** 目の高さ(足元からの高さ) */
			float eyeHeight = 120.0f;
			/** 壁に当たった時に止める、カメラの球の半径(壁にめり込んで向こうが見えないように) */
			float collisionRadius = 15.0f;
			/** 壁に押されても、目の位置からこれ以上は近づけない距離 */
			float minDistance = 30.0f;
			/** 壁が無くなった時に、元の距離へ戻る速さ(1秒に進む距離) */
			float returnSpeed = 400.0f;

			/**
			 * @brief JSON から読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
			{
				rotateSpeed = file.GetFloat("rotateSpeed", rotateSpeed);
				finderRotateSpeed = file.GetFloat("finderRotateSpeed", finderRotateSpeed);
				thirdPersonDistance = file.GetFloat("thirdPersonDistance", thirdPersonDistance);
				eyeHeight = file.GetFloat("eyeHeight", eyeHeight);
				collisionRadius = file.GetFloat("collisionRadius", collisionRadius);
				minDistance = file.GetFloat("minDistance", minDistance);
				returnSpeed = file.GetFloat("returnSpeed", returnSpeed);
			}
		};



		/**
		 * @brief ゲームカメラクラス
		 */
		class GameCamera
		{
		public:
			GameCamera();
			~GameCamera() = default;
			bool Start();
			void Update();

			/**
			 * @brief 1人称視点かどうかを設定する
			 * @param isFirstPerson 1人称視点ならtrue, falseで3人称
			 */
			void SetFirstPeerson(bool isFirstPerson);
			/**
			 * @brief ズームの比率を設定する(ズームしているほど、ファインダーをゆっくり回す)
			 * @param zoomRatio 今の画角 ÷ いちばん広い画角(ズームなしで 1、寄るほど小さい)
			 */
			void SetZoomRatio(const float zoomRatio) { m_zoomRatio = zoomRatio; }

			/**
			 * @brie 水平方向(XZ平面)の前方向を取得
			 * @return 前方向
			 * @details プレイヤーの移動に使う
			 */
			Vector3 GetForwardXZ() const;
			/**
			 * @brief 水平方向(XZ平面)の右方向を取得
			 * @return 右方向
			 * @details プレイヤーの移動に使う
			 */
			Vector3 GetRightXZ() const;

			/**
			 * @brief 追従対象を設定する
			 * @param target 追従対象のトランスフォーム
			 */
			void SetTarget(const core::TransformNode* target) { m_target = target; }


		private:
			/**
			 * @brief 1人称視点の計算を行う関数
			 * @return 視点
			 */
			Vector3 CalcEyePos();
			/**
			 * @brief 視線の向きを計算する
			 * @return 視線の向き
			 */
			Vector3 CalcForwardXZ() const;
			/**
			 * @brief スティック入力で回転させる
			 */
			void RotateCamera();
			/**
			 * @brief 3人称のカメラの距離を、壁や手すりに当たらない距離にする
			 * @param eyePos 目の位置(ここから後ろへ球を飛ばす)
			 * @param backward 目からカメラへの向き(長さ 1)
			 * @return カメラの距離
			 * @details 当たった時はすぐ縮めて、当たらなくなったら少しずつ戻す(急に跳ねないように)
			 */
			float CalcThirdPersonDistance(const Vector3& eyePos, const Vector3& backward);


		private:
			/** 調整用パラメーター */
			util::Parameter<GameCameraParameter> m_parameter;
			/** モデルレンダー */
			ModelRender* m_modelRender;
			/** 追従対象 */
			const core::TransformNode* m_target;
			/** 左右の向き(度) */
			float m_yawDeg;
			/** 上下の向き(度。正で上向き) */
			float m_pitchDeg;
			/** 直前の視点 */
			Vector3 m_prevEyePos;
			/** 1人称視点かどうか */
			bool m_isFirstPerson;
			/** ズームの比率(ズームなしで 1、寄るほど小さい) */
			float m_zoomRatio;
			/** 今の 3人称のカメラの距離(壁に押されると縮む。負なら、まだ決まっていない) */
			float m_currentDistance;
		};
	}
}

