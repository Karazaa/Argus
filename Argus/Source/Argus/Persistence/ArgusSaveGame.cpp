// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSaveGame.h"
#include "ArgusEntity.h"

void UArgusSaveGame::Serialize(FArchive& archive)
{
	ARGUS_TRACE(UArgusSaveGame::Serialize);

	Super::Serialize(archive);

	ArgusEntity::Serialize(archive);
}