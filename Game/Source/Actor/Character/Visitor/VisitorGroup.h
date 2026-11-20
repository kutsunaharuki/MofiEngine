/**
 * @file VisitorGroup.h
 * @brief 訪問者(一般客)の 1グループ
 */
#pragma once
#include "Source/Util/ParameterManager.h"
#include "Object/VisitorParameter.h"
#include "Visitor.h"
#include <memory>
#include <vector>
#include <random>


namespace app
{
	namespace ai
	{
		/** 前方宣言 */
		class Pathfinder;
		class NavGrid;
		class PathFollower;
	}


	namespace actor
	{
		/**
		 * @brief 組の種類
		 */
		enum class GroupType : uint8_t
		{
			Solo,		// 1 人の客(今までの 5 人)
			Student,	// 学生(1 人)
			Couple,		// カップル(2 人)
			Family,		// 家族(4 人／4 人＋ベビーカー／6 人)
			Max
		};


		/**
		 * @brief 1 組の顔ぶれ(VisitorManager が決めて、VisitorGroup に渡す)
		 */
		struct GroupComposition
		{
			/** 組の種類 */
			GroupType type = GroupType::Solo;
			/** メンバーの見た目(先頭が親。人数 = この数) */
			std::vector<Visitor::VisitorType> memberTypeList;
			/** ベビーカーを押しているか(家族だけ。先頭の親が押す) */
			bool hasStroller = false;
		};


		/**
		 * @brief 訪問者(一般客)の 1グループ
		 * @details グループが頭脳になって、メンバー(Visitor)に「どこへ動くか」「話すか」「どこを見るか」を渡す。
		 *          親は美術品へ道順を辿って歩き、ほかの人は「親の位置 + 自分の立ち位置」についていく。
		 *          着いたら、しばらく全員で美術品を見てから、次の美術品へ向かう
		 */
		class VisitorGroup : public Noncopyable
		{
		public:
			VisitorGroup();
			~VisitorGroup();

			/** 初期化処理 */
			void Initialize();
			/** 更新処理 */
			void Update();
			/** 描画処理 */
			void Render(RenderContext& rc);


		public:
			/**
			 * @brief 経路探索に使うものを設定
			 * @param pathfinder 道順を求めるもの(nullptr なら歩かずに立っているだけ)
			 * @param navGrid 地図
			 * @details Initialize より前に呼ぶ
			 */
			void SetNavigation(const ai::Pathfinder* pathfinder, const ai::NavGrid* navGrid);
			/**
			 * @brief 見に行く美術品の位置の一覧を設定
			 * @param targetList 美術品の位置の一覧(空なら歩かずに立っているだけ)
			 */
			void SetTargetList(const std::vector<Vector3>& targetList) { m_targetList = targetList; }
			/**
			 * @brief 出す場所と顔ぶれを設定(VisitorManager が組ごとに決める)
			 * @param position 出す場所(親の位置)
			 * @param composition 顔ぶれ(組の種類・メンバーの見た目・ベビーカー)
			 * @details Initialize より前に呼ぶ。呼ばなければ JSON の groupPosition と memberCount で、今までの 5 人からランダムに作る(試し用)
			 */
			void SetSpawn(const Vector3& position, const GroupComposition& composition);
			/**
			 * @brief 人数を取得
			 * @return メンバーの数(親も入る)
			 */
			int GetMemberCount() const { return static_cast<int>(m_memberList.size()); }
			/**
			 * @brief 顔ぶれを取得
			 * @return 組の種類・メンバーの見た目・ベビーカー(SetSpawn を呼んでいなければ、種類は Solo のまま)
			 */
			const GroupComposition& GetComposition() const { return m_composition; }


		private:
			/**
			 * @brief グループの今の行動
			 */
			enum class GroupState : uint8_t
			{
				Look,	// 立ち止まって美術品を見ている
				Move,	// 次の美術品へ歩いている
			};


		private:
			/**
			 * @brief 全員の立ち位置を決める
			 */
			void MakeSlotList();
			/**
			 * @brief 扇形の中に、1人の立ち位置を決める
			 * @param quadrant 扇形の番号(0:右上,1:左上,2:左下,3:右下)
			 * @param placedList もう決まった立ち位置の一覧(近すぎないかを調べる)
			 */
			Vector3 MakeSlot(const int quadrant, const std::vector<Vector3>& placedList);
			/**
			 * @brief 決まった立ち位置全てを走査して最低の間隔以上離れているか
			 * @param slot グループの中での自分の立ち位置
			 * @param placedList もう決まった立ち位置の一覧
			 * @param minSpacing メンバー同士の最低の間隔
			 */
			bool IsFarFromOthers(const Vector3& slot, const std::vector<Vector3>& placedList, const float minSpacing) const;
			/**
			 * @brief 立ち位置に、メンバーを置く
			 * @param isStarted Start の後なら true(Warp で置く。Start の前は SetPosition)
			 */
			void PlaceMembers(const bool isStarted);
			/**
			 * @brief 並び方の値が変わったか(ImGui で変えたら並び直す)
			 * @param parameter 今のパラメーター
			 */
			bool IsLayoutChanged(const VisitorGroupParameter& parameter) const;
			/**
			 * @brief 並び方の値が変わった時に並び直す
			 * @param parameter 今のパラメーター
			 * @details 位置が変わった時だけワープする。半径などが変わった時は、新しい立ち位置へ歩いていく
			 */
			void ApplyLayoutChange(const VisitorGroupParameter& parameter);
			/**
			 * @brief 出す場所(親の位置)を取得
			 * @param parameter 今のパラメーター
			 * @return SetSpawn で決めた場所。決めていなければ JSON の groupPosition
			 */
			const Vector3& GetSpawnPosition(const VisitorGroupParameter& parameter) const;


		private:
			/**
			 * @brief 親の行動を更新する(見る → 歩く → 見る … をくり返す)
			 * @param deltaTime 1フレームの経過時間(秒)
			 */
			void UpdateLeader(const float deltaTime);
			/**
			 * @brief ほかの人が、親の位置 + 自分の立ち位置へついていく
			 */
			void UpdateFollowers();
			/**
			 * @brief 誰が話すかを、一定の間隔で選び直す
			 * @param deltaTime 1フレームの経過時間(秒)
			 */
			void UpdateTalk(const float deltaTime);
			/**
			 * @brief 次の美術品を選んで、親の道順を決める
			 * @return 道順が見つかれば true
			 */
			bool StartMoveToNextTarget();
			/**
			 * @brief 立ち止まって美術品を見始める(見る時間をランダムに決める)
			 */
			void StartLook();
			/**
			 * @brief ベビーカーのモデルを作る(ベビーカーがある組だけ)
			 */
			void InitStroller();
			/**
			 * @brief ベビーカーを、親の前に置く
			 * @details 親の向きに合わせて回し、親の前に少し離して置く(毎フレーム、親が動いた後に呼ぶ)
			 */
			void UpdateStroller();
			/**
			 * @brief 親を取得
			 * @return 親
			 */
			Visitor& GetLeader() { return *m_memberList[m_leaderIndex]; }


		private:
			/** メンバー(親も入る) */
			std::vector<std::unique_ptr<Visitor>> m_memberList;
			/** 親から見た立ち位置 */
			std::vector<Vector3> m_slotList;
			/** 親の番号 */
			int m_leaderIndex;
			/** 調整パラメーター(単位はグループ) */
			util::Parameter<VisitorGroupParameter> m_parameter;
			/** 最後に並べた時のパラメーター */
			VisitorGroupParameter m_layoutParameter;
			/** 乱数 */
			std::mt19937 m_random;

			/** 道順を求める(シーンのものを借りる) */
			const ai::Pathfinder* m_pathfinder;
			/** 親の道順を辿る */
			std::unique_ptr<ai::PathFollower> m_pathFollower;
			/** 見に行く美術品の位置の一覧 */
			std::vector<Vector3> m_targetList;
			/** 今見ている(向かっている)美術品の番号(まだ無ければ -1) */
			int m_targetIndex;
			/** グループの今の行動 */
			GroupState m_groupState;
			/** 立ち止まって見る残りの時間(秒) */
			float m_lookTimer;
			/** 誰が話すかを選び直すまでの残りの時間(秒) */
			float m_talkTimer;
			/** 出す場所(SetSpawn で決めた時だけ使う) */
			Vector3 m_spawnPosition;
			/** 顔ぶれ(SetSpawn で決めた時だけ使う) */
			GroupComposition m_composition;
			/** SetSpawn で、出す場所と顔ぶれを決めたか */
			bool m_hasSpawn;
			/** ベビーカーのモデル(ベビーカーが無い組は nullptr) */
			std::unique_ptr<ModelRender> m_strollerRender;
		};
	}
}
