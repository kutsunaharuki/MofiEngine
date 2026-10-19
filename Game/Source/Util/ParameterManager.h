/**
 * @file ParameterManager.h
 * @brief 調整用のパラメーター(JSON)の管理とホットリロード
 * @details 移動の速さやクールタイムなど、何度も調整する値を JSON ファイルに書いておく。
 *          K2_DEBUG の構成(Debug / Preview)では、JSON を保存すると、ゲームを動かしたまま値が変わる。
 *          ImGui の画面からも値を変えられて、そのまま JSON に保存できる(DebugScene で使う)
 *
 * 使い方
 *   1. 値をまとめた構造体を作り、JSON から読む Read 関数を書く
 *        struct BoyParameter
 *        {
 *            float walkSpeed = 150.0f;
 *            void Read(const util::ParameterFile& file) { walkSpeed = file.GetFloat("walkSpeed", walkSpeed); }
 *        };
 *   2. Parameter<構造体> をメンバーに持ち、Initialize で JSON のパスを渡す
 *        util::Parameter<BoyParameter> m_parameter;
 *        m_parameter.Initialize("Assets/parameter/Actor/Boy.json");
 *   3. 使うときに Get() を呼ぶ(JSON が変わっていれば、ここで読み直される)
 *        m_parameter.Get().walkSpeed
 */
#pragma once
#include "nlohmann/json.hpp"
#include <ctime>
#include <memory>
#include <string>
#include <vector>


#ifdef K2_DEBUG
#define APP_ENABLE_PARAMETER_HOTRELOAD
#endif


namespace app
{
	namespace util
	{
		/**
		 * @brief 1つの JSON ファイルのパラメーター
		 */
		class ParameterFile : public Noncopyable
		{
		public:
			explicit ParameterFile(const std::string& filePath);
			~ParameterFile();


		public:
			/**
			 * @brief ファイルから読み込む
			 * @return 読み込めたかどうか(失敗したときは、今の値のまま)
			 */
			bool Load();
			/**
			 * @brief ファイルに保存する
			 * @return 保存できたかどうか
			 */
			bool Save();
			/**
			 * @brief ファイルが保存し直されていたら、読み込み直す
			 * @return 読み込み直したかどうか
			 */
			bool ReloadIfModified();

			/**
			 * @brief 値を変えたことを知らせる
			 * @details ImGui などから JSON を直接書き換えたときに呼ぶ。使う側が読み直すようになる
			 */
			void MarkChanged();


		public:
			/** float を取得(無ければ defaultValue) */
			float GetFloat(const char* key, const float defaultValue) const;
			/** int を取得(無ければ defaultValue) */
			int GetInt(const char* key, const int defaultValue) const;
			/** bool を取得(無ければ defaultValue) */
			bool GetBool(const char* key, const bool defaultValue) const;
			/** Vector3 を取得(無ければ defaultValue) */
			Vector3 GetVector3(const char* key, const Vector3& defaultValue) const;

			/**
			 * @brief JSON を取得
			 * @return JSON(ImGui で書き換えるときに使う)
			 */
			nlohmann::json& GetJson() { return m_json; }
			/**
			 * @brief ファイルのパスを取得
			 * @return ファイルのパス
			 */
			const std::string& GetFilePath() const { return m_filePath; }
			/**
			 * @brief 値が変わった回数を取得
			 * @return 変わった回数(使う側は、これが変わったら読み直す)
			 */
			uint32_t GetVersion() const { return m_version; }
			/**
			 * @brief 保存していない変更があるか
			 * @return 変更があれば true
			 */
			bool IsDirty() const { return m_isDirty; }


		private:
			/** ファイルのパス */
			std::string m_filePath;
			/** JSON */
			nlohmann::json m_json;
			/** ファイルの最終更新日時 */
			time_t m_lastWriteTime;
			/** 値が変わった回数 */
			uint32_t m_version;
			/** 保存していない変更があるか */
			bool m_isDirty;
		};




		/*****************************************************/


		/**
		 * @brief パラメーターファイルをまとめて管理するクラス
		 * @details 同じパスのファイルは1つだけ読み込んで、使い回す
		 */
		class ParameterManager : public Noncopyable
		{
		public:
			/** インスタンスを作成する */
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new ParameterManager;
				}
			}
			/** インスタンスを取得する */
			static ParameterManager* GetInstance() { return m_instance; }
			/** インスタンスを破棄する */
			static void DestroyInstance()
			{
				if (m_instance != nullptr)
				{
					delete m_instance;
					m_instance = nullptr;
				}
			}


		public:
			/**
			 * @brief 更新処理(ホットリロードの確認)
			 * @details 毎フレーム呼ぶ。K2_DEBUG の構成でだけ、ファイルの保存を確認する
			 */
			void Update();

			/**
			 * @brief パラメーターファイルを取得する(まだなら読み込む)
			 * @param filePath JSON ファイルのパス
			 * @return パラメーターファイル(所有は ParameterManager が持つ)
			 */
			ParameterFile* Load(const char* filePath);

			/**
			 * @brief 読み込んだ全てのパラメーターを、ImGui で編集する画面を出す
			 * @details 数値はドラッグで、bool はチェックボックスで変えられる。「保存」で JSON に書き込む
			 */
			void DrawImGui();


		private:
			ParameterManager();
			~ParameterManager();


		private:
			/** シングルトンインスタンス */
			static ParameterManager* m_instance;
			/** 読み込んだパラメーターファイル */
			std::vector<std::unique_ptr<ParameterFile>> m_fileList;
			/** ファイルの更新を確認するまでの残り時間 */
			float m_checkTimer;
		};




		/*****************************************************/


		/**
		 * @brief パラメーターを構造体で使うためのクラス
		 * @tparam T パラメーターの構造体(void Read(const ParameterFile&) を持つ)
		 * @details JSON が変わったときだけ、構造体を読み直す
		 */
		template <typename T>
		class Parameter
		{
		public:
			Parameter() : m_file(nullptr), m_version(0) {}

			/**
			 * @brief 初期化
			 * @param filePath JSON ファイルのパス
			 * @details ファイルが無いときは、構造体の初期値のまま使う
			 */
			void Initialize(const char* filePath)
			{
				m_file = ParameterManager::GetInstance()->Load(filePath);
				Refresh(true);
			}

			/**
			 * @brief パラメーターを取得
			 * @return パラメーター(JSON が変わっていれば、読み直してから返す)
			 */
			const T& Get()
			{
				Refresh(false);
				return m_value;
			}


		private:
			/**
			 * @brief 必要なら、構造体を読み直す
			 * @param isForce 変わっていなくても読み直すか
			 */
			void Refresh(const bool isForce)
			{
				if (m_file == nullptr) return;
				if (!isForce && m_version == m_file->GetVersion()) return;

				m_value.Read(*m_file);
				m_version = m_file->GetVersion();
			}


		private:
			/** パラメーターの値 */
			T m_value;
			/** パラメーターファイル */
			ParameterFile* m_file;
			/** 読み込んだときの、ファイルの変わった回数 */
			uint32_t m_version;
		};
	}
}
