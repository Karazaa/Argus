// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ComponentDefinitions/IdentityComponent.h"
#include "ArgusActor.h"
#include "ArgusController.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UArgusController : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARGUS_API IArgusController
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual ETeam GetControlledTeam();

	virtual bool HasRequiredEntities();

	virtual TArray<TSubclassOf<AArgusActor>> GetRequredEntityClasses();

};
