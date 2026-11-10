/**
 * @file EnemyBase.h
 * @brief 敵の親となるクラス
 */
#pragma once
#include "Source/Core/TransformNode.h"
#include "Source/Camera/PhotoSubject.h"
#include "Source/Actor/Character/CharacterBase.h"
#include <cstdint>
#include <memory>
#include <vector>
#include "Source/Stage/HidePoint.h"
#include "Source/Util/ParameterManager.h"
#include "Object/EnemyParameter.h"
#include "Source/AI/Vision.h"


namespace app
{
	namespace ai
	{
		/** 前方宣言 */
		class NavGrid;
		class Pathfinder;
	}


	namespace actor
	{
		/** 前方宣言 */
		class EnemyStateMachine;
		class EnemyDestinationMode;
		class EnemyAlertMode;


		/**
		 * @brief 敵の親となるクラス
		 * @details 敵の種類でモデルとパラメーターを切り替える
		 * @details 今回は多重継承でCharacterBase と IPhotoSubject を継承している(IPhotoSubjectがインターフェースなので安全)
		 */
		class EnemyBase : public CharacterBase, public camera::IPhotoSubject
		{
		public:
			// 敵の種類
			enum class EnemyType : uint8_t
			{
				BigMan,   // 大男
				RatMan,   // ねずみ男
				Max       // 最大数
			};


			// 敵のアニメーション
			enum class EnemyAnimation : uint8_t
			{
				Idle,      // 待機
				Walk,      // 歩く
				Alert,     // 警戒
				Steal,     // 盗む
				Max        // 最大数
			};


		public:
			explicit EnemyBase(EnemyType type);
			~EnemyBase() override;

			/** 初期化処理 */
			bool Start() override;
			/** 更新処理 */
			void Update() override;


		public:
			/**
			 * @brief 敵の種類を取得
			 * @return 敵の種類
			 */
			EnemyType GetEnemyType() const { return m_enemyType; }
			
			/**
			 * @brief 敵に被写体のピントを合わせる位置を取得
			 * @return 被写体のピントを合わせる位置(定義した場所で体の中心や顔のあたりの位置が変わる)
			 */
			Vector3 GetFocusPosition() const override;
			/**
			 * @brief 敵の当たり判定を取得(レイが敵のキャラクターコントローラーに当たったかを比べる)
			 * @return キャラクターコントローラーの当たり判定
			 */
			const btCollisionObject* GetSubjectCollisionObject() const override { return GetCollisionObject(); }

			/**
			 * @brief 敵のステートマシンを取得
			 * @return 敵のステートマシン
			 * @detalis 敵のステートマシンは CharacterStateMachine が所持しているので変換して返す
			 */
			EnemyStateMachine* GetEnemyStateMachine();

			/**
			 * @brief 調整パラメーターを取得
			 * @return 調整パラメーター(JSON が変わっていれば、読み直してから返す)
			 * @details Get() が読み直しで中身を書き換えるので、const 関数にはできない
			 */
			const EnemyParameter& GetParameter() { return m_enemyParameter.Get(); }

			/**
			 * @brief 経路探索に使うものを設定
			 * @param pathfinder 道順を求める
			 * @parma navGrid 地図
			 */
			void SetNavigation(const ai::Pathfinder* pathfinder, const ai::NavGrid* navGrid);
			/**
			 * @brief 盗みに行く美術品の位置の一覧を設定
			 * @param targetList 美術品の位置の一覧
			 * @details Start より前に呼んでも、後に呼んでもよい(目的地モードは Start で作られるので、それまでは預かっておく)
			 */
			void SetTargetList(const std::vector<Vector3>& targetList);
			/**
			 * @brief 隠れ場所の一覧を設定
			 * @param hidePointList 隠れ場所の一覧
			 * @details 警戒モードができるまで預かっておく。警戒モードができたらすぐ渡す
			 */
			void SetHidePointList(const std::vector<stage::HidePoint>& hidePointList);
			/**
			 * @brief 隠れ場所の一覧を取得
			 * @return 隠れ場所の一覧
			 */
			const std::vector<stage::HidePoint>& GetHidePointList() const { return m_hidePointList; }

			/**
			 * @brief 目的地モードを取得(デバッグ表示用)
			 * @return 目的地モード(Start 前は nullptr)
			 */
			const EnemyDestinationMode* GetDestinationMode() const { return m_destinationMode.get(); }
			/**
			 * @brief 警戒モードを取得(デバッグ表示用)
			 * @return 警戒モード(Start 前は nullptr)
			 */
			const EnemyAlertMode* GetAlertMode() const { return m_alertMode.get(); }
			/**
			 * @brief 警戒を開始
			 * @details Start 前に呼ばれた時は何もしない
			 */
			void StartAlert();
			/**
			 * @brief 警戒をやめる
			 * @details 警戒していない時・Start 前に呼ばれた時は何もしない
			 */
			void EndAlert();
			/**
			 * @brief 見る相手(主人公)を設定
			 * @param player 主人公
			 */
			void SetPlayer(const CharacterBase* player) { m_player = player; }
			/**
			 * @brief 視線(デバッグ表示用)
			 * @return ビジョン
			 */
			const ai::Vision& GetVision() const { return m_vision; }


		private:
			/**
			 * @brief 視線の更新
			 * @param setting ビジョンの構造体
			 * @param deltaTime 計測時間(秒)
			 */
			void UpdateVision(const ai::VisionSetting& setting, const float deltaTime);


		private:
			/** 敵の種類 */
			EnemyType m_enemyType;
			/** 敵のパラメーター情報 */
			util::Parameter<EnemyParameter> m_enemyParameter;
			/** ピントを合わせる高さ */
			float m_focusHeight;

			/** 道順を求める */
			const ai::Pathfinder* m_pathfinder;
			/** 地図 */
			const ai::NavGrid* m_navGrid;

			/** 視線 */
			ai::Vision m_vision;
			/** 見る相手(主人公) */
			const CharacterBase* m_player;

			/**
			 * 目的地モード(美術品を盗みに行く頭)
			 * ※ ステートマシンを借りるので、ステートマシン(CharacterBase が持つ)より先に消える必要がある。
			 *    派生クラスのメンバーは基底クラスより先に消えるので大丈夫
			 */
			std::unique_ptr<EnemyDestinationMode> m_destinationMode;
			/**
			 * 警戒モード(隠れていそうな位置や以前隠れていた場所などを見て回る頭)
			 * ※ 目的地モードと同様にステートマシンより先に消える必要がある
			 */
			std::unique_ptr<EnemyAlertMode> m_alertMode;
			
			/** 美術品の位置の一覧(目的地モードができるまで預かる) */
			std::vector<Vector3> m_targetList;
			/** 隠れ場所の一覧(警戒モードができるまで預かる) */
			std::vector<stage::HidePoint> m_hidePointList;
		};
	}
}