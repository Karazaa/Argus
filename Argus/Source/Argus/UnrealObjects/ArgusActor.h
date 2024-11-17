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

	virtual void Reset();

	ArgusEntity GetEntity() const;
	void SetEntity(const ArgusEntity& entity);
	void SetSelectionState(bool isSelected);
	void Show();
	void Hide();
	bool IsVisible() const;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnSelected();
	UFUNCTION(BlueprintImplementableEvent)
	void OnDeselected();
	UFUNCTION(BlueprintImplementableEvent)
	void OnPopulateFaction(const UFactionRecord* factionRecord);
	UFUNCTION(BlueprintImplementableEvent)
	void OnPopulateTeam(FColor teamColor);

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TObjectPtr<UArgusEntityTemplate> m_entityTemplate = nullptr;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	virtual void Tick(float deltaTime) override;

private:
	ArgusEntity m_entity = ArgusEntity::s_emptyEntity;
	bool m_isSelected = false;
};
