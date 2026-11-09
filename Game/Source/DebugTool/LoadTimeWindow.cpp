/**
 * @file LoadTimeWindow.cpp
 * @brief 読み込みにかかった時間を区間ごとに計って、ImGui のウィンドウに表示する
 */
#include "stdafx.h"
#include "LoadTimeWindow.h"
#include "imgui.h"
#include <cstdio>


namespace app
{
	namespace debug
	{
		namespace
		{
			/** 1 行の文字の長さ(区間の名前 + 時間) */
			constexpr int LOAD_TIME_LINE_LENGTH = 128;
		}


		LoadTimeWindow::LoadTimeWindow()
			: m_title()
			, m_sectionList()
			, m_beginTime()
			, m_lastTime()
			, m_totalSeconds(0.0)
			, m_isMeasuring(false)
		{}


		LoadTimeWindow::~LoadTimeWindow()
		{}


		void LoadTimeWindow::Begin(const char* title)
		{
			m_title = title;
			m_sectionList.clear();
			m_totalSeconds = 0.0;
			m_isMeasuring = true;
			m_beginTime = Clock::now();
			m_lastTime = m_beginTime;
		}


		void LoadTimeWindow::Mark(const char* name)
		{
			if (!m_isMeasuring) return;

			const Clock::time_point now = Clock::now();
			const double seconds = std::chrono::duration<double>(now - m_lastTime).count();
			m_sectionList.push_back(Section{ name, seconds });
			// 次の区間は、ここから数える
			m_lastTime = now;
		}


		void LoadTimeWindow::End()
		{
			if (!m_isMeasuring) return;

			// 最後の Mark から End までに何かあれば、それも区間として残す(計り漏れに気づけるように)
			const double restSeconds = std::chrono::duration<double>(Clock::now() - m_lastTime).count();
			if (!m_sectionList.empty() && restSeconds >= 0.001)
			{
				Mark("(rest)");
			}

			m_totalSeconds = std::chrono::duration<double>(m_lastTime - m_beginTime).count();
			m_isMeasuring = false;

			// Release ではウィンドウが出ないので、出力ウィンドウにも書く
			OutputDebugStringA(MakeReport().c_str());
		}


		void LoadTimeWindow::Draw()
		{
#ifdef K2_DEBUG
			if (m_sectionList.empty()) return;

			ImGui::Begin("Load Time");

			ImGui::Text("%s  total %.3f s", m_title.c_str(), m_totalSeconds);
			// Obsidian などに貼れるように、表を文字でコピーする
			ImGui::SameLine();
			if (ImGui::Button("Copy"))
			{
				ImGui::SetClipboardText(MakeReport().c_str());
			}
#ifdef _DEBUG
			ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Debug build: slower than Release. Use Preview to compare.");
#endif

			if (ImGui::BeginTable("LoadTimeSections", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("Section");
				ImGui::TableSetupColumn("Seconds");
				ImGui::TableSetupColumn("Share");
				ImGui::TableHeadersRow();

				for (const Section& section : m_sectionList)
				{
					// 合計に対する割合(どこが重いかを一目で見る)
					const float share = m_totalSeconds > 0.0 ? static_cast<float>(section.seconds / m_totalSeconds) : 0.0f;

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(section.name.c_str());
					ImGui::TableNextColumn();
					ImGui::Text("%.3f", section.seconds);
					ImGui::TableNextColumn();
					char label[16];
					std::snprintf(label, sizeof(label), "%.1f%%", share * 100.0f);
					ImGui::ProgressBar(share, ImVec2(-1.0f, 0.0f), label);
				}
				ImGui::EndTable();
			}

			ImGui::End();
#endif
		}


		std::string LoadTimeWindow::MakeReport() const
		{
			std::string report = "[LoadTime] " + m_title + "\n";
			char line[LOAD_TIME_LINE_LENGTH];
			for (const Section& section : m_sectionList)
			{
				std::snprintf(line, sizeof(line), "  %-24s %8.3f s\n", section.name.c_str(), section.seconds);
				report += line;
			}
			std::snprintf(line, sizeof(line), "  %-24s %8.3f s\n", "total", m_totalSeconds);
			report += line;
			return report;
		}
	}
}
