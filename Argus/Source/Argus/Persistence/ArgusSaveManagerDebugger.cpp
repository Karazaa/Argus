// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING
#include "ArgusSaveManager.h"
#include "ArgusMacros.h"
#include "ArgusMetadataSaveGame.h"
#include "HAL/IConsoleManager.h"
#include "imgui.h"

static TAutoConsoleVariable<bool> CVarDrawSaveManagerDebugger(TEXT("Argus.Debug.SaveManager"), false, TEXT("Whether or not the SaveManager ImGui debugger should be drawn."));

namespace
{
	static constexpr int32 k_stringBufferSize = 128;
}

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

	bool isSaving = IsSaving();
	bool isLoading = IsLoading();
	ImGui::Checkbox("Is Saving?", &isSaving);
	ImGui::SameLine();
	ImGui::Checkbox("Is Loading?", &isLoading);

	ImGui::NewLine();

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

	ImGui::SameLine();

	if (ImGui::Button("Delete"))
	{
		if (m_saveMetadata && m_saveMetadata->m_saveSlotMetadata.IsValidIndex(m_debugSelectedIndex))
		{
			DeleteSaveGame(m_saveMetadata->m_saveSlotMetadata[m_debugSelectedIndex].m_slotName, [](const FString& slotName, bool didSucceed)
			{
				// TODO JAMES: Do something
			});
		}
	}

	if (ImGui::BeginTable("CurrentSaveTable", 3, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuter))
	{
		ImGui::TableSetupColumn("Slot Name");
		ImGui::TableSetupColumn("Description");
		ImGui::TableSetupColumn("Timestamp");
		ImGui::TableHeadersRow();
		if (m_saveMetadata)
		{
			for (int32 i = 0; i < m_saveMetadata->m_saveSlotMetadata.Num(); ++i)
			{
				ImGui::PushID(i);

				bool selected = i == static_cast<int32>(m_debugSelectedIndex);
				const char* slotName = ARGUS_FSTRING_TO_CHAR(m_saveMetadata->m_saveSlotMetadata[i].m_slotName);
				const char* timestamp = ARGUS_FSTRING_TO_CHAR(m_saveMetadata->m_saveSlotMetadata[i].m_saveTimestamp.ToFormattedString(TEXT("%Y-%m-%d-%I:%M")));

				ImGui::TableNextColumn();
				if (ImGui::Selectable(slotName, &selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
				{
					m_debugSelectedIndex = i;
				}

				ImGui::TableNextColumn();
				if (m_saveMetadata->m_saveSlotMetadata[i].m_description.GetAllocatedSize() != k_stringBufferSize)
				{
					m_saveMetadata->m_saveSlotMetadata[i].m_description.Reserve(k_stringBufferSize);
				}
				char* buffer = ARGUS_FSTRING_TO_CHAR(m_saveMetadata->m_saveSlotMetadata[i].m_description);
				ImGui::PushItemWidth(-1.0f);
				if (ImGui::InputText("", buffer, k_stringBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (!IsSaving())
					{
						m_saveMetadata->m_saveSlotMetadata[i].m_description = FString(buffer);
						SaveLoadLock saveLoadLock = SaveLoadLock(SaveLoadLockType::SaveLock);
						SaveMetadata(saveLoadLock);
					}
				}
				ImGui::PopItemWidth();

				ImGui::TableNextColumn();
				ImGui::Text(timestamp);

				ImGui::PopID();
			}
		}
		ImGui::EndTable();
	}

	ImGui::End();
}

#endif //!UE_BUILD_SHIPPING