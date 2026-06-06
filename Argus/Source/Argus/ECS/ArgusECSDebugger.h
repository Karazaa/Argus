// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSConstants.h"
#include "ArgusSetAllocator.h"
#include "Containers/BitArray.h"
#include "ComponentDependencies/Teams.h"
#include <string>

class FArchive;

class ArgusECSDebugger
{
public:
	static void DrawECSDebugger();
	static bool IsEntityBeingDebugged(uint16 entityId) { return HasEntityDebugFlag(entityId, EntityDebugFlag::ShowDebugMenu); }
	static bool ShouldShowAvoidanceDebugForEntity(uint16 entityId) { return HasEntityDebugFlag(entityId, EntityDebugFlag::ShowAvoidanceDebug); }
	static bool ShouldShowGroupDebugForEntity(uint16 entityId) { return HasEntityDebugFlag(entityId, EntityDebugFlag::ShowGroupDebug); }
	static bool ShouldShowNavigationDebugForEntity(uint16 entityId) { return HasEntityDebugFlag(entityId, EntityDebugFlag::ShowNavigationDebug); }
	static bool ShouldShowFlockingDebugForEntity(uint16 entityId) { return HasEntityDebugFlag(entityId, EntityDebugFlag::ShowGroupLeaderFlockingDisplay); }
	static bool ShouldShowIdDebugForEntity(uint16 entityId) { return HasEntityDebugFlag(entityId, EntityDebugFlag::ShowIdDebug); };
	static bool ShouldIgnoreTeamRequirementsForSelectingEntities() { return s_ignoreTeamRequirementsForSelectingEntities; };
	static bool ShouldDrawFogOfWar() { return s_shouldDrawFogOfWar; }
	static bool IsTeamAIEnabled() { return s_isTeamAIEnabled; }

	static void Serialize(FArchive& archive);

private:
	enum class EntityDebugFlag : uint8
	{
		ShowDebugMenu = 1 << 0,
		ShowAvoidanceDebug = 1 << 1,
		ShowGroupDebug = 1 << 2,
		ShowNavigationDebug = 1 << 3,
		ShowFlockingDebug = 1 << 4,
		ShowGroupLeaderFlockingDisplay = 1 << 5,
		ShowIdDebug = 1 << 6
	};

	static bool s_shouldDrawFogOfWar;
	static bool s_onlyDebugSelectedEntities;
	static bool s_ignoreTeamRequirementsForSelectingEntities;
	static bool s_isTeamAIEnabled;

	static uint8 s_entityDebugFlags[ArgusECSConstants::k_maxEntities];
	static bool s_teamEntityShowRevealedAreaDebug[sizeof(ETeam) * 8];
	static int	s_teamToApplyResourcesTo;
	static TArray<std::string> s_resourceToAddStrings;

	static void DrawEntityScrollRegion();
	static void DrawSelectableEntityScrollRegion(int windowFlags, int childFlags, int tableWidth);
	static void DrawTeamEntityScrollRegion(int windowFlags, int childFlags, int tableWidth);
	static void DrawSingletonEntityScrollRegion(int windowFlags, int childFlags);

	static void DrawEntityScrollRegionMenuItems();
	static void DrawCurrentlySelectedEntities();
	static void DrawEntityDockSpace();
	static void DrawWindowForEntity(uint16 entityId);
	static void DrawResourceRegion();

	static void ClearAllEntityDebugWindows();

	static void SetEntityDebugFlag(uint16 entityId, EntityDebugFlag debugFlag);
	static void UnsetEntityDebugFlag(uint16 entityId, EntityDebugFlag debugFlag);
	static bool HasEntityDebugFlag(uint16 entityId, EntityDebugFlag debugFlag);
	static bool EntityDebugFlagCheckbox(const char* label, uint16 entityId, EntityDebugFlag debugFlag);
	static bool EntityDebugFlagSelectable(const char* label, uint16 entityId, EntityDebugFlag debugFlag);
};
#endif //!UE_BUILD_SHIPPING
