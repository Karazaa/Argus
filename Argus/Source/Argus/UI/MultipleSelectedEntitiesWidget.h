// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "MultipleSelectedEntitiesWidget.generated.h"

UCLASS()
class UMultipleSelectedEntitiesWidget : public UArgusUserWidget
{
	GENERATED_BODY()

	virtual void OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity) override;
};