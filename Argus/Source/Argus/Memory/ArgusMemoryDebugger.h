// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING
class ArgusMemoryDebugger
{
public:
	static void DrawMemoryDebugger();
};
#endif //!UE_BUILD_SHIPPING