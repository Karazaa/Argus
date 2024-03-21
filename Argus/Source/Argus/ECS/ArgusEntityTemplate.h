// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusECSConstants.h"
#include "DataComponentDefinitions/ComponentData.h"
#include "ArgusEntityTemplate.generated.h"

UCLASS()
class UArgusEntityTemplate : public UDataAsset 
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UEntityPriority EntityPriority;

	UPROPERTY(EditAnywhere)
	TArray<UComponentData*> ComponentData;
};