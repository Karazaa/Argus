// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ComponentData.h"
#include "HealthComponentData.generated.h"

UCLASS()
class ARGUS_API UHealthComponentData : public UComponentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	uint32 Health = 1000u;

	void InstantiateComponentForEntity(ArgusEntity& entity) override;
};