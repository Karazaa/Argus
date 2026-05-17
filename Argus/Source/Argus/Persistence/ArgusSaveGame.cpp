// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSaveGame.h"
#include "ArgusEntity.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#endif

void UArgusSaveGame::Serialize(FArchive& archive)
{
	ARGUS_TRACE(UArgusSaveGame::Serialize);

	Super::Serialize(archive);

	ArgusEntity::Serialize(archive);

#if !UE_BUILD_SHIPPING
	ArgusECSDebugger::Serialize(archive);
#endif
}