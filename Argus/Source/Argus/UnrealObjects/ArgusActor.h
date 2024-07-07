// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArgusEntityTemplate.h"
#include "ArgusActor.generated.h"

class UFactionRecord;

UCLASS()
class AArgusActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AArgusActor();
	ArgusEntity GetEntity();

	void SetSelectionState(bool isSelected);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnSelected();
	UFUNCTION(BlueprintImplementableEvent)
	void OnDeselected();
	UFUNCTION(BlueprintImplementableEvent)
	void OnPopulateFaction(const UFactionRecord* factionRecord);

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TObjectPtr<UArgusEntityTemplate> m_entityTemplate = nullptr;

	virtual void BeginPlay() override;
	virtual void Tick(float deltaTime) override;

private:
	ArgusEntity m_entity = ArgusEntity::s_emptyEntity;
	bool m_isSelected = false;
};
