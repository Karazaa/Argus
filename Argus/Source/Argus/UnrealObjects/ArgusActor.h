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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsFlying() const;

	virtual void Reset();

	ArgusEntity GetEntity() const;
	void SetEntity(const ArgusEntity& entity);
	void SetSelectionState(bool isSelected);
	void Show();
	void Hide();
	bool IsVisible() const;
	void Update(float deltaTime, ETeam activePlayerControllerTeam);
	bool IsSeenBy(ETeam team) const;

	/**
	* Used to show animations or effects when the entity starts or stops attacking. N.B. This does not indicate whether the entity is currently in the process of attacking, just that we can start or stop visible attacking.
	*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void  ShowArgusEntityCombatState(ECombatState state);

	/**
	* Called each frame the entity has received damage. Can be used to trigger damage effects or to raise the alarm that a unit is under attack.
	*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ArgusEntityUnderAttack();
	/**
	* Returns the current waypoints in the sequence they would be visited. If there are no waypoints, an empty array is returned.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	TArray<FVector> GetCurrentWaypoints() const;
	/**
	* Used to show the current location the entity is moving towards.
	*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentTargetVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	FVector GetCurrentTargetLocation() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	AArgusActor* GetCurrentTargetActor() const;
	/**
	* Used to show the current waypoints for this actor. Use in conjuction with GetCurrentWaypoints() to get the locations
	*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentWaypointsVisible(bool bVisible);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnCursorHover(bool bHover);

	/**
	* Used to set the move to location for the actor
	* @Param the location to move to
	* @Param if we should clear the waypoints too, if we don't then the actor will move to the location then go back to following the waypoints
	*/
	UFUNCTION(BlueprintNativeEvent, BLueprintCallable)
	void SetMoveToLocation(FVector targetLocation, bool bClearWaypoints);

	/**
	* Used to set the move to actor for the actor
	* @Param the actor to move to
	* @Param if we should clear the waypoints too, if we don't then this actor will move to the target actor, attack or interact with it then go back to following the waypoints
	*/
	UFUNCTION(BlueprintNativeEvent, BLueprintCallable)
	void SetMoveToActor(AActor* targetActor, bool bClearWaypoints);

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
	void OnTakeOff();

	UFUNCTION(BlueprintImplementableEvent)
	void OnLand();

	UFUNCTION(BlueprintImplementableEvent)
	void OnArgusEntityPassengerStateChanged(bool isPassenger);

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
	virtual void OnChanged_m_flightState(EFlightState oldValue, EFlightState newValue) override;
	virtual void OnChanged_m_carrierEntityId(uint16 oldValue, uint16 newValue) override;

	void InitializeWidgets();
	void UpdateUIWidgetComponentLocation();
	void FixupTransformForFlying();

private:
	ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
	bool m_isSelected = false;
	TWeakObjectPtr<UArgusActorInfoView> m_argusActorInfoWidget = nullptr;
};
