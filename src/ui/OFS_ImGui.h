#pragma once
#include "OFS_Util.h"
#include "localization/OFS_Localization.h"

#include <imgui.h>

namespace OFS
{
	// ExampleAppLog taken from "imgui_demo.cpp"
	struct AppLog
	{
		ImGuiTextBuffer     Buf;
		ImGuiTextFilter     Filter;
		ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
		bool                AutoScroll;  // Keep scrolling if already at the bottom.

		AppLog() noexcept
		{
			AutoScroll = true;
			Clear();
		}

		void Clear() noexcept
		{
			Buf.clear();
			LineOffsets.clear();
			LineOffsets.push_back(0);
		}

		inline int LogSizeBytes() const noexcept
		{
			return Buf.Buf.size_in_bytes() + LineOffsets.size_in_bytes();
		}

		inline int AllocatedSizeBytes() const noexcept
		{
			return Buf.Buf.Capacity + LineOffsets.Capacity*sizeof(int);
		}

		void AddLog(const char* fmt, ...) noexcept IM_FMTARGS(2)
		{
			int old_size = Buf.size();
			va_list args;
			va_start(args, fmt);
			Buf.appendfv(fmt, args);
			va_end(args);
			for (int new_size = Buf.size(); old_size < new_size; old_size++)
				if (Buf[old_size] == '\n')
					LineOffsets.push_back(old_size + 1);
		}

		void Draw(const char* title, bool* p_open = NULL) noexcept
		{
			if (!ImGui::Begin(title, p_open)) {
				ImGui::End();
				return;
			}

			// Options menu
			if (ImGui::BeginPopup(TR_ID("OPTIONS", Tr::OPTIONS).c_str())) {
				ImGui::Checkbox(TR(AUTO_SCROLL), &AutoScroll);
				ImGui::EndPopup();
			}

			// Main window
			if (ImGui::Button(TR(OPTIONS)))
				ImGui::OpenPopup(TR_ID("OPTIONS", Tr::OPTIONS).c_str());
			ImGui::SameLine();
			bool clear = ImGui::Button(TR(CLEAR));
			ImGui::SameLine();
			bool copy = ImGui::Button(TR(COPY));
			ImGui::SameLine();
			Filter.Draw(TR(FILTER), -100.0f);

			ImGui::Text("%s: %s", TR(USED), OFS::util::formatBytes(LogSizeBytes()).c_str());
			ImGui::SameLine();
			ImGui::Text("%s: %s", TR(ALLOCATED), OFS::util::formatBytes(AllocatedSizeBytes()).c_str());
			ImGui::Separator();
			ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

			if (clear)
				Clear();
			if (copy)
				ImGui::LogToClipboard();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			const char* buf = Buf.begin();
			const char* buf_end = Buf.end();
			if (Filter.IsActive()) {
				for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					if (Filter.PassFilter(line_start, line_end))
						ImGui::TextUnformatted(line_start, line_end);
				}
			}
			else {
				ImGuiListClipper clipper;
				clipper.Begin(LineOffsets.Size);
				while (clipper.Step()) {
					for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
						const char* line_start = buf + LineOffsets[line_no];
						const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
						ImGui::TextUnformatted(line_start, line_end);
					}
				}
				clipper.End();
			}
			ImGui::PopStyleVar();

			if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);

			ImGui::EndChild();
			ImGui::End();
		}
	};

	// same as ImGui::Image except it has an id
	void ImageWithId(ImGuiID id, ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0)) noexcept;
    bool Spinner(const char* label, float radius, int thickness, const ImU32& color) noexcept;

	inline void Tooltip(const char* tip) noexcept
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(tip);
			ImGui::EndTooltip();
		}
	}
}

struct OFS_ImGui
{
	// This can be used during rendering callbacks to get the current viewport
	static ImGuiViewport* CurrentlyRenderedViewport;
};