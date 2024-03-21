// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "../ArgusEntity.h"
#include "ComponentData.generated.h"

UCLASS()
class UComponentData : public UDataAsset
{
	GENERATED_BODY()

public:
	virtual void InstantiateComponentForEntity(ArgusEntity& entity) {};
};