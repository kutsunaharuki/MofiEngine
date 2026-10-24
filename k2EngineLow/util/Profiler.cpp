/**
 * @file Profiler.cpp
 * @brief CPU と GPU の処理時間を計測する
 */
#include "k2EngineLowPreCompile.h"
#include "Profiler.h"
#include <cstring>


namespace nsK2EngineLow
{
	namespace
	{
		/** GPU のフレーム全体の区間の名前(記録の一番目に入れる) */
		constexpr const char* GPU_FRAME_NAME = "GPU Frame";


		/**
		 * @brief 履歴を1つずらして、一番新しい値を最後に入れる
		 * @param history 履歴
		 * @param value 新しい値
		 */
		void PushHistory(float* history, const float value)
		{
			memmove(history, history + 1, sizeof(float) * (Profiler::HISTORY_COUNT - 1));
			history[Profiler::HISTORY_COUNT - 1] = value;
		}
	}


	Profiler& Profiler::GetInstance()
	{
		static Profiler instance;
		return instance;
	}


	Profiler::Profiler()
		: m_currentGpuSlot(-1)
		, m_gpuFrameCount(0)
		, m_gpuFrameSumMs(0.0)
		, m_queryHeap(nullptr)
		, m_readbackBuffer(nullptr)
		, m_timestampFrequency(0)
		, m_isGpuInitialized(false)
		, m_publishTimer(0.0f)
		, m_cpuFrameCount(0)
		, m_cpuFrameSumMs(0.0)
		, m_cpuFrameMs(0.0f)
		, m_gpuFrameMs(0.0f)
		, m_isFirstFrame(true)
	{
		memset(m_cpuFrameHistory, 0, sizeof(m_cpuFrameHistory));
		memset(m_gpuFrameHistory, 0, sizeof(m_gpuFrameHistory));
	}


	Profiler::~Profiler()
	{
		Release();
	}


	void Profiler::Release()
	{
		if (m_queryHeap != nullptr)
		{
			m_queryHeap->Release();
			m_queryHeap = nullptr;
		}
		if (m_readbackBuffer != nullptr)
		{
			m_readbackBuffer->Release();
			m_readbackBuffer = nullptr;
		}
		m_currentGpuSlot = -1;
	}


