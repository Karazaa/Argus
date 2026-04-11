// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "GameFramework/SaveGame.h"
#include "ArgusSaveGame.generated.h"

UCLASS()
class UArgusSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	virtual void Serialize(FArchive& archive) override;
};