// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActor.h"
#include "ArgusCameraActor.h"
#include "ArgusGameInstance.h"
#include "ArgusGameModeBase.h"
#include "ArgusMath.h"
#include "ArgusStaticData.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Systems/FogOfWarSystems.h"
#include "Views/ArgusActorInfoView.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#endif //!UE_BUILD_SHIPPING

AArgusActor::AArgusActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(FName("RootSceneComponent")));
}

bool AArgusActor::IsFlying() const
{
	if (!m_entity)
	{
		return false;
	}

	return m_entity.IsFlying();
}

void AArgusActor::Reset()
{
	ARGUS_TRACE(AArgusActor::Reset);

	if (m_entity)
	{
		if (ObserversComponent* observersComponent = m_entity.GetComponent<ObserversComponent>())
		{
			observersComponent->m_TaskComponentObservers.RemoveObserver(this);
			if (PassengerComponent* passengerComponent = m_entity.GetComponent<PassengerComponent>())
			{
				observersComponent->m_PassengerComponentObservers.RemoveObserver(this);
			}
		}

		if (const UWorld* world = GetWorld())
		{
			if (UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>())
			{
				gameInstance->DeregisterArgusEntityActor(this);
			}
		}
	}

	m_argusActorRecord = nullptr;
	m_initialTeamInfoRecord = nullptr;
	SetSelectionState(false);
	m_entity = ArgusEntity::k_emptyEntity;
	m_shouldActorSpawnLocationSetEntityLocation = false;

	Hide();
}

ArgusEntity AArgusActor::GetEntity() const
{
	return m_entity;
}

void AArgusActor::SetEntity(ArgusEntity entity)
{
	ARGUS_TRACE(AArgusActor::SetEntity);

	const UWorld* world = GetWorld();
	if (!world)
	{
		return;
	}

	UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		return;
	}

	if (!entity)
	{
		if (m_entity)
		{
			gameInstance->DeregisterArgusEntityActor(this);
		}
		m_entity = entity;
		return;
	}
	m_entity = entity;
	gameInstance->RegisterArgusEntityActor(this);

	if (ObserversComponent* observersComponent = m_entity.GetComponent<ObserversComponent>())
	{
		observersComponent->m_TaskComponentObservers.AddObserver(this);
		if (PassengerComponent* passengerComponent = m_entity.GetComponent<PassengerComponent>())
		{
			observersComponent->m_PassengerComponentObservers.AddObserver(this);
		}
	}

	if (TransformComponent* transformComponent = m_entity.GetComponent<TransformComponent>())
	{
		if (m_shouldActorSpawnLocationSetEntityLocation)
		{
			transformComponent->m_location = GetActorLocation();

			if (FacingComponent* facingComponent = m_entity.GetComponent<FacingComponent>())
			{
				const float desiredFacing = ArgusMath::GetYawFromDirection(GetActorForwardVector());
				facingComponent->m_targetFacing = desiredFacing;
				facingComponent->m_smoothedFacing.Reset(desiredFacing);
			}
			FixupTransformForFlying();
		}
		else
		{
			float currentFacing = 0.0f;
			if (FacingComponent* facingComponent = m_entity.GetComponent<FacingComponent>())
			{
				currentFacing = facingComponent->GetCurrentFacing();
			}
			SetActorLocationAndRotation(transformComponent->m_location, FRotator(0.0f, ArgusMath::GetUEYawDegreesFromYaw(currentFacing), 0.0f));
		}
	}

	AArgusGameModeBase* gameMode = Cast<AArgusGameModeBase>(world->GetAuthGameMode());
	if (!gameMode)
	{
		return;
	}

	if (const IdentityComponent* identityComponent = m_entity.GetComponent<IdentityComponent>())
	{
		if (const UFactionRecord* factionRecord = ArgusStaticData::GetRecord<UFactionRecord>(identityComponent->m_factionId))
		{
			OnPopulateFaction(factionRecord);
		}

		OnPopulateTeam(gameMode->GetTeamColor(identityComponent->m_team));
	}

	InitializeWidgets();
	CallEventsForInitialState();
}

void AArgusActor::SetSelectionState(bool isSelected)
{
	if (isSelected == m_isSelected)
	{
		return;
	}

	m_isSelected = isSelected;

	if (m_isSelected)
	{
		OnSelected();
	}
	else
	{
		OnDeselected();
	}
}

void AArgusActor::Show()
{
	if (IsVisible())
	{
		return;
	}

	RootComponent->SetVisibility(true, true);
}

void AArgusActor::Hide()
{
	if (!IsVisible())
	{
		return;
	}

	RootComponent->SetVisibility(false, true);
}

bool AArgusActor::IsVisible() const
{
	return RootComponent->IsVisible();
}

void AArgusActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (m_argusActorRecord.IsNull())
	{
		return;
	}

	const UArgusActorRecord* loadedArgusActorRecord = m_argusActorRecord.LoadSynchronous();
	if (!loadedArgusActorRecord)
	{
		return;
	}

	const UArgusEntityTemplate* loadedEntityTemplate = loadedArgusActorRecord->m_entityTemplate.LoadAndStorePtr();
	if (!loadedEntityTemplate)
	{
		return;
	}

	m_entity = loadedEntityTemplate->MakeEntity();
	if (!m_entity)
	{
		return;
	}

	if (const UPlacedArgusActorTeamInfoRecord* loadedTeamInfoRecord = m_initialTeamInfoRecord.LoadSynchronous())
	{
		if (IdentityComponent* identityComponent = m_entity.GetComponent<IdentityComponent>())
		{
			identityComponent->m_team = loadedTeamInfoRecord->m_team;
			ArgusEntity::RegisterTeam(identityComponent->m_team);
		}
	}

	if (TaskComponent* entityTaskComponent = m_entity.GetComponent<TaskComponent>())
	{
		entityTaskComponent->m_spawnedFromArgusActorRecordId = loadedArgusActorRecord->m_id;
	}
	
	SetEntity(m_entity);
}

void AArgusActor::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);

	if (!m_entity)
	{
		return;
	}

	const UWorld* world = GetWorld();
	if (!world)
	{
		return;
	}

	UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		return;
	}

	gameInstance->DeregisterArgusEntityActor(this);
}

void AArgusActor::Update(float deltaTime)
{
	ARGUS_TRACE(AArgusActor::Update);

	if (!m_entity)
	{
		return;
	}

	bool hasMovedOrRotated = false;
	if (const TransformComponent* transformComponent = m_entity.GetComponent<TransformComponent>())
	{
		float currentFacing = 0.0f;
		if (FacingComponent* facingComponent = m_entity.GetComponent<FacingComponent>())
		{
			currentFacing = facingComponent->GetCurrentFacing();
		}

		const FRotator facingRotation = FRotator(0.0f, ArgusMath::GetUEYawDegreesFromYaw(currentFacing), 0.0f);
		hasMovedOrRotated = (!IsLocationNearlyActorLocation(transformComponent->m_location)) || (!IsRotationNearlyActorRotation(facingRotation));

		if (hasMovedOrRotated)
		{
			ARGUS_TRACE(AArgusActor::SetActorLocationAndRotation);
			SetActorLocationAndRotation(transformComponent->m_location, facingRotation);
		}

#if !UE_BUILD_SHIPPING
		if (ArgusECSDebugger::ShouldShowIdDebugForEntity(m_entity.GetId()))
		{
			const FVector debugLocation = transformComponent->m_location + FVector(transformComponent->m_radius, 0.0f, 0.0f);
			DrawDebugString(GetWorld(), debugLocation, FString::Printf(TEXT("%d"), m_entity.GetId()), nullptr, FColor::Yellow, 0.0f, true, 1.0f);
		}
#endif //!UE_BUILD_SHIPPING
	}

	if (m_argusActorInfoWidget.IsValid())
	{
		if (hasMovedOrRotated)
		{
			UpdateUIWidgetComponentLocation();
		}
		
		m_argusActorInfoWidget->RefreshDisplay(m_entity);
	}
}

void AArgusActor::OnChanged_m_baseState(EBaseState oldState, EBaseState newState)
{
	if (oldState != EBaseState::Dead && newState == EBaseState::Dead)
	{
		OnArgusEntityDeath();
	}
}

void AArgusActor::OnChanged_m_flightState(EFlightState oldValue, EFlightState newValue)
{
	if (oldValue != EFlightState::TakingOff && newValue == EFlightState::TakingOff)
	{
		OnTakeOff();
	}
	else if (oldValue != EFlightState::Grounded && newValue == EFlightState::Grounded)
	{
		OnLand();
	}
}

void AArgusActor::OnChanged_m_carrierEntityId(uint16 oldValue, uint16 newValue)
{
	OnArgusEntityPassengerStateChanged(newValue != ArgusECSConstants::k_maxEntities);
}

void AArgusActor::InitializeWidgets()
{
	UClass* widgetClass = m_argusActorInfoWidgetClass.LoadSynchronous();
	if (m_argusActorInfoWidget.IsValid() || !widgetClass)
	{
		return;
	}

	UWidgetComponent* widgetComponent = GetComponentByClass<UWidgetComponent>();
	if (!widgetComponent)
	{
		return;
	}

	widgetComponent->SetWidgetClass(widgetClass);
	m_argusActorInfoWidget = Cast<UArgusActorInfoView>(widgetComponent->GetWidget());

	if (m_argusActorInfoWidget.IsValid())
	{
		UpdateUIWidgetComponentLocation();
		m_argusActorInfoWidget->SetInitialDisplay(m_entity);
	}
}

void AArgusActor::UpdateUIWidgetComponentLocation()
{
	ARGUS_TRACE(AArgusActor::UpdateUIWidgetComponentLocation)

	UWidgetComponent* widgetComponent = GetComponentByClass<UWidgetComponent>();
	if (!widgetComponent)
	{
		return;
	}

	float zValue = widgetComponent->GetRelativeLocation().Z;
	FVector offsetLocation = AArgusCameraActor::GetPanUpVector() * m_uiWidgetOffsetDistance;
	offsetLocation.Z = zValue;
	widgetComponent->SetWorldLocation(offsetLocation + GetActorLocation());
}

void AArgusActor::FixupTransformForFlying()
{
	const TaskComponent* taskComponent = m_entity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return;
	}

	if (taskComponent->m_flightState != EFlightState::Flying && taskComponent->m_flightState != EFlightState::Landing)
	{
		return;
	}

	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	TransformComponent* transformComponent = m_entity.GetComponent<TransformComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(transformComponent, ArgusECSLog);

	float currentFacing = 0.0f;
	if (FacingComponent* facingComponent = m_entity.GetComponent<FacingComponent>())
	{
		currentFacing = facingComponent->GetCurrentFacing();
	}
	transformComponent->m_location.Z = spatialPartitioningComponent->m_flyingPlaneHeight;

	SetActorLocationAndRotation(transformComponent->m_location, FRotator(0.0f, ArgusMath::GetUEYawDegreesFromYaw(currentFacing), 0.0f));
}

void AArgusActor::CallEventsForInitialState()
{
	const TaskComponent* taskComponent = m_entity.GetComponent<TaskComponent>();
	ARGUS_RETURN_ON_NULL(taskComponent, ArgusUnrealObjectsLog);

	if (taskComponent->m_baseState == EBaseState::Dead)
	{
		OnArgusEntityDeath();
	}

	bool isPassenger = false;
	if (const PassengerComponent* passengerComponent = m_entity.GetComponent<PassengerComponent>())
	{
		if (passengerComponent->m_carrierEntityId != ArgusECSConstants::k_maxEntities)
		{
			isPassenger = true;
		}
	}
	OnArgusEntityPassengerStateChanged(isPassenger);

	if (taskComponent->m_flightState == EFlightState::Grounded)
	{
		OnLand();
	}
	else
	{
		OnTakeOff();
	}

	m_isSelected = m_entity.IsSelected();
	m_isSelected ? OnSelected() : OnDeselected();
}

bool AArgusActor::IsLocationNearlyActorLocation(const FVector& location) const
{
	const FVector actorLocation = GetActorLocation();
	return FMath::IsNearlyEqual(location.X, actorLocation.X) && FMath::IsNearlyEqual(location.Y, actorLocation.Y) && FMath::IsNearlyEqual(location.Z, actorLocation.Z);
}

bool AArgusActor::IsRotationNearlyActorRotation(const FRotator& rotation) const
{
	const FRotator actorRotation = GetActorRotation();
	return FMath::IsNearlyEqual(rotation.Pitch, actorRotation.Pitch) && FMath::IsNearlyEqual(rotation.Yaw, actorRotation.Yaw) && FMath::IsNearlyEqual(rotation.Roll, actorRotation.Roll);
}
