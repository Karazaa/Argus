// Copyright Karazaa. This is a part of an RTS project called Argus.

#if !UE_BUILD_SHIPPING
#include "ArgusSaveManager.h"
#include "ArgusMacros.h"
#include "imgui.h"

static TAutoConsoleVariable<bool> CVarDrawSaveManagerDebugger(TEXT("Argus.Debug.SaveManager"), false, TEXT("Whether or not the SaveManager ImGui debugger should be drawn."));

void UArgusSaveManager::DrawDebugger()
{
	ARGUS_TRACE(UArgusSaveManager::DrawDebugger);

	if (!CVarDrawSaveManagerDebugger.GetValueOnGameThread())
	{
		return;
	}

	const ImGui::FScopedContext scopedContext;
	if (!scopedContext)
	{
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(260, 260), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("SaveManager"))
	{
		ImGui::End();
		return;
	}

	if (ImGui::Button("Save"))
	{
		Save();
	}

	ImGui::SameLine();

	if (ImGui::Button("Load"))
	{
		LoadMostRecent([](UArgusSaveGame* saveGame) 
		{
			// TODO JAMES: Do something
		});
	}

	ImGui::End();
}

#endif //!UE_BUILD_SHIPPING