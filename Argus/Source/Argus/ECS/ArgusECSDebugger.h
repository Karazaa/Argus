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

private:
	static bool s_entityDebugToggles[ArgusECSConstants::k_maxEntities];

	static void DrawEntityScrollRegion();
	static void DrawEntityScrollRegionMenuItems();
	static void DrawCurrentlySelectedEntities();
	static void DrawEntityDockSpace();
	static void DrawWindowForEntity(uint16 entityId);

	static void ClearAllEntityDebugWindows();
};
#endif //!UE_BUILD_SHIPPING
