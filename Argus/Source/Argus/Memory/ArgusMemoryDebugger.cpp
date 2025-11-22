// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusMemoryDebugger.h"
#include "ArgusMath.h"
#include "ArgusMemorySource.h"
#include "imgui.h"

#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<bool> CVarDrawMemoryDebugger(TEXT("Argus.Debug.Memory"), false, TEXT("Whether or not the Memory ImGui debugger should be drawn."));

void ArgusMemoryDebugger::DrawMemoryDebugger()
{
	ARGUS_TRACE(ArgusMemoryDebugger::DrawMemoryDebugger);

	if (!CVarDrawMemoryDebugger.GetValueOnGameThread())
	{
		return;
	}

	const ImGui::FScopedContext scopedContext;
	if (!scopedContext)
	{
		return;
	}

	// ImGui::ShowDemoWindow();

	ImGui::SetNextWindowSize(ImVec2(260, 260), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Memory"))
	{
		ImGui::End();
		return;
	}

	const float denominator = static_cast<float>(ArgusMemorySource::k_1MB);
	ImGui::Text("Amount currently occupied including loss = %.3f MB", ArgusMath::SafeDivide(static_cast<float>(ArgusMemorySource::GetOccupiedAmount()), denominator));
	ImGui::Text("Amount currently occupied excluding loss = %.3f MB", ArgusMath::SafeDivide(static_cast<float>(ArgusMemorySource::GetOccupiedAmount() - ArgusMemorySource::GetTotalLossAmount()), denominator));
	ImGui::Text("Loss due to alignment = %.3f MB", ArgusMath::SafeDivide(static_cast<float>(ArgusMemorySource::GetAlignmentLossAmount()), denominator));
	ImGui::Text("Loss due to reallocation = %.3f MB", ArgusMath::SafeDivide(static_cast<float>(ArgusMemorySource::GetReallocationLossAmount()), denominator));
	ImGui::Text("Loss due to deallocation = %.3f MB", ArgusMath::SafeDivide(static_cast<float>(ArgusMemorySource::GetDeallocationLossAmount()), denominator));
	ImGui::Text("Total memory loss = %.3f MB", ArgusMath::SafeDivide(static_cast<float>(ArgusMemorySource::GetTotalLossAmount()), denominator));
	ImGui::Text("Remaining space available = %.3f MB", ArgusMath::SafeDivide(static_cast<float>(ArgusMemorySource::GetAvailableSpace()), denominator));
	ImGui::Text("Total space allocated from OS =  %.3f MB", ArgusMath::SafeDivide(static_cast<float>(ArgusMemorySource::GetCapacity()), denominator));

	ImGui::End();
}
#endif //!UE_BUILD_SHIPPING