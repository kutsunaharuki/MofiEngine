/**
 * @file StageLayout.h
 * @brief ステージの構造(階・区画・階段)
 */
#pragma once
#include <string>
#include <vector>


namespace app
{
	namespace stage
	{
		/**
		 * @brief 区画(U字の廊下の1本や、部屋など)
		 */
		struct StageArea
		{
			/** 名前 */
			std::string name;
			/** 箱の小さい方の角 */
			Vector3 min;
			/** 箱の大きい方の角 */
			Vector3 max;

			/**
			 * @brief 位置が箱の中に入っているか
			 * @param position 位置
			 * @return 入っていれば true
			 */
			bool Contains(const Vector3& position) const
			{
				return position.x >= min.x && position.x <= max.x
					&& position.y >= min.y && position.y <= max.y
					&& position.z >= min.z && position.z <= max.z;
			}
		};


		/**
		 * @brief 階
		 */
		struct StageFloor
		{
			/** 名前("1F" など) */
			std::string name;
			/** 床の高さ */
			float height;
			/** 区画の一覧 */
			std::vector<StageArea> areaList;
		};


		/**
		 * @brief 階段(2つの階をつなぐ)
		 */
		struct StageStairs
		{
			/** 名前 */
			std::string name;
			/** 下の階の名前 */
			std::string from;
			/** 上の階の名前 */
			std::string to;
			/** 箱の小さい方の角 */
			Vector3 min;
			/** 箱の大きい方の角 */
			Vector3 max;
		};




		/****************************************************/


		/**
		 * @brief ステージの構造(階・区画・階段)
		 * @details JSON に、階ごとの床の高さと、区画(箱)と、階段のつながりを書いておく。
		 *          JSON の例:
		 *          { "floors": [ { "name": "1F", "height": 0,
		 *                          "areas": [ { "name": "Hall", "min": [x, y, z], "max": [x, y, z] } ] } ],
		 *            "stairs": [ { "name": "EastStairs", "from": "1F", "to": "2F", "min": [...], "max": [...] } ] }
		 */
		class StageLayout
		{
		public:
			StageLayout();
			~StageLayout();

			/**
			 * @brief JSON を読み込む
			 * @param filePath JSON のパス
			 * @return 読み込めたら true
			 * @details 階は床の低い順に並べ直す
			 */
			bool Load(const std::string& filePath);


		public:
			/**
			 * @brief 高さから、その高さがどの階かを求める
			 * @param height 高さ(y)
			 * @return 階の番号(1F が 0。階が無ければ -1)
			 * @details 床が高さ以下の階のうち、一番上の階。一番下の階より低ければ、一番下の階にする
			 */
			int FindFloorIndex(const float height) const;
			/**
			 * @brief 位置が入っている区画を探す
			 * @param position 位置
			 * @return 区画(どこにも入っていなければ nullptr)
			 */
			const StageArea* FindArea(const Vector3& position) const;

			/**
			 * @brief 階の数を取得
			 * @return 階の数
			 */
			int GetFloorCount() const { return static_cast<int>(m_floorList.size()); }
			/**
			 * @brief 階を取得
			 * @param index 階の番号
			 * @return 階
			 */
			const StageFloor& GetFloor(const int index) const { return m_floorList[index]; }
			/**
			 * @brief 階段の一覧を取得
			 * @return 階段の一覧
			 */
			const std::vector<StageStairs>& GetStairsList() const { return m_stairsList; }


		private:
			/** 階の一覧(床の低い順) */
			std::vector<StageFloor> m_floorList;
			/** 階段の一覧 */
			std::vector<StageStairs> m_stairsList;
		};
	}
}
