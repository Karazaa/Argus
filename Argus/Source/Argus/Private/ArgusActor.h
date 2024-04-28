// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArgusEntityTemplate.h"
#include "ArgusActor.generated.h"

UCLASS()
class AArgusActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AArgusActor();

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TObjectPtr<UArgusEntityTemplate> m_entityTemplate = nullptr;

	virtual void BeginPlay() override;
	virtual void Tick(float deltaTime) override;

private:
	ArgusEntity m_entity = ArgusEntity::s_emptyEntity;
};