	void Profiler::InitializeGpu()
	{
		m_isGpuInitialized = true;

		// タイムスタンプが1秒に何回進むか(GPU によって違う)
		if (FAILED(g_graphicsEngine->GetCommandQueue()->GetTimestampFrequency(&m_timestampFrequency)) || m_timestampFrequency == 0)
		{
			return;
		}

		ID3D12Device5* device = g_graphicsEngine->GetD3DDevice();

		// タイムスタンプのクエリ(フレームの数 × 1フレームのクエリの数)
		D3D12_QUERY_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		heapDesc.Count = GPU_FRAME_COUNT * QUERIES_PER_FRAME;
		if (FAILED(device->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&m_queryHeap))))
		{
			m_queryHeap = nullptr;
			return;
		}

		// クエリの結果を CPU から読むためのバッファ(READBACK ヒープ)
		const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_READBACK);
		const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT64) * GPU_FRAME_COUNT * QUERIES_PER_FRAME);
		if (FAILED(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_readbackBuffer))))
		{
			Release();
		}
	}


	void Profiler::BeginFrame()
	{
		const auto now = std::chrono::steady_clock::now();

		// 前のフレームの CPU の記録を貯める
		if (!m_isFirstFrame)
		{
			const float frameMs = std::chrono::duration<float, std::milli>(now - m_frameBeginTime).count();
			for (const Record& record : m_cpuRecords)
			{
				Accumulate(record.name, record.depth, record.ms, m_cpuAccumulators, m_cpuOrder);
			}
			m_cpuFrameCount++;
			m_cpuFrameSumMs += frameMs;
			PushHistory(m_cpuFrameHistory, frameMs);
			m_publishTimer += frameMs / 1000.0f;
		}
		m_isFirstFrame = false;
		m_cpuRecords.clear();
		m_cpuStack.clear();
		m_frameBeginTime = now;

		// GPU
		if (!m_isGpuInitialized)
		{
			InitializeGpu();
		}
		m_currentGpuSlot = -1;
		m_gpuStack.clear();
		if (m_queryHeap != nullptr)
		{
			// バックバッファの番号ごとにクエリを分ける。同じ番号の前の結果(2フレーム前)は、
			// GraphicsEngine が前のフレームの EndRender で GPU の完了を待っているので、もう読める
			const int slot = static_cast<int>(g_graphicsEngine->GetBackBufferIndex()) % GPU_FRAME_COUNT;
			GpuFrame& frame = m_gpuFrames[slot];
			if (frame.isResolved)
			{
				ReadGpuResult(frame, slot);
			}
			frame.records.clear();
			frame.queryCount = 0;
			frame.isResolved = false;
			m_currentGpuSlot = slot;

			// フレーム全体の区間(記録の一番目)
			BeginGpu(GPU_FRAME_NAME);
		}

		// 一定時間ごとに平均を出す(毎フレームだと数字が速く変わりすぎて読めない)
		if (m_publishTimer >= PUBLISH_INTERVAL_SEC)
		{
			if (m_cpuFrameCount > 0)
			{
				Publish(m_cpuAccumulators, m_cpuOrder, m_cpuFrameCount, m_cpuSamples);
				m_cpuFrameMs = static_cast<float>(m_cpuFrameSumMs / m_cpuFrameCount);
			}
			if (m_gpuFrameCount > 0)
			{
				Publish(m_gpuAccumulators, m_gpuOrder, m_gpuFrameCount, m_gpuSamples);
				m_gpuFrameMs = static_cast<float>(m_gpuFrameSumMs / m_gpuFrameCount);
			}
			m_cpuFrameCount = 0;
			m_cpuFrameSumMs = 0.0;
			m_gpuFrameCount = 0;
			m_gpuFrameSumMs = 0.0;
			m_publishTimer = 0.0f;
		}
	}


	void Profiler::EndGpuFrame()
	{
		if (m_currentGpuSlot < 0) return;

		// 閉じ忘れた区間と、フレーム全体の区間を閉じる
		while (!m_gpuStack.empty())
		{
			EndGpu();
		}

		// 使ったクエリの結果を、読み出し用のバッファにコピーする命令を積む
		GpuFrame& frame = m_gpuFrames[m_currentGpuSlot];
		if (frame.queryCount > 0)
		{
			const UINT baseIndex = static_cast<UINT>(m_currentGpuSlot * QUERIES_PER_FRAME);
			g_graphicsEngine->GetCommandList()->ResolveQueryData(
				m_queryHeap,
				D3D12_QUERY_TYPE_TIMESTAMP,
				baseIndex,
				static_cast<UINT>(frame.queryCount),
				m_readbackBuffer,
				sizeof(UINT64) * baseIndex
			);
			frame.isResolved = true;
		}
		m_currentGpuSlot = -1;
	}


	void Profiler::BeginCpu(const char* name)
	{
		Record record;
		record.name = name;
		record.depth = static_cast<int>(m_cpuStack.size());
		record.beginTime = std::chrono::steady_clock::now();
		m_cpuStack.push_back(static_cast<int>(m_cpuRecords.size()));
		m_cpuRecords.push_back(record);
	}


	void Profiler::EndCpu()
	{
		if (m_cpuStack.empty()) return;

		Record& record = m_cpuRecords[m_cpuStack.back()];
		m_cpuStack.pop_back();
		record.ms = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - record.beginTime).count();
	}


	void Profiler::BeginGpu(const char* name)
	{
		// 計測していない時や、区間が多すぎる時は計らない(EndGpu と数を合わせるため、-1 を積む)
		if (m_currentGpuSlot < 0)
		{
			m_gpuStack.push_back(-1);
			return;
		}
		GpuFrame& frame = m_gpuFrames[m_currentGpuSlot];
		if (frame.queryCount + 2 > QUERIES_PER_FRAME)
		{
			m_gpuStack.push_back(-1);
			return;
		}

		Record record;
		record.name = name;
		record.depth = static_cast<int>(m_gpuStack.size());
		record.beginQuery = m_currentGpuSlot * QUERIES_PER_FRAME + frame.queryCount;
		frame.queryCount++;
		g_graphicsEngine->GetCommandList()->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, static_cast<UINT>(record.beginQuery));

		m_gpuStack.push_back(static_cast<int>(frame.records.size()));
		frame.records.push_back(record);
	}


	void Profiler::EndGpu()
	{
		if (m_gpuStack.empty()) return;

		const int recordIndex = m_gpuStack.back();
		m_gpuStack.pop_back();
		if (recordIndex < 0 || m_currentGpuSlot < 0) return;

		GpuFrame& frame = m_gpuFrames[m_currentGpuSlot];
		Record& record = frame.records[recordIndex];
		record.endQuery = m_currentGpuSlot * QUERIES_PER_FRAME + frame.queryCount;
		frame.queryCount++;
		g_graphicsEngine->GetCommandList()->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, static_cast<UINT>(record.endQuery));
	}


	void Profiler::ReadGpuResult(GpuFrame& frame, const int slot)
	{
		const size_t baseIndex = static_cast<size_t>(slot) * QUERIES_PER_FRAME;
		const D3D12_RANGE readRange = { sizeof(UINT64) * baseIndex, sizeof(UINT64) * (baseIndex + frame.queryCount) };
		void* mapped = nullptr;
		if (FAILED(m_readbackBuffer->Map(0, &readRange, &mapped))) return;

		const UINT64* timestamps = static_cast<const UINT64*>(mapped);
		const double toMs = 1000.0 / static_cast<double>(m_timestampFrequency);

		for (size_t i = 0; i < frame.records.size(); ++i)
		{
			const Record& record = frame.records[i];
			if (record.beginQuery < 0 || record.endQuery < 0) continue;

			const UINT64 begin = timestamps[record.beginQuery];
			const UINT64 end = timestamps[record.endQuery];
			const float ms = end > begin ? static_cast<float>((end - begin) * toMs) : 0.0f;

			if (i == 0)
			{
				// 一番目はフレーム全体
				m_gpuFrameSumMs += ms;
				PushHistory(m_gpuFrameHistory, ms);
			}
			else
			{
				// フレーム全体の区間の中にあるので、深さを1つ戻す
				Accumulate(record.name, record.depth - 1, ms, m_gpuAccumulators, m_gpuOrder);
			}
		}
		m_gpuFrameCount++;

		// CPU からは書いていないので、書いた範囲は空にする
		const D3D12_RANGE writtenRange = { 0, 0 };
		m_readbackBuffer->Unmap(0, &writtenRange);
	}


	void Profiler::Accumulate(const char* name, const int depth, const float ms, std::map<std::string, Accumulator>& accumulators, std::vector<std::string>& order)
	{
		auto it = accumulators.find(name);
		if (it == accumulators.end())
		{
			it = accumulators.emplace(name, Accumulator()).first;
			order.push_back(name);
		}
		Accumulator& accumulator = it->second;
		accumulator.depth = depth;
		accumulator.sumMs += ms;
		if (ms > accumulator.maxMs)
		{
			accumulator.maxMs = ms;
		}
	}


	void Profiler::Publish(std::map<std::string, Accumulator>& accumulators, std::vector<std::string>& order, const int frameCount, std::vector<Sample>& out)
	{
		out.clear();
		for (const std::string& name : order)
		{
			const Accumulator& accumulator = accumulators[name];
			Sample sample;
			sample.name = name;
			sample.depth = accumulator.depth;
			// 1フレームに何回も通る区間は、合計を1フレームの時間にする
			sample.averageMs = static_cast<float>(accumulator.sumMs / frameCount);
			sample.maxMs = accumulator.maxMs;
			out.push_back(sample);
		}
		accumulators.clear();
		order.clear();
	}
}
