#include "pch.h"
#include "View.h"

void CView::Show(Context& context)
{
	ImGui::BeginChild( context.Label.c_str() , context.Size , context.ChildFlags , context.WindowFlags );
	OnShow(context);
	ImGui::EndChild();
}