/**
 * @file Boy.h
 * @brief 主人公(男の子)
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"
#include "Source/Util/ParameterManager.h"


namespace app
{
	/** 前方宣言 */
	namespace camera
	{
		class GameCamera;
	}


	namespace actor
	{
		/**
		 * @brief 主人公のアニメーション(ModelData のアニメーションの順番と合わせる)
		 */
		enum class EnBoyAnimation : uint8_t
		{
			Idle,
			Walk,
			Run,
			Jump,
			Num
		};


		/**
		 * @brief 主人公の調整用パラメーター
		 * @details JSON : Assets/parameter/Actor/Boy.json(保存すると、ゲームを動かしたまま反映される)
		 */
		struct BoyParameter
		{
			/** 歩く速さ */
			float walkSpeed = 150.0f;
			/** 走る速さ */
			float runSpeed = 400.0f;
			/** 空中で動ける速さ */
			float airMoveSpeed = 250.0f;
			/** ジャンプした瞬間の上向きの速度 */
			float jumpSpeed = 600.0f;
			/** 重力加速度 */
			float gravity = 1500.0f;
			/** 向きを変える速さ(大きいほど早く向く) */
			float rotateSpeed = 15.0f;
			/** ジャンプしてから、着地の判定を始めるまでの時間(秒) */
			float jumpLandingCheckTime = 0.1f;

			/**
			 * @brief JSON から読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
			{
				walkSpeed = file.GetFloat("walkSpeed", walkSpeed);
				runSpeed = file.GetFloat("runSpeed", runSpeed);
				airMoveSpeed = file.GetFloat("airMoveSpeed", airMoveSpeed);
				jumpSpeed = file.GetFloat("jumpSpeed", jumpSpeed);
				gravity = file.GetFloat("gravity", gravity);
				rotateSpeed = file.GetFloat("rotateSpeed", rotateSpeed);
				jumpLandingCheckTime = file.GetFloat("jumpLandingCheckTime", jumpLandingCheckTime);
			}
		};


		/**
		 * @brief 主人公(男の子)
		 * @details 行動はステートパターン(BoyStateMachine と、各ステート)で決める。
		 *          Boy 自身は、ステートが使う情報(カメラ・ファインダーの状態)を持つだけ
		 */
		class Boy : public CharacterBase
		{
		public:
			Boy();
			~Boy() override;

			/** 初期化処理 */
			bool Start() override;


		public:
			/**
			 * @brief 移動の向きの基準にするカメラを設定
			 * @param camera ゲームカメラ
			 * @details スティックの上が、カメラの向いている方向になる
			 */
			void SetGameCamera(const camera::GameCamera* camera) { m_gameCamera = camera; }
			/**
			 * @brief 移動の向きの基準にするカメラを取得
			 * @return ゲームカメラ(設定されていなければ nullptr)
			 */
			const camera::GameCamera* GetGameCamera() const { return m_gameCamera; }

			/**
			 * @brief ファインダーを覗いているかを設定
			 * @param isFinderActive 覗いているか
			 * @details 覗いている間は、撮影ステートになる
			 */
			void SetIsFinderActive(const bool isFinderActive) { m_isFinderActive = isFinderActive; }
			/**
			 * @brief ファインダーを覗いているか
			 * @return 覗いているか
			 */
			bool IsFinderActive() const { return m_isFinderActive; }

			/**
			 * @brief 調整用パラメーターを取得
			 * @return パラメーター(JSON が変わっていれば、読み直してから返す)
			 */
			const BoyParameter& GetParameter() { return m_parameter.Get(); }

			/**
			 * @brief 今のステートが、指定したステートか
			 * @param stateId ステートのID(BoyIdleState::ID() など)
			 * @return 同じなら true
			 * @details DebugScene で、今のステートを表示するのに使う
			 */
			bool IsState(const uint32_t stateId) const;


		private:
			/** 調整用パラメーター */
			util::Parameter<BoyParameter> m_parameter;
			/** 移動の向きの基準にするカメラ */
			const camera::GameCamera* m_gameCamera;
			/** ファインダーを覗いているか */
			bool m_isFinderActive;
		};
	}
}
