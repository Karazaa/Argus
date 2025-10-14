// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntityTemplate.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArgusActor.generated.h"

class UArgusActorInfoView;
class UArgusActorRecord;
class UFactionRecord;
class UPlacedArgusActorTeamInfoRecord;

UCLASS()
class AArgusActor : public AActor, public IPassengerComponentObserver, public ITaskComponentObserver
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
	void Update(float deltaTime, ETeam activePlayerControllerTeam);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnSelected();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDeselected();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPopulateFaction(const UFactionRecord* factionRecord);

	UFUNCTION(BlueprintImplementableEvent)
	void OnPopulateTeam(FColor teamColor);

	UFUNCTION(BlueprintImplementableEvent)
	void OnArgusEntityDeath();

	UFUNCTION(BlueprintImplementableEvent)
	void OnArgusEntityPassengerStateChanged(bool isPassenger);

	/**
	* Used to show animations or effects when the entity starts or stops attacking. N.B. This does not indicate whether the entity is currently in the process of attacking, just that we can start or stop visible attacking.
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void  OnArgusEntityCombatStateChanged(ECombatState state);

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftClassPtr<UArgusActorInfoView> m_argusActorInfoWidgetClass = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UArgusActorRecord> m_argusActorRecord = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UPlacedArgusActorTeamInfoRecord> m_initialTeamInfoRecord = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool m_shouldActorSpawnLocationSetEntityLocation = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float m_uiWidgetOffsetDistance = 50.0f;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	virtual void OnChanged_m_baseState(EBaseState oldValue, EBaseState newValue) override;
	virtual void OnChanged_m_carrierEntityId(uint16 oldValue, uint16 newValue) override;

	void InitializeWidgets();
	void UpdateUIWidgetComponentLocation();

private:
	ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
	bool m_isSelected = false;
	TWeakObjectPtr<UArgusActorInfoView> m_argusActorInfoWidget = nullptr;
};
