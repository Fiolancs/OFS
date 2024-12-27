#include "GradientBar.h"
#include "OFS_Profiling.h"

#include <imgui_internal.h>

#include <algorithm>

void ImGradient::addMark(float position, const ImColor& color) noexcept
{
	position = ImClamp(position, 0.0f, 1.0f);
	m_marks.emplace_back(color.Value.x, color.Value.y, color.Value.z, color.Value.w, position);
}

void ImGradient::removeMark(const ImGradientMark& mark) noexcept
{
	auto it = std::find(m_marks.begin(), m_marks.end(), mark);
	if (it != m_marks.end())
		m_marks.erase(it);
}

void ImGradient::computeColorAt(float position, float* color) const noexcept
{
	OFS_PROFILE(__FUNCTION__);

	position = ImClamp(position, 0.0f, 1.0f);

	const ImGradientMark* lower = nullptr;
	const ImGradientMark* upper = nullptr;

	for (auto& m : m_marks)
	{
		auto mark = &m;
		if (mark->position < position)
		{
			if (!lower || lower->position < mark->position)
			{
				lower = mark;
			}
		}

		if (mark->position >= position)
		{
			if (!upper || upper->position > mark->position)
			{
				upper = mark;
			}
		}
	}

	if (upper && !lower)
	{
		lower = upper;
	}
	else if (!upper && lower)
	{
		upper = lower;
	}
	else if (!lower && !upper)
	{
		color[0] = color[1] = color[2] = 0;
		return;
	}

	if (upper == lower)
	{
		color[0] = upper->color[0];
		color[1] = upper->color[1];
		color[2] = upper->color[2];
	}
	else
	{
		float distance = upper->position - lower->position;
		float delta = (position - lower->position) / distance;

		//lerp
		color[0] = ((1.0f - delta) * lower->color[0]) + ((delta)*upper->color[0]);
		color[1] = ((1.0f - delta) * lower->color[1]) + ((delta)*upper->color[1]);
		color[2] = ((1.0f - delta) * lower->color[2]) + ((delta)*upper->color[2]);
	}
}

void ImGradient::refreshCache() noexcept
{
	OFS_PROFILE(__FUNCTION__);
	std::sort(m_marks.begin(), m_marks.end(), [](auto& a, auto& b) { return a.position < b.position; });
	
	for (int i = 0; i < 256; ++i)
	{
		computeColorAt(i / 255.0f, &m_cachedValues[i * 3]);
	}
}

void ImGradient::DrawGradientBar(ImGradient* gradient, const ImVec2& bar_pos, float maxWidth, float height) noexcept
{
	OFS_PROFILE(__FUNCTION__);
	ImVec4 colorA = { 1,1,1,1 };
	ImVec4 colorB = { 1,1,1,1 };
	float prevX = bar_pos.x;
	float barBottom = bar_pos.y + height;
	ImGradientMark* prevMark = nullptr;
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	draw_list->AddRectFilled(ImVec2(bar_pos.x - 2, bar_pos.y - 2),
		ImVec2(bar_pos.x + maxWidth + 2, barBottom + 2),
		IM_COL32(100, 100, 100, 255));

	if (gradient->getMarks().size() == 0)
	{
		draw_list->AddRectFilled(ImVec2(bar_pos.x, bar_pos.y),
			ImVec2(bar_pos.x + maxWidth, barBottom),
			IM_COL32(255, 255, 255, 255));

	}

	ImU32 colorAU32 = 0;
	ImU32 colorBU32 = 0;

	for(auto& markIt : gradient->m_marks)
	{
		ImGradientMark* mark = &(markIt);

		float from = prevX;
		float to = prevX = bar_pos.x + mark->position * maxWidth;

		if (prevMark == nullptr)
		{
			colorA.x = mark->color[0];
			colorA.y = mark->color[1];
			colorA.z = mark->color[2];
		}
		else
		{
			colorA.x = prevMark->color[0];
			colorA.y = prevMark->color[1];
			colorA.z = prevMark->color[2];
		}

		colorB.x = mark->color[0];
		colorB.y = mark->color[1];
		colorB.z = mark->color[2];

		colorAU32 = ImGui::ColorConvertFloat4ToU32(colorA);
		colorBU32 = ImGui::ColorConvertFloat4ToU32(colorB);

		if (mark->position > 0.0)
		{

			draw_list->AddRectFilledMultiColor(ImVec2(from, bar_pos.y),
				ImVec2(to, barBottom),
				colorAU32, colorBU32, colorBU32, colorAU32);
		}

		prevMark = mark;
	}

	if (prevMark && prevMark->position < 1.0)
	{

		draw_list->AddRectFilledMultiColor(ImVec2(prevX, bar_pos.y),
			ImVec2(bar_pos.x + maxWidth, barBottom),
			colorBU32, colorBU32, colorBU32, colorBU32);
	}

	ImGui::SetCursorScreenPos(ImVec2(bar_pos.x, bar_pos.y + height + 10.0f));
}
