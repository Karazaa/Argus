// Copyright Karazaa. This is a part of an RTS project called Argus.

#if !UE_BUILD_SHIPPING
#include "ArgusSaveManager.h"
#include "ArgusMacros.h"
#include "ArgusMetadataSaveGame.h"
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

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar;
	int childFlags = ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY;

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

	if (ImGui::Button("Load Most Recent"))
	{
		LoadMostRecent([](UArgusSaveGame* saveGame)
		{
			// TODO JAMES: Do something
		});
	}

	ImGui::SameLine();

	if (ImGui::Button("Load"))
	{
		if (m_saveMetadata && m_saveMetadata->m_saveSlotMetadata.IsValidIndex(m_debugSelectedIndex))
		{
			Load(m_saveMetadata->m_saveSlotMetadata[m_debugSelectedIndex].m_slotName, [](UArgusSaveGame* saveGame)
			{
				// TODO JAMES: Do something
			});
		}
	}

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

	ImGui::BeginChild("CurrentSaveRegion", ImVec2(0, 0), childFlags, windowFlags);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Current Saves"))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	if (ImGui::BeginTable("CurrentSaveTable", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp))
	{
		if (m_saveMetadata)
		{
			for (int32 i = 0; i < m_saveMetadata->m_saveSlotMetadata.Num(); ++i)
			{
				bool selected = i == static_cast<int32>(m_debugSelectedIndex);
				const char* slotName = ARGUS_FSTRING_TO_CHAR(m_saveMetadata->m_saveSlotMetadata[i].m_slotName);
				const char* timestamp = ARGUS_FSTRING_TO_CHAR(m_saveMetadata->m_saveSlotMetadata[i].m_saveTimestamp.ToFormattedString(TEXT("%Y-%m-%d-%I:%M")));

				ImGui::TableNextColumn();
				if (ImGui::Selectable(slotName, &selected, ImGuiSelectableFlags_SpanAllColumns))
				{
					m_debugSelectedIndex = i;
				}
				ImGui::TableNextColumn();
				ImGui::Text(timestamp);
			}
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();

	ImGui::End();
}

#endif //!UE_BUILD_SHIPPING