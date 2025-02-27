// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ComponentDependencies/ResourceSet.h"
#include "ResourceWidget.generated.h"

UCLASS()
class UResourceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateDisplay(EResourceType resourceType, int32 quantity);
};