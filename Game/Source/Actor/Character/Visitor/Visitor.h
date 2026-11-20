/**
 * @file Visitor.h
 * @brief 美術館の訪問者(一般客)
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"
#include "Source/Util/ParameterManager.h"
#include "Source/Util/WorldTime.h"
#include "Object/VisitorParameter.h"
#include <string>


namespace app
{
	namespace actor
	{
		/**
		 * @brief 訪問者(一般客)
		 */
		class Visitor : public CharacterBase
		{
		public:
			/**
			 * @brief 訪問者の種類(見た目)
			 */
			enum class VisitorType : uint8_t
			{
				Karen,			// カレン
				OfficeMan,		// サラリーマン
				WomanAfro,		// アフロの女性
				WomanAsian,		// アジア系の女性
				WomanEuro,		// ヨーロッパ系の女性
				CoupleA,		// カップル(CouplePerson01)
				CoupleB,		// カップル(CouplePerson02)
				CoupleC,		// カップル(CouplePerson03)
				CoupleD,		// カップル(CouplePerson04)
				CoupleE,		// カップル(CouplePerson05)
				FamilyWomanA,	// 家族の大人(HumanoWoman01)
				FamilyWomanB,	// 家族の大人(HumanoWoman02)
				FamilyWomanC,	// 家族の大人(HumanoWoman03)
				FamilyWomanD,	// 家族の大人(HumanoWoman04)
				FamilyBoy,		// 家族の子供(LittleBoy)
				Student,		// 学生(YoungBoy)
				Max				// 最大数
			};

			/**
			 * @brief 訪問者のアニメーション
			 */
			enum class VisitorAnimation : uint8_t
			{
				Idle,	  // 待機
				Walk,	  // 歩行
				Talk,	  // 会話
				WalkTalk, // 歩きながら会話
				Max		  // 最大数
			};


		public:
			/**
			 * @param type 訪問者の種類(見た目)
			 */
			explicit Visitor(const VisitorType type);
			~Visitor() override;

			/** 初期化処理(一度だけ) */
			bool Start() override;
			/** 更新処理 */
			void Update() override;
			/** 時間の倍率(スローモーションで遅くなる) */
			float GetTimeScale() const override { return util::WorldTime::GetScale(); }

			/**
			 * @brief "Karen" などの名前を、見た目の種類にする(JSON で見た目を書くため)
			 * @param name 見た目の名前(VisitorType と同じ綴り)
			 * @param outType 見た目の種類
			 * @return 名前が見つかれば true
			 */
			static bool ToVisitorType(const std::string& name, VisitorType& outType);


		public:
			/**
			 * @brief 移動の入力を設定(グループが毎フレーム決める)
			 * @param direction 進む方向
			 * @param amount 移動量(0 なら止まる。1 で歩く速さ)
			 */
			void SetMoveInput(const Vector3& direction, const float amount)
			{
				m_moveDirection = direction;
				m_moveAmount = amount;
			}
			/**
			 * @brief 進む方向を取得
			 * @return 進む方向
			 */
			const Vector3& GetMoveDirection() const { return m_moveDirection; }
			/**
			 * @brief 移動量を取得
			 * @return 移動量
			 */
			float GetMoveAmount() const { return m_moveAmount; }
			/**
			 * @brief 話しているかを設定(グループが決める)
			 * @param isTalking 話しているなら true
			 */
			void SetTalking(const bool isTalking) { m_isTalking = isTalking; }
			/**
			 * @brief 話しているか
			 * @return 話しているなら true
			 */
			bool IsTalking() const { return m_isTalking; }
			/**
			 * @brief 止まっている時に向く方向を設定(美術品の方を向く)
			 * @param direction 向く方向(0 なら向きを変えない)
			 */
			void SetLookDirection(const Vector3& direction) { m_lookDirection = direction; }
			/**
			 * @brief 止まっている時に向く方向を取得
			 * @return 向く方向
			 */
			const Vector3& GetLookDirection() const { return m_lookDirection; }
			/**
			 * @brief 調整パラメーターを取得
			 * @return 種類ごとの JSON の値
			 */
			const VisitorParameter& GetParameter() { return m_parameter.Get(); }


		private:
			/** 訪問者(一般客)のパラメーター */
			util::Parameter<VisitorParameter> m_parameter;
			/** 訪問者の種類(見た目) */
			VisitorType m_type;
			/** 進む方向 */
			Vector3 m_moveDirection;
			/** 移動量 */
			float m_moveAmount;
			/** 話しているか */
			bool m_isTalking;
			/** 止まっている時に向く方向 */
			Vector3 m_lookDirection;
		};
	}
}