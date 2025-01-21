// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArgusEntityTemplate.h"
#include "ArgusActor.generated.h"

class UArgusActorInfoWidget;
class UArgusActorRecord;
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
	TSoftClassPtr<UArgusActorInfoWidget> m_argusActorInfoWidgetClass = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UArgusActorRecord> m_argusActorRecord = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool m_shouldActorSpawnLocationSetEntityLocation = false;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	virtual void Tick(float deltaTime) override;

	void InitializeWidgets();

private:
	ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
	bool m_isSelected = false;
	TWeakObjectPtr<UArgusActorInfoWidget> m_argusActorInfoWidget = nullptr;
};
