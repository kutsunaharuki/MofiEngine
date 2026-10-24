/**
 * @file Profiler.h
 * @brief CPU と GPU の処理時間を計測する
 */
#pragma once
#include <chrono>
#include <map>
#include <string>
#include <vector>


namespace nsK2EngineLow
{
	/**
	 * @brief CPU と GPU の処理時間を計測するクラス
	 * @details 計りたい所を K2_PROFILE_CPU / K2_PROFILE_GPU / K2_PROFILE で囲むと、その区間の時間を計る。
	 *          結果は一定時間(0.5秒)ごとに平均して、GetCpuSamples などで取れる(画面の表示は ImGui 側で行う)
	 *
	 * CPU : std::chrono で、区間の始まりと終わりの時刻の差を計る
	 * GPU : D3D12 のタイムスタンプのクエリを、コマンドリストに積んで計る。
	 *       GPU が実際に処理し終わるのは後なので、結果は2フレーム後に読む
	 *
	 * 1フレームの呼び方
	 *   g_engine->BeginFrame();          // コマンドリストの記録が始まる
	 *   Profiler::GetInstance().BeginFrame();
	 *   ...(計りたい所を K2_PROFILE_xxx で囲む)
	 *   Profiler::GetInstance().EndGpuFrame();   // コマンドリストを閉じる前に呼ぶ
	 *   g_engine->EndFrame();
	 */
	class Profiler : public Noncopyable
	{
	public:
		/** 計測結果の1行 */
		struct Sample
		{
			/** 区間の名前 */
			std::string name;
			/** 入れ子の深さ(0 が一番外側) */
			int depth = 0;
			/** 平均の時間(ミリ秒) */
			float averageMs = 0.0f;
			/** 一番長かった時間(ミリ秒) */
			float maxMs = 0.0f;
		};

		/** 履歴(グラフ用)の数 */
		static constexpr int HISTORY_COUNT = 120;


	public:
		/**
		 * @brief インスタンスを取得
		 * @return インスタンス
		 */
		static Profiler& GetInstance();

		/** GPU のリソースを解放する(GraphicsEngine を消す前に呼ぶ) */
		void Release();

		/**
		 * @brief フレームの始まり
		 * @details g_engine->BeginFrame() の後に呼ぶ(コマンドリストの記録が始まってから)
		 */
		void BeginFrame();
		/**
		 * @brief GPU のフレームの終わり
		 * @details g_engine->EndFrame() の前に呼ぶ(コマンドリストを閉じる前に、クエリの結果を読み出す命令を積む)
		 */
		void EndGpuFrame();

		/**
		 * @brief CPU の区間の始まり
		 * @param name 区間の名前(文字列リテラルなど、フレームの終わりまで消えない文字列)
		 */
		void BeginCpu(const char* name);
		/** CPU の区間の終わり */
		void EndCpu();
		/**
		 * @brief GPU の区間の始まり
		 * @param name 区間の名前(文字列リテラルなど、フレームの終わりまで消えない文字列)
		 */
		void BeginGpu(const char* name);
		/** GPU の区間の終わり */
		void EndGpu();

		/**
		 * @brief 数を記録する(描いたモデルの数など)
		 * @param name 名前
		 * @param value 数
		 */
		void SetCounter(const char* name, const int value) { m_counters[name] = value; }


	public:
		/** CPU の区間ごとの結果 */
		const std::vector<Sample>& GetCpuSamples() const { return m_cpuSamples; }
		/** GPU の区間ごとの結果 */
		const std::vector<Sample>& GetGpuSamples() const { return m_gpuSamples; }
		/** 記録した数 */
		const std::map<std::string, int>& GetCounters() const { return m_counters; }
		/** 1フレームの CPU の時間(ループ1周。GPU を待っている時間も入る)の平均(ミリ秒) */
		float GetCpuFrameMs() const { return m_cpuFrameMs; }
		/** 1フレームの GPU の時間の平均(ミリ秒) */
		float GetGpuFrameMs() const { return m_gpuFrameMs; }
		/** CPU のフレーム時間の履歴(古い順) */
		const float* GetCpuFrameHistory() const { return m_cpuFrameHistory; }
		/** GPU のフレーム時間の履歴(古い順) */
		const float* GetGpuFrameHistory() const { return m_gpuFrameHistory; }
		/** GPU の計測ができているか(タイムスタンプが使えない環境では false) */
		bool IsGpuAvailable() const { return m_queryHeap != nullptr; }


	private:
		/** 1フレームの中の、1つの区間の記録 */
		struct Record
		{
			const char* name = nullptr;
			int depth = 0;
			/** CPU : 始まりの時刻 / GPU : 始まりのクエリの番号 */
			std::chrono::steady_clock::time_point beginTime;
			int beginQuery = -1;
			/** GPU : 終わりのクエリの番号 */
			int endQuery = -1;
			/** CPU : かかった時間(ミリ秒) */
			float ms = 0.0f;
		};

		/** 区間ごとに貯めておく値(平均を出すため) */
		struct Accumulator
		{
			int depth = 0;
			double sumMs = 0.0;
			float maxMs = 0.0f;
		};

		/** フレームごとの GPU の記録(バックバッファの数だけ持つ) */
		struct GpuFrame
		{
			std::vector<Record> records;
			int queryCount = 0;
			/** クエリの結果を読み出す命令を積んだか */
			bool isResolved = false;
		};


	private:
		Profiler();
		~Profiler();

