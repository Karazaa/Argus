// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "DataComponentDefinitions/ComponentData.h"
#include "ArgusEntityTemplate.generated.h"

UCLASS()
class UArgusEntityTemplate : public UDataAsset 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<UComponentData*> m_componentData;
};