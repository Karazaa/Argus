// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "GameFramework/SaveGame.h"
#include "Misc/DateTime.h"
#include "ArgusMetadataSaveGame.generated.h"

USTRUCT()
struct FSaveSlotMetadata
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FDateTime m_saveTimestamp;
};

UCLASS()
class UArgusMetadataSaveGame : public USaveGame
{
	GENERATED_BODY()

private:
	UPROPERTY(SaveGame)
	TArray<FSaveSlotMetadata> m_saveSlotMetadata;

	UPROPERTY(SaveGame)
	FString m_mostRecentSaveName;

	friend class UArgusSaveManager;
};