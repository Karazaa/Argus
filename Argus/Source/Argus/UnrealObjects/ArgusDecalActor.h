// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "ArgusDecalActor.generated.h"

UCLASS()
class AArgusDecalActor : public AArgusActor
{
	GENERATED_BODY()

public:
	void SetEntity(ArgusEntity entity) override;
};