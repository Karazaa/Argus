// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Argus_STTask_Base.h"
#include "ArgusAIController.h"

#include "CheckRequiredEntities.generated.h"


/**
 * 
 */
UCLASS()
class ARGUS_API UCheckRequiredEntities : public UArgus_STTask_Base
{
	GENERATED_BODY()

public:

	void OnArgusStartTask() override;
	
};
