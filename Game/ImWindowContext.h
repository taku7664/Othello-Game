#pragma once

struct ImPopupContext
{
	std::string			Title;
	ImVec2				Size;
	ImGuiWindowFlags	Flags;
	bool				IsOpen = true;
};

