#include "pch.h"
#include "DebugWindow.h"

void CDebugWindow::OnCreate()
{
	SetTitle( "디버그" );

	UINT flags = ImGuiWindowFlags_NoDocking;
	GetImGuiWindowFlags().Set( flags );
}

void CDebugWindow::OnRenderStay()
{
	const ImGuiIO& io = ImGui::GetIO();

	ImGui::Text( "=== Frame ===" );
	ImGui::Text( "FPS           : %.1f" , io.Framerate );
	ImGui::Text( "Frame Time    : %.3f ms" , 1000.0f / ( io.Framerate > 0.0f ? io.Framerate : 1.0f ) );
	ImGui::Text( "Delta Time    : %.6f sec" , io.DeltaTime );

	ImGui::Separator();

	ImGui::Text( "=== Display ===" );
	ImGui::Text( "Display Size  : %.0f x %.0f" , io.DisplaySize.x , io.DisplaySize.y );
	ImGui::Text( "Framebuffer   : %.0f x %.0f" , io.DisplayFramebufferScale.x , io.DisplayFramebufferScale.y );

	ImGui::Separator();

	ImGui::Text( "=== Render Stats ===" );
	ImGui::Text( "Vertices      : %d" , io.MetricsRenderVertices );
	ImGui::Text( "Indices       : %d" , io.MetricsRenderIndices );
	ImGui::Text( "Windows       : %d" , io.MetricsRenderWindows );
	ImGui::Text( "Active Windows: %d" , io.MetricsActiveWindows );

	ImGui::Separator();

	ImGui::Text( "=== Dear ImGui ===" );
	ImGui::Text( "WantCaptureMouse    : %d" , io.WantCaptureMouse );
	ImGui::Text( "WantCaptureKeyboard : %d" , io.WantCaptureKeyboard );
	ImGui::Text( "WantTextInput       : %d" , io.WantTextInput );
}
