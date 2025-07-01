// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "SpawnQueueWidget.generated.h"

class ArgusEntity;

UCLASS()
class USpawnQueueWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	virtual void RefreshDisplay(const ArgusEntity& selectedEntity);
};