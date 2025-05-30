// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING
class ArgusECSDebugger
{
public:
	static void DrawECSDebugger();
	static bool IsEntityBeingDebugged(uint16 entityId);
	static bool ShouldShowAvoidanceDebugForEntity(uint16 entityId);
	static bool ShouldShowNavigationDebugForEntity(uint16 entityId);
	static bool ShouldIgnoreTeamRequirementsForSelectingEntities() { return s_ignoreTeamRequirementsForSelectingEntities; };

private:
	static bool s_onlyDebugSelectedEntities;
	static bool s_ignoreTeamRequirementsForSelectingEntities;
	static bool s_entityDebugToggles[ArgusECSConstants::k_maxEntities];
	static bool s_entityShowAvoidanceDebug[ArgusECSConstants::k_maxEntities];
	static bool s_entityShowNavigationDebug[ArgusECSConstants::k_maxEntities];

	static void DrawEntityScrollRegion();
	static void DrawEntityScrollRegionMenuItems();
	static void DrawCurrentlySelectedEntities();
	static void DrawEntityDockSpace();
	static void DrawWindowForEntity(uint16 entityId);

	static void ClearAllEntityDebugWindows();
};
#endif //!UE_BUILD_SHIPPING
