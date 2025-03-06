// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntityTemplate.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArgusActor.generated.h"

class UArgusActorInfoWidget;
class UArgusActorRecord;
class UFactionRecord;

UCLASS()
class AArgusActor : public AActor, public ITaskComponentObserver
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

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float m_uiWidgetOffsetDistance = 50.0f;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	virtual void Tick(float deltaTime) override;
	virtual void OnChanged_m_baseState(BaseState oldValue, BaseState newValue) {};

	void InitializeWidgets();
	void UpdateUIWidgetComponentLocation();

private:
	ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
	bool m_isSelected = false;
	TWeakObjectPtr<UArgusActorInfoWidget> m_argusActorInfoWidget = nullptr;
};
