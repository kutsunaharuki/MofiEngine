/**
 * @file ProfilerWindow.cpp
 * @brief CPU と GPU の処理時間を、ImGui のウィンドウに表示する
 */
#include "stdafx.h"
#include "ProfilerWindow.h"
#include "imgui.h"


namespace app
{
	namespace util
	{
		namespace
		{
			/** CPU の区間のうち、GPU を待っている区間の名前(main.cpp と合わせる) */
			constexpr const char* WAIT_GPU_SAMPLE_NAME = "EndFrame (Present + Wait GPU)";
			/** グラフの縦の最大(ミリ秒)。33.3ms = 30FPS */
			constexpr float GRAPH_MAX_MS = 33.3f;
			/** グラフの高さ */
			constexpr float GRAPH_HEIGHT = 60.0f;
			/** 60FPS の1フレームの時間(ミリ秒) */
			constexpr float TARGET_FRAME_MS = 1000.0f / 60.0f;
		}


		ProfilerWindow::ProfilerWindow()
			: m_isOpen(true)
		{}


		void ProfilerWindow::Draw()
		{
			// F1 で表示・非表示(文字を入力している時は切り替えない)
			if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_F1, false))
			{
				m_isOpen = !m_isOpen;
			}
			if (!m_isOpen) return;

			ImGui::Begin("Profiler (F1)", &m_isOpen);

			DrawSummary();

			if (ImGui::CollapsingHeader("CPU", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawSampleTable("CpuSamples", true);
			}
			if (ImGui::CollapsingHeader("GPU", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (Profiler::GetInstance().IsGpuAvailable())
				{
					DrawSampleTable("GpuSamples", false);
				}
				else
				{
					ImGui::TextDisabled("GPU timestamps are not available.");
				}
			}
			if (ImGui::CollapsingHeader("Counters", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawCounters();
			}

			ImGui::End();
		}


		void ProfilerWindow::DrawSummary()
		{
			const Profiler& profiler = Profiler::GetInstance();
			const float cpuFrameMs = profiler.GetCpuFrameMs();
			const float gpuFrameMs = profiler.GetGpuFrameMs();

			// CPU が自分の仕事をしている時間 = フレーム全体 - GPU を待っている時間
			float waitGpuMs = 0.0f;
			for (const Profiler::Sample& sample : profiler.GetCpuSamples())
			{
				if (sample.name == WAIT_GPU_SAMPLE_NAME)
				{
					waitGpuMs = sample.averageMs;
				}
			}
			const float cpuWorkMs = cpuFrameMs - waitGpuMs;

			ImGui::Text("FPS: %.1f  (frame %.2f ms)", cpuFrameMs > 0.0f ? 1000.0f / cpuFrameMs : 0.0f, cpuFrameMs);
			ImGui::Text("CPU work: %.2f ms   GPU: %.2f ms   (60 FPS = %.2f ms)", cpuWorkMs, gpuFrameMs, TARGET_FRAME_MS);

			// 長い方が、フレームの時間を決めている(ボトルネック)
			if (gpuFrameMs > cpuWorkMs)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Bottleneck: GPU");
			}
			else
			{
				ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Bottleneck: CPU");
			}

#ifdef K2_DEBUG
			// Debug は最適化されていないので、CPU の時間が何倍も長くなる
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "Debug build: measure in Release for real numbers.");
#endif

			ImGui::PlotLines("CPU frame", profiler.GetCpuFrameHistory(), Profiler::HISTORY_COUNT, 0, nullptr, 0.0f, GRAPH_MAX_MS, ImVec2(0.0f, GRAPH_HEIGHT));
			ImGui::PlotLines("GPU frame", profiler.GetGpuFrameHistory(), Profiler::HISTORY_COUNT, 0, nullptr, 0.0f, GRAPH_MAX_MS, ImVec2(0.0f, GRAPH_HEIGHT));
		}


		void ProfilerWindow::DrawSampleTable(const char* tableId, const bool isCpu)
		{
			const Profiler& profiler = Profiler::GetInstance();
			const auto& samples = isCpu ? profiler.GetCpuSamples() : profiler.GetGpuSamples();
			// バーの長さは、フレーム全体に対する割合にする
			const float frameMs = isCpu ? profiler.GetCpuFrameMs() : profiler.GetGpuFrameMs();

			const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
			if (!ImGui::BeginTable(tableId, 4, flags)) return;

			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Avg ms");
			ImGui::TableSetupColumn("Max ms");
			ImGui::TableSetupColumn("%", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableHeadersRow();

			for (const Profiler::Sample& sample : samples)
			{
				ImGui::TableNextRow();

				// 入れ子の深さだけ字下げする
				ImGui::TableSetColumnIndex(0);
				ImGui::Indent(static_cast<float>(sample.depth) * 12.0f + 0.001f);
				ImGui::TextUnformatted(sample.name.c_str());
				ImGui::Unindent(static_cast<float>(sample.depth) * 12.0f + 0.001f);

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f", sample.averageMs);
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%.3f", sample.maxMs);

				ImGui::TableSetColumnIndex(3);
				const float ratio = frameMs > 0.0f ? sample.averageMs / frameMs : 0.0f;
				char overlay[16];
				snprintf(overlay, sizeof(overlay), "%.1f%%", ratio * 100.0f);
				ImGui::ProgressBar(ratio, ImVec2(-1.0f, 0.0f), overlay);
			}
			ImGui::EndTable();
		}


		void ProfilerWindow::DrawCounters()
		{
			for (const auto& pair : Profiler::GetInstance().GetCounters())
			{
				ImGui::Text("%s: %d", pair.first.c_str(), pair.second);
			}
		}
	}
}
