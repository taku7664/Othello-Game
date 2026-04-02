#include "pch.h"
#include "ImGuiUtillity.h"

bool ImGui::Utillity::IsWindowDrawable(ImGuiWindow* window)
{
    if (!window)
        window = ImGui::GetCurrentWindowRead();
    if (!window)
        return false;

    return !window->SkipItems;
}

void ImGui::Utillity::TextWithVerticalSeparator( const char* text , float startX )
{
	ImGui::Text( text );
	if ( FLT_MAX == startX )
	{
		startX = ImGui::GetCursorPosX();
		startX += ImGui::CalcTextSize( text ).x;
		startX += ImGui::GetStyle().ItemSpacing.x;
	}
	ImGui::SameLine( startX );
	ImGui::SeparatorEx( ImGuiSeparatorFlags_Vertical );
	ImGui::SameLine();

	float availX = ImGui::GetContentRegionAvail().x;
	ImGui::SetNextItemWidth( availX );
}

ImVec4 ImGui::Utillity::ColorFromGuid( const GUID& guid )
{
	float r = guid.Data1 % 256 / 255.0f;
	float g = guid.Data2 % 256 / 255.0f;
	float b = guid.Data3 % 256 / 255.0f;

	// 너무 어둡거나 밝은 색 방지 (선택)
	const float minBrightness = 0.5f;
	r = minBrightness + r * ( 1.0f - minBrightness );
	g = minBrightness + g * ( 1.0f - minBrightness );
	b = minBrightness + b * ( 1.0f - minBrightness );

	return ImVec4( r , g , b , 1.0f);
}

bool ImGui::Utillity::HoveredToolTip( const char* toolTip , int flags)
{
	bool isHovered = ImGui::IsItemHovered(flags);
	if (isHovered)
	{
		ImGui::SetTooltip(toolTip);
	}
	return isHovered;
}
