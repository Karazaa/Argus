// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusECSDebugger.h"

#if !UE_BUILD_SHIPPING
#include "ArgusIterators.h"
#include "ArgusMacros.h"
#include "ComponentDependencies/ResourceSet.h"
#include "imgui.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/ResourceSystems.h"
#include "Systems/TeamCommanderSystems.h"

static TAutoConsoleVariable<bool> CVarDrawECSDebugger(TEXT("Argus.Debug.ECS"), false, TEXT("Whether or not the ECS ImGui debugger should be drawn."));

bool ArgusECSDebugger::s_shouldDrawFogOfWar = true;
bool ArgusECSDebugger::s_onlyDebugSelectedEntities = false;
bool ArgusECSDebugger::s_ignoreTeamRequirementsForSelectingEntities = false;
bool ArgusECSDebugger::s_isTeamAIEnabled = true;
uint8 ArgusECSDebugger::s_entityDebugFlags[ArgusECSConstants::k_maxEntities];
bool ArgusECSDebugger::s_teamEntityShowRevealedAreaDebug[sizeof(ETeam) * 8];
int  ArgusECSDebugger::s_teamToApplyResourcesTo = 0;
TArray<std::string> ArgusECSDebugger::s_resourceToAddStrings;

void ArgusECSDebugger::DrawECSDebugger()
{
	ARGUS_TRACE(ArgusECSDebugger::DrawECSDebugger);

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

void ArgusECSDebugger::Serialize(FArchive& archive)
{
	archive << s_shouldDrawFogOfWar;
	archive << s_onlyDebugSelectedEntities;
	archive << s_ignoreTeamRequirementsForSelectingEntities;
	archive << s_isTeamAIEnabled;
	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		archive << s_entityDebugFlags[i];
	}
}

void ArgusECSDebugger::DrawEntityScrollRegion()
{
	DrawResourceRegion();

	ImGui::Checkbox("Draw fog of war", &s_shouldDrawFogOfWar);
	ImGui::SameLine();
	ImGui::Checkbox("Is team AI enabled", &s_isTeamAIEnabled);
	ImGui::Checkbox("Only debug selected entities", &s_onlyDebugSelectedEntities);
	ImGui::SameLine();
	ImGui::Checkbox("Ignore team requirements for selection", &s_ignoreTeamRequirementsForSelectingEntities);

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar;
	int childFlags = ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY;
	int tableWidth = 8;
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

	DrawSelectableEntityScrollRegion(windowFlags, childFlags, tableWidth);
	DrawTeamEntityScrollRegion(windowFlags, childFlags, tableWidth);
	DrawSingletonEntityScrollRegion(windowFlags, childFlags);

	ImGui::PopStyleVar();
}

void ArgusECSDebugger::DrawSelectableEntityScrollRegion(int windowFlags, int childFlags, int tableWidth)
{
	ImGui::BeginChild("EntitiesScrollRegion", ImVec2(0, 0), childFlags, windowFlags);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Entities"))
		{
			DrawEntityScrollRegionMenuItems();
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	if (ImGui::BeginTable("EntitiesTable", tableWidth, ImGuiTableFlags_NoSavedSettings))
	{
		for (uint16 i = 0u; i <= ArgusEntity::GetHighestNonReservedEntityId(); ++i)
		{
			ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
			if (!entity)
			{
				UnsetEntityDebugFlag(i, EntityDebugFlag::ShowDebugMenu);
				continue;
			}

			if (s_onlyDebugSelectedEntities)
			{
				if (entity.IsSelected())
				{
					SetEntityDebugFlag(i, EntityDebugFlag::ShowDebugMenu);
				}
				else
				{
					UnsetEntityDebugFlag(i, EntityDebugFlag::ShowDebugMenu);
				}
			}

			char buf[32];
			sprintf_s(buf, "%d", i);
			ImGui::TableNextColumn();
			EntityDebugFlagSelectable(buf, i, EntityDebugFlag::ShowDebugMenu);
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void ArgusECSDebugger::DrawTeamEntityScrollRegion(int windowFlags, int childFlags, int tableWidth)
{
	ImGui::BeginChild("TeamEntitiesScrollRegion", ImVec2(0, 0), childFlags, windowFlags);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Team Entities"))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	if (ImGui::BeginTable("TeamEntitiesTable", tableWidth, ImGuiTableFlags_NoSavedSettings))
	{
		ArgusIterators::IterateTeamEntities([](ArgusEntity teamEntity)
			{
				const TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
				if (!teamCommanderComponent)
				{
					return;
				}

				const char* teamName = ARGUS_FSTRING_TO_CHAR(StaticEnum<ETeam>()->GetNameStringByValue(static_cast<uint8>(teamCommanderComponent->m_teamToCommand)));
				ImGui::TableNextColumn();
				EntityDebugFlagSelectable(teamName, teamEntity.GetId(), EntityDebugFlag::ShowDebugMenu);
			});
		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void ArgusECSDebugger::DrawSingletonEntityScrollRegion(int windowFlags, int childFlags)
{
	ImGui::BeginChild("SingletonEntityScrollRegion", ImVec2(0, 0), childFlags, windowFlags);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Singleton Entity"))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	EntityDebugFlagSelectable("Singleton Entity", ArgusECSConstants::k_singletonEntityId, EntityDebugFlag::ShowDebugMenu);
	ImGui::EndChild();
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
		SetEntityDebugFlag(inputInterfaceComponent->m_selectedArgusEntityIds[i], EntityDebugFlag::ShowDebugMenu);
	}
}

void ArgusECSDebugger::DrawEntityDockSpace()
{
	ImGui::SetNextWindowSize(ImVec2(260, 260), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Entity Info"))
	{
		ImGui::End();
		return;
	}

	ImGuiID dockspaceId = ImGui::GetID("Entity Dock Space");
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	for (uint16 i = 0u; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		UnsetEntityDebugFlag(i, EntityDebugFlag::ShowGroupLeaderFlockingDisplay);
		if (!ArgusEntity::DoesEntityExist(i) || !HasEntityDebugFlag(i, EntityDebugFlag::ShowDebugMenu))
		{
			UnsetEntityDebugFlag(i, EntityDebugFlag::ShowAvoidanceDebug);
			UnsetEntityDebugFlag(i, EntityDebugFlag::ShowGroupDebug);
			UnsetEntityDebugFlag(i, EntityDebugFlag::ShowNavigationDebug);
			UnsetEntityDebugFlag(i, EntityDebugFlag::ShowFlockingDebug);
			UnsetEntityDebugFlag(i, EntityDebugFlag::ShowIdDebug);
			continue;
		}

		ImGui::SetNextWindowDockID(dockspaceId, ImGuiCond_Once);
		DrawWindowForEntity(i);
	}

	ImGui::End();
}

void ArgusECSDebugger::DrawWindowForEntity(uint16 entityId)
{
	ImGui::SetNextWindowSize(ImVec2(260, 260), ImGuiCond_FirstUseEver);

	const char* name = nullptr;
	char buf[32];
	bool isSelectableEntity = false;
	bool hasRevealedAreas = false;
	uint16 offset = 0u;

	ArgusEntity entity = ArgusEntity::RetrieveEntity(entityId);
	if (entityId == ArgusECSConstants::k_singletonEntityId)
	{
		name = "Singleton";
	}
	else if (ArgusEntity::IsReservedEntityId(entityId))
	{
		const TeamCommanderComponent* teamCommanderComponent = entity.GetComponent<TeamCommanderComponent>();
		if (teamCommanderComponent)
		{
			name = ARGUS_FSTRING_TO_CHAR(StaticEnum<ETeam>()->GetNameStringByValue(static_cast<uint8>(teamCommanderComponent->m_teamToCommand)));
			offset = ArgusEntity::GetTeamOffset(teamCommanderComponent->m_teamToCommand);
			hasRevealedAreas = true;
		}
		else
		{
			sprintf_s(buf, "%d", entityId);
			name = buf;
		}
	}
	else
	{
		sprintf_s(buf, "%d", entityId);
		name = buf;
		isSelectableEntity = true;
	}

	if (isSelectableEntity && HasEntityDebugFlag(entityId, EntityDebugFlag::ShowFlockingDebug))
	{
		if (ArgusEntity groupLeaderEntity = AvoidanceSystems::GetAvoidanceGroupLeader(entity))
		{
			SetEntityDebugFlag(groupLeaderEntity.GetId(), EntityDebugFlag::ShowGroupLeaderFlockingDisplay);
		}
	}

	if (!ImGui::Begin(name))
	{
		ImGui::End();
		return;
	}

	if (isSelectableEntity)
	{
		EntityDebugFlagCheckbox("Show ID", entityId, EntityDebugFlag::ShowIdDebug);
		ImGui::SameLine();
		EntityDebugFlagCheckbox("Show Group debug", entityId, EntityDebugFlag::ShowGroupDebug);
		ImGui::SameLine();
		EntityDebugFlagCheckbox("Show Avoidance debug", entityId, EntityDebugFlag::ShowAvoidanceDebug);
		EntityDebugFlagCheckbox("Show Navigation debug", entityId, EntityDebugFlag::ShowNavigationDebug);
		ImGui::SameLine();
		EntityDebugFlagCheckbox("Show Flocking debug", entityId, EntityDebugFlag::ShowFlockingDebug);
	}

	if (hasRevealedAreas)
	{
		ImGui::Checkbox("Show Revealed Areas debug", &s_teamEntityShowRevealedAreaDebug[offset]);
		if (s_teamEntityShowRevealedAreaDebug[offset])
		{
			TeamCommanderSystems::DebugRevealedAreasForTeamEntityId(entityId);
		}
	}

	ImGui::SeparatorText("Components");
	ArgusComponentRegistry::DrawComponentsDebug(entityId);

	ImGui::End();
}

void ArgusECSDebugger::DrawResourceRegion()
{
	if (!ImGui::CollapsingHeader("Team Resources"))
	{
		return;
	}

	const uint8 numResources = static_cast<uint8>(EResourceType::Count);
	const uint8 numDebugStrings = static_cast<uint8>(s_resourceToAddStrings.Num());
	if (numDebugStrings != numResources)
	{
		s_resourceToAddStrings.SetNumZeroed(numResources);
		for (uint8 i = 0u; i < numResources; ++i)
		{
			s_resourceToAddStrings[i] = std::string("0");
			s_resourceToAddStrings.Reserve(32);
		}
	}

	for (uint8 i = 0u; i < numResources; ++i)
	{
		const char* resourceName = ARGUS_FSTRING_TO_CHAR(StaticEnum<EResourceType>()->GetNameStringByValue(i));
		char* buffer = s_resourceToAddStrings[i].data();
		ImGui::InputText(resourceName, buffer, IM_ARRAYSIZE(buffer));
	}

	std::string teamNames[8] = {};
	const char* teamNamesCStr[8] = {};
	for (uint8 i = 0; i < 8; ++i)
	{
		const uint8 value = 1u << i;
		teamNames[i] = ARGUS_FSTRING_TO_CHAR(StaticEnum<ETeam>()->GetNameStringByValue(value));
		teamNamesCStr[i] = teamNames[i].c_str();
	}

	ImGui::Combo("Team", &s_teamToApplyResourcesTo, teamNamesCStr, IM_ARRAYSIZE(teamNamesCStr));
	ImGui::SameLine();
	if (ImGui::Button("Add Resources"))
	{
		FResourceSet resourcesToAdd;
		for (uint8 i = 0u; i < numResources; ++i)
		{
			resourcesToAdd.m_resourceQuantities[i] = std::atoi(s_resourceToAddStrings[i].c_str()); 
		}

		const ETeam teamValue = static_cast<ETeam>(1u << s_teamToApplyResourcesTo);
		ResourceSystems::ApplyTeamResourceChangeIfAffordable(teamValue, resourcesToAdd);
	}
}

void ArgusECSDebugger::ClearAllEntityDebugWindows()
{
	for (uint16 i = 0u; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		UnsetEntityDebugFlag(i, EntityDebugFlag::ShowDebugMenu);
	}
}

void ArgusECSDebugger::SetEntityDebugFlag(uint16 entityId, EntityDebugFlag debugFlag)
{
	if (!ensure(entityId < ArgusECSConstants::k_maxEntities))
	{
		return;
	}

	s_entityDebugFlags[entityId] |= static_cast<uint8>(debugFlag);
}

void ArgusECSDebugger::UnsetEntityDebugFlag(uint16 entityId, EntityDebugFlag debugFlag)
{
	if (!ensure(entityId < ArgusECSConstants::k_maxEntities))
	{
		return;
	}

	s_entityDebugFlags[entityId] &= (~static_cast<uint8>(debugFlag));
}

bool ArgusECSDebugger::HasEntityDebugFlag(uint16 entityId, EntityDebugFlag debugFlag)
{
	if (!ensure(entityId < ArgusECSConstants::k_maxEntities))
	{
		return false;
	}

	return (s_entityDebugFlags[entityId] & static_cast<uint8>(debugFlag)) != 0;
}

bool ArgusECSDebugger::EntityDebugFlagCheckbox(const char* label, uint16 entityId, EntityDebugFlag debugFlag)
{
	if (!ensure(entityId < ArgusECSConstants::k_maxEntities))
	{
		return false;
	}

	bool value = HasEntityDebugFlag(entityId, debugFlag);
	if (ImGui::Checkbox(label, &value))
	{
		if (value)
		{
			SetEntityDebugFlag(entityId, debugFlag);
		}
		else
		{
			UnsetEntityDebugFlag(entityId, debugFlag);
		}

		return true;
	}

	return false;
}

bool ArgusECSDebugger::EntityDebugFlagSelectable(const char* label, uint16 entityId, EntityDebugFlag debugFlag)
{
	if (!ensure(entityId < ArgusECSConstants::k_maxEntities))
	{
		return false;
	}

	bool value = HasEntityDebugFlag(entityId, debugFlag);
	if (ImGui::Selectable(label, &value, ImGuiSelectableFlags_None))
	{
		if (value)
		{
			SetEntityDebugFlag(entityId, debugFlag);
		}
		else
		{
			UnsetEntityDebugFlag(entityId, debugFlag);
		}

		return true;
	}

	return false;
}

#endif //!UE_BUILD_SHIPPING