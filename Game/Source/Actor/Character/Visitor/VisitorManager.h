/**
 * @file VisitorManager.h
 * @brief 一般客の管理場所
 */
#pragma once
#include "VisitorGroup.h"
#include <array>
#include <memory>
#include <random>
#include <vector>



namespace app
{
	namespace ai
	{
		/** 前方宣言 */
		class Pathfinder;
		class NavGrid;
	}


	namespace actor
	{
		/** 前方宣言 */
		class VisitorGroup;


		/**
		 * @brief 一般客の管理場所
		 * @details JSON を読んで、1 人の客とグループを何組も作り、まとめて更新・描画する。
		 *          1 人の客も「メンバー 1 人の VisitorGroup」なので、動き(移動 → 鑑賞 → 次の目的地)はグループと同じ。
		 *          一覧だけ分けて持つ(数を数える・片方だけ止める・デバッグ表示を分ける時のため)。
		 *          JSON : Assets/parameter/Actor/Character/Visitor/VisitorSpawn.json
		 */
		class VisitorManager : public Noncopyable
		{
		public:
			VisitorManager();
			~VisitorManager();

			/**
			 * @brief 初期化(JSON の数だけ、1 人の客とグループを作る)
			 * @param spawnFilePath 何組出すかの JSON のパス
			 * @param pathfinder 道順を求めるもの(シーンのものを借りる。nullptr なら立っているだけ)
			 * @param navGrid 地図(出す場所を、通れるマスから選ぶのに使う)
			 * @param targetList 見に行く美術品の位置の一覧
			 */
			void Initialize(
				const char* spawnFilePath,
				const ai::Pathfinder* pathfinder,
				const ai::NavGrid* navGrid,
				const std::vector<Vector3>& targetList);
			/** 更新処理(全部の組) */
			void Update();
			/** 描画処理(全部の組) */
			void Render(RenderContext& rc);


		public:
			/**
			 * @brief 1 人の客の数を取得
			 * @return 1 人の客の数
			 */
			int GetSoloCount() const { return static_cast<int>(m_soloList.size()); }
			/**
			 * @brief グループの数を取得
			 * @return グループの数
			 */
			int GetGroupCount() const { return static_cast<int>(m_groupList.size()); }


		private:
			/**
			 * @brief 人数の候補 1つ(家族なら 4人 / 4人 + ベビーカー / 6人)
			 */
			struct Variation
			{
				/** 歩く人数 */
				int memberCount = 1;
				/** ベビーカーを押すか */
				bool hasStroller = false;
				/** 選ばれやすさ */
				float weight = 1.0f;
			};

			/**
			 * @brief 組の種類 1つの設定
			 */
			struct GroupTypeSetting
			{
				/** 人数の候補 */
				std::vector<Variation> variationList;
				/** 見た目の候補(大人・子供を分けない種類) */
				std::vector<Visitor::VisitorType> appearanceList;
				/** 大人の見た目の候補(家族) */
				std::vector<Visitor::VisitorType> adultList;
				/** 子供の見た目の候補(家族) */
				std::vector<Visitor::VisitorType> childList;
				/** 大人の数(人数) */
				int adultCount = 0;
				/** 同じ見た目を選ばないか */
				bool isNoDuplicate = false;
			};


		private:
			/**
			 * @brief 組の種類ごとの設定を読む
			 * @param filePath 組の種類の設定の JSON のパス(VisitorGroupType.json)
			 * @details "Solo / "Student / "Couple / "Family" ごとに人数の候補と見た目の候補を読む
			 *          JSON に書いていない種類は、空のまま(その種類の組は作られない)
			 */
			void LoadGroupTypeSetting(const char* filePath);
			/**
			 * @brief 1組作って人数で一覧に入れる
			 * @param type 組の種類
			 * @details 人数を決める → 誰が来るか決める → 置く場所を決めて作る
			 *          1人なら m_soloListに 2人以上なら m_groupList に入れる
			 */
			void CreateGroup(const GroupType type);
			/**
			 * @brief 何人で来るかをくじ引きで決める
			 * @param setting 組の種類の設定
			 * @return 決まった人数の設定(設定がなければ nullptr)
			 * @details weight が大きいほど当たりやすい
			 *          1つしか無ければくじは引かない
			 */
			const Variation* PickVariation(const GroupTypeSetting& setting);
			/**
			 * @brief 誰が来るか(見た目)を決める
			 * @param type 組の種類
			 * @param setting 組の種類の設定
			 * @param 決まった人数の設定
			 * @return 来る人の見た目とベビーカーがあるか(見た目が 1つも無ければ空)
			 * @details 家族は 1人目と 2人目を大人で残りを子供にする(1人目が親で、ベビーカーを押す)
			 *          他の種類は見た目の中から人数分選ぶ
			 */
			GroupComposition MakeComposition(
				const GroupType type,
				const GroupTypeSetting& setting, 
				const Variation& variation
			);
			/**
			 * @brief 見た目を選んでリストの後ろに足す
			 * @param candidateList 選べる見た目
			 * @param count 何人選ぶか
			 * @param isNoDuplicate true なら同じ見た目を 2回選ばない
			 * @param outList 足す先(前に入っている人は消さない)
			 * @details 同じ見た目を選ばない時は、トランプのように混ぜてから上から取る
			 *          見た目が足りなくなったら同じ見た目も選ぶ
			 */
			void PickTypes(
				const std::vector<Visitor::VisitorType>& candidateList,
				const int count, 
				const bool isNoDuplicate, 
				std::vector<Visitor::VisitorType>& outList
			);
			/**
			 * @brief 出す場所を決める
			 * @return 出す場所(美術品の近くの通れるマス。ほかの組から離れた所)
			 */
			Vector3 FindSpawnPosition();
			/**
			 * @brief 位置の近くで、通れるマスを探す
			 * @param position 探し始める位置
			 * @param outPosition 見つかったマスの中心
			 * @return 見つかれば true
			 */
			bool FindOpenPositionNear(const Vector3& position, Vector3& outPosition) const;
			/**
			 * @brief ほかの組から十分に離れているか
			 * @param position 調べる位置
			 * @return 離れていれば true
			 */
			bool IsFarFromSpawned(const Vector3& position) const;


		private:
			/** 1 人の客 */
			std::vector<std::unique_ptr<VisitorGroup>> m_soloList;
			/** グループ(2 人以上) */
			std::vector<std::unique_ptr<VisitorGroup>> m_groupList;
			/** 組の種類ごとの設定 */
			std::array<GroupTypeSetting, static_cast<size_t>(GroupType::Max)> m_groupTypeSettingList;

			/** 道順を求める(シーンのものを借りる) */
			const ai::Pathfinder* m_pathfinder;
			/** 地図(シーンのものを借りる) */
			const ai::NavGrid* m_navGrid;
			/** 見に行く美術品の位置の一覧 */
			std::vector<Vector3> m_targetList;
			/** もう出した場所(組どうしが重ならないように) */
			std::vector<Vector3> m_spawnedPositionList;
			/** 組どうしの最低の間隔 */
			float m_spawnMinDistance;
			/** 通れるマスを探す範囲(マスの数) */
			int m_spawnSearchCell;
			/** 出す場所が決められない時の場所(美術品も地図も無い時など) */
			Vector3 m_fallbackPosition;
			/** 乱数 */
			std::mt19937 m_random;
		};
	}
}