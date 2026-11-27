/**
 * @file Boy.h
 * @brief 主人公(男の子)
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"
#include "Source/Util/ParameterManager.h"
#include "Source/Actor/Character/Boy/Object/BoyParameter.h"
#include "Source/Actor/Character/Boy/PhotoChance.h"


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
			Idle,                  // ファインダーを覗いていない時の待機
			Walk,				   // ファインダーを覗いていない時の歩行
			Run,                   // ファインダーを覗いていない時の走行
			PhotoIdle,		       // ファインダーを覗いている時の待機
			PhotoWalkForward,	   // ファインダーを覗いている時の前歩き
			PhotoWalkBack,		   // ファインダーを覗いている時の後ろ歩き
			PhotoWalkLeft,		   // ファインダーを覗いている時の左歩き
			PhotoWalkRight,		   // ファインダーを覗いている時の右歩き
			CrouchHide,            // しゃがんで止まっている
			CrouchWalk,            // しゃがんだまま歩く
			CrouchCamera,          // しゃがんでカメラを構える
			CrouchShoot,           // しゃがんで撮る
			LeanCameraLeft,        // 柱の左からのぞいてカメラを構える
			LeanCameraRight,       // 柱の右からのぞいてカメラを構える
			CrouchLeanCameraLeft,  // しゃがんで、柱の左からのぞいてカメラを構える
			CrouchLeanCameraRight, // しゃがんで、柱の右からのぞいてカメラを構える
			Max
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

			/** 更新処理 */
			void Update() override;


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
			 * @brief 柱のどちらからのぞけるかを設定(シーンが PhotoChance で調べて渡す)
			 * @param leanSide のぞく向き(のぞけなければ None)
			 * @details ファインダーを覗いて止まっている時に、この向きでのぞく
			 */
			void SetLeanSide(const EnLeanSide leanSide) { m_leanSide = leanSide; }
			/**
			 * @brief 柱のどちらからのぞけるか
			 * @return のぞく向き
			 */
			EnLeanSide GetLeanSide() const { return m_leanSide; }
			/**
			 * @brief 目(カメラ)の位置のずれを取得(しゃがむと下がる・のぞくと横にずれる)
			 * @return ずれ(ワールドの向き)。GameCamera に渡す
			 */
			const Vector3& GetEyeOffset() const { return m_eyeOffset; }
			/**
			 * @brief しゃがんでいるか
			 * @return しゃがんでいれば true
			 */
			bool IsCrouching() const;
			/**
			 * @brief 柱からのぞいているか
			 * @return のぞくステート(左・右)なら true
			 */
			bool IsLeaning() const;

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
			/**
			 * @brief 目立ちやすさの倍率
			 * @return 走っていれば runNoticeScale、それ以外は 1
			 */
			float GetNoticeScale() const override { return m_noticeScale; }
			/**
			 * @brief 悪党が見る高さ(足元から)
			 * @param defaultHeight ふつうの高さ
			 * @return しゃがんでいれば crouchVisionHeight(ふつうより低ければ)、それ以外は defaultHeight
			 */
			float GetVisionTargetHeight(const float defaultHeight) const override;
			/**
			 * @brief 悪党に見つかる状態か
			 * @return カメラを構えている(ファインダーを覗いている)時だけ true
			 */
			bool CanBeFound() const override { return m_isFinderActive; }
			/** 主人公と、手に持つカメラを描く(InGameScene から呼ぶので public) */
			void Render(RenderContext& rc) override;


		protected:
			/** 足音を 1 回鳴らすまでに歩く長さ(走ると速く進むので、足音も速くなる) */
			float GetFootstepStride() const override;


		private:
			/**
			 * @brief 手に持つカメラを、手のボーンの位置と向きに合わせる
			 * @param parameter 調整パラメーター
			 */
			void UpdateHandCamera(const BoyParameter& parameter);
			/**
			 * @brief 目(カメラ)の位置のずれを、しゃがみ・のぞきに合わせて少しずつ動かす
			 * @param parameter 調整パラメーター
			 */
			void UpdateEyeOffset(const BoyParameter& parameter);


		private:
			/** 調整用パラメーター */
			util::Parameter<BoyParameter> m_parameter;
			/** 移動の向きの基準にするカメラ */
			const camera::GameCamera* m_gameCamera;
			/** ファインダーを覗いているか */
			bool m_isFinderActive;
			/** 目立ちやすさの倍率 */
			float m_noticeScale;
			/** 柱のどちらからのぞけるか */
			EnLeanSide m_leanSide;
			/** 目(カメラ)の位置のずれ */
			Vector3 m_eyeOffset;
			/** しゃがんだ時に、悪党が見る高さ(パラメーターの値を Update で覚える) */
			float m_crouchVisionHeight;
			/** 手に持つカメラのモデル */
			ModelRender m_handCamera;
			/** カメラを持つ手のボーンの番号(見つからなければ -1) */
			int m_handBoneNo;
			/** 今探しているボーンの名前(JSON で変わったら探し直す) */
			std::string m_handBoneName;
			/** 手のひらの先の方のボーンの番号(見つからなければ -1) */
			int m_palmBoneNo;
			/** 今探している、手のひらの先の方のボーンの名前 */
			std::string m_palmBoneName;
		};
	}
}