		/** GPU のリソースを作る(初めて使う時に呼ぶ) */
		void InitializeGpu();
		/**
		 * @brief 2フレーム前の GPU の結果を読む
		 * @param frame 読むフレーム
		 * @param slot フレームの番号
		 */
		void ReadGpuResult(GpuFrame& frame, const int slot);
		/**
		 * @brief 1フレーム分の記録を貯める
		 * @param name 名前
		 * @param depth 深さ
		 * @param ms 時間
		 * @param accumulators 貯める先
		 * @param order 出てきた順番
		 */
		static void Accumulate(const char* name, const int depth, const float ms, std::map<std::string, Accumulator>& accumulators, std::vector<std::string>& order);
		/**
		 * @brief 貯めた値から平均を出して、結果にする
		 * @param accumulators 貯めた値
		 * @param order 出てきた順番
		 * @param frameCount 貯めたフレーム数
		 * @param out 結果
		 */
		static void Publish(std::map<std::string, Accumulator>& accumulators, std::vector<std::string>& order, const int frameCount, std::vector<Sample>& out);


	private:
		/** 1フレームに計れる GPU の区間の数 */
		static constexpr int MAX_GPU_RECORDS = 32;
		/** 1フレームに使うクエリの数(区間ごとに始まりと終わりの2つ) */
		static constexpr int QUERIES_PER_FRAME = MAX_GPU_RECORDS * 2;
		/** フレームを持つ数(バックバッファの数と合わせる) */
		static constexpr int GPU_FRAME_COUNT = 2;
		/** 平均を出す間隔(秒) */
		static constexpr float PUBLISH_INTERVAL_SEC = 0.5f;

		/** CPU : 今のフレームの記録 */
		std::vector<Record> m_cpuRecords;
		/** CPU : 今開いている区間(m_cpuRecords の番号) */
		std::vector<int> m_cpuStack;
		/** CPU : 貯めている値 */
		std::map<std::string, Accumulator> m_cpuAccumulators;
		/** CPU : 区間が出てきた順番 */
		std::vector<std::string> m_cpuOrder;
		/** CPU : 結果 */
		std::vector<Sample> m_cpuSamples;

		/** GPU : フレームごとの記録 */
		GpuFrame m_gpuFrames[GPU_FRAME_COUNT];
		/** GPU : 今のフレームの番号(m_gpuFrames の番号。-1 なら計測していない) */
		int m_currentGpuSlot;
		/** GPU : 今開いている区間(記録の番号) */
		std::vector<int> m_gpuStack;
		/** GPU : 貯めている値 */
		std::map<std::string, Accumulator> m_gpuAccumulators;
		/** GPU : 区間が出てきた順番 */
		std::vector<std::string> m_gpuOrder;
		/** GPU : 結果 */
		std::vector<Sample> m_gpuSamples;
		/** GPU : 貯めたフレーム数 */
		int m_gpuFrameCount;
		/** GPU : フレーム全体の時間の合計(平均を出すため) */
		double m_gpuFrameSumMs;
		/** GPU : タイムスタンプのクエリ */
		ID3D12QueryHeap* m_queryHeap;
		/** GPU : クエリの結果を CPU から読むためのバッファ */
		ID3D12Resource* m_readbackBuffer;
		/** GPU : タイムスタンプの1秒あたりの数 */
		UINT64 m_timestampFrequency;
		/** GPU : 初期化を試したか */
		bool m_isGpuInitialized;

		/** 記録した数 */
		std::map<std::string, int> m_counters;

		/** フレームの始まりの時刻 */
		std::chrono::steady_clock::time_point m_frameBeginTime;
		/** 平均を出してからの時間(秒) */
		float m_publishTimer;
		/** CPU : 貯めたフレーム数 */
		int m_cpuFrameCount;
		/** CPU : フレーム全体の時間の合計 */
		double m_cpuFrameSumMs;
		/** CPU : フレームの平均の時間 */
		float m_cpuFrameMs;
		/** GPU : フレームの平均の時間 */
		float m_gpuFrameMs;
		/** 履歴 */
		float m_cpuFrameHistory[HISTORY_COUNT];
		float m_gpuFrameHistory[HISTORY_COUNT];
		/** 1フレーム目か(前のフレームが無い) */
		bool m_isFirstFrame;
	};




	/*****************************************************/


	/**
	 * @brief スコープの間だけ CPU の時間を計る
	 */
	class ScopedCpuProfile
	{
	public:
		explicit ScopedCpuProfile(const char* name) { Profiler::GetInstance().BeginCpu(name); }
		~ScopedCpuProfile() { Profiler::GetInstance().EndCpu(); }
	};


	/**
	 * @brief スコープの間だけ GPU の時間を計る
	 */
	class ScopedGpuProfile
	{
	public:
		explicit ScopedGpuProfile(const char* name) { Profiler::GetInstance().BeginGpu(name); }
		~ScopedGpuProfile() { Profiler::GetInstance().EndGpu(); }
	};
}


// 変数名がぶつからないように、行番号を付けた名前にする
#define K2_PROFILE_CONCAT_INNER(a, b) a##b
#define K2_PROFILE_CONCAT(a, b) K2_PROFILE_CONCAT_INNER(a, b)

/** このスコープの CPU の時間を計る */
#define K2_PROFILE_CPU(name) nsK2EngineLow::ScopedCpuProfile K2_PROFILE_CONCAT(cpuProfile_, __LINE__)(name)
/** このスコープの GPU の時間を計る(コマンドリストを記録している間だけ使える) */
#define K2_PROFILE_GPU(name) nsK2EngineLow::ScopedGpuProfile K2_PROFILE_CONCAT(gpuProfile_, __LINE__)(name)
/** このスコープの CPU と GPU の時間を両方計る */
#define K2_PROFILE(name) K2_PROFILE_CPU(name); K2_PROFILE_GPU(name)
