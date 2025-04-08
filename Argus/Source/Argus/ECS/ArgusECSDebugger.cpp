// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusECSDebugger.h"
#include "ArgusEntity.h"
#include "imgui.h"

#if !UE_BUILD_SHIPPING

static TAutoConsoleVariable<bool> CVarDrawECSDebugger(TEXT("Argus.Debug.ECS"), false, TEXT("Whether or not the ECS ImGui debugger should be drawn."));
bool ArgusECSDebugger::s_onlyDebugSelectedEntities = false;
bool ArgusECSDebugger::s_ignoreTeamRequirementsForSelectingEntities = false;
bool ArgusECSDebugger::s_entityDebugToggles[ArgusECSConstants::k_maxEntities];
bool ArgusECSDebugger::s_entityShowAvoidanceDebug[ArgusECSConstants::k_maxEntities];
bool ArgusECSDebugger::s_entityShowNavigationDebug[ArgusECSConstants::k_maxEntities];

void ArgusECSDebugger::DrawECSDebugger()
{
	if (!CVarDrawECSDebugger.GetValueOnGameThread())
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
	if (!ImGui::Begin("ECS"))
	{
		ImGui::End();
		return;
	}

	DrawEntityScrollRegion();
	DrawEntityDockSpace();

	ImGui::End();
}

bool ArgusECSDebugger::IsEntityBeingDebugged(uint16 entityId)
{
	return s_entityDebugToggles[entityId];
}

bool ArgusECSDebugger::ShouldShowAvoidanceDebugForEntity(uint16 entityId)
{
	return s_entityShowAvoidanceDebug[entityId];
}

bool ArgusECSDebugger::ShouldShowNavigationDebugForEntity(uint16 entityId)
{
	return s_entityShowNavigationDebug[entityId];
}

void ArgusECSDebugger::DrawEntityScrollRegion()
{
	ImGui::Checkbox("Only debug selected entities", &s_onlyDebugSelectedEntities);
	ImGui::SameLine();
	ImGui::Checkbox("Ignore team requirements for selection", &s_ignoreTeamRequirementsForSelectingEntities);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar;
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::BeginChild("EntitiesScrollRegion", ImVec2(0, 0), ImGuiChildFlags_Borders, window_flags);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Entities"))
		{
			DrawEntityScrollRegionMenuItems();
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	if (ImGui::BeginTable("EntitiesTable", 4, ImGuiTableFlags_NoSavedSettings))
	{
		for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
		{
			ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
			if (!entity)
			{
				continue;
			}

			if (s_onlyDebugSelectedEntities)
			{
				s_entityDebugToggles[i] = entity.IsSelected();
			}

			char buf[32];
			sprintf_s(buf, "%d", i);
			ImGui::TableNextColumn();
			ImGui::Selectable(buf, &s_entityDebugToggles[i], ImGuiSelectableFlags_None);
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();
}

void ArgusECSDebugger::DrawEntityScrollRegionMenuItems()
{
	if (ImGui::MenuItem("Clear All Entity Debug Windows"))
	{
		ClearAllEntityDebugWindows();
	}

	if (ImGui::MenuItem("Debug Currently Selected Entities"))
	{
		DrawCurrentlySelectedEntities();
	}
}

void ArgusECSDebugger::DrawCurrentlySelectedEntities()
{
	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		return;
	}

	const InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		return;
	}

	for (int32 i = 0; i < inputInterfaceComponent->m_selectedArgusEntityIds.Num(); ++i)
	{
		s_entityDebugToggles[inputInterfaceComponent->m_selectedArgusEntityIds[i]] = true;
	}
}

void ArgusECSDebugger::DrawEntityDockSpace()
{
	ImGui::SetNextWindowSize(ImVec2(260, 260), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Entity Windows"))
	{
		ImGui::End();
		return;
	}

	ImGuiID dockspaceId = ImGui::GetID("Entity Dock Space");
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		if (!ArgusEntity::DoesEntityExist(i) || !s_entityDebugToggles[i])
		{
			continue;
		}

		ImGui::SetNextWindowDockID(dockspaceId, ImGuiCond_Once);
		DrawWindowForEntity(i);
	}

	ImGui::End();
}

void ArgusECSDebugger::DrawWindowForEntity(uint16 entityId)
{
	char buf[32];
	sprintf_s(buf, "%d", entityId);

	ImGui::SetNextWindowSize(ImVec2(260, 260), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(buf))
	{
		ImGui::End();
		return;
	}

	ImGui::Checkbox("Show Avoidance debug", &s_entityShowAvoidanceDebug[entityId]);
	ImGui::SameLine();
	ImGui::Checkbox("Show Navigation debug", &s_entityShowNavigationDebug[entityId]);

	ImGui::SeparatorText("Components");
	ArgusComponentRegistry::DrawComponentsDebug(entityId);

	ImGui::End();
}

void ArgusECSDebugger::ClearAllEntityDebugWindows()
{
	for (uint16 i = 0u; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		s_entityDebugToggles[i] = false;
	}
}

#endif //!UE_BUILD_SHIPPING