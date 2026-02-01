// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#if !UE_BUILD_SHIPPING
#include "ArgusECSConstants.h"
#include "CoreMinimal.h"
#include <string>

class ArgusECSDebugger
{
public:
	static void DrawECSDebugger();
	static bool IsEntityBeingDebugged(uint16 entityId);
	static bool ShouldShowAvoidanceDebugForEntity(uint16 entityId);
	static bool ShouldShowNavigationDebugForEntity(uint16 entityId);
	static bool ShouldShowFlockingDebugForEntity(uint16 entityId);
	static bool ShouldIgnoreTeamRequirementsForSelectingEntities() { return s_ignoreTeamRequirementsForSelectingEntities; };
	static bool ShouldDrawFogOfWar() { return s_shouldDrawFogOfWar; }

private:
	static bool s_shouldDrawFogOfWar;
	static bool s_onlyDebugSelectedEntities;
	static bool s_ignoreTeamRequirementsForSelectingEntities;
	static bool s_entityDebugToggles[ArgusECSConstants::k_maxEntities];
	static bool s_entityShowAvoidanceDebug[ArgusECSConstants::k_maxEntities];
	static bool s_entityShowNavigationDebug[ArgusECSConstants::k_maxEntities];
	static bool s_entityShowFlockingDebug[ArgusECSConstants::k_maxEntities];
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
};
#endif //!UE_BUILD_SHIPPING
