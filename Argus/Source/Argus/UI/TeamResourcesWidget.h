// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "TeamResourcesWidget.generated.h"

UCLASS()
class UTeamResourcesWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	virtual void UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams) override;

};