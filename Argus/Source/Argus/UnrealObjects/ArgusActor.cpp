// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActor.h"
#include "ArgusActorInfoWidget.h"
#include "ArgusCameraActor.h"
#include "ArgusGameInstance.h"
#include "ArgusGameModeBase.h"
#include "ArgusMath.h"
#include "ArgusStaticData.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#endif //!UE_BUILD_SHIPPING

AArgusActor::AArgusActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(FName("RootSceneComponent")));
}

void AArgusActor::Reset()
{
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
	m_isSelected = false;
	m_entity = ArgusEntity::k_emptyEntity;

	Hide();
}

ArgusEntity AArgusActor::GetEntity() const
{
	return m_entity;
}

void AArgusActor::SetEntity(const ArgusEntity& entity)
{
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
			const float desiredYaw = ArgusMath::GetYawFromDirection(GetActorForwardVector());
			transformComponent->m_targetYaw = desiredYaw;
			transformComponent->m_smoothedYaw.Reset(desiredYaw);
		}
		else
		{
			SetActorLocation(transformComponent->m_location);
			SetActorRotation(FRotationMatrix::MakeFromXZ(ArgusMath::GetDirectionFromYaw(transformComponent->GetCurrentYaw()), FVector::UpVector).ToQuat());
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
	RootComponent->SetVisibility(true, true);
}

void AArgusActor::Hide()
{
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

	const UArgusEntityTemplate* loadedEntityTemplate = loadedArgusActorRecord->m_entityTemplateOverride.LoadSynchronous();
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
			identityComponent->m_allies = loadedTeamInfoRecord->m_allies;
			identityComponent->m_enemies = loadedTeamInfoRecord->m_enemies;
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
	if (!m_entity)
	{
		return;
	}

	if (const TransformComponent* transformComponent = m_entity.GetComponent<TransformComponent>())
	{
		SetActorLocation(transformComponent->m_location);
		SetActorRotation(FRotationMatrix::MakeFromXZ(ArgusMath::GetDirectionFromYaw(transformComponent->GetCurrentYaw()), FVector::UpVector).ToQuat());

#if !UE_BUILD_SHIPPING
		if (ArgusECSDebugger::IsEntityBeingDebugged(m_entity.GetId()))
		{
			DrawDebugString(GetWorld(), transformComponent->m_location, m_entity.GetDebugString(), nullptr, FColor::Yellow, 0.0f, true, 0.75f);
		}
#endif //!UE_BUILD_SHIPPING
	}

	if (m_argusActorInfoWidget.IsValid())
	{
		UpdateUIWidgetComponentLocation();
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
	m_argusActorInfoWidget = Cast<UArgusActorInfoWidget>(widgetComponent->GetWidget());

	if (m_argusActorInfoWidget.IsValid())
	{
		UpdateUIWidgetComponentLocation();
		m_argusActorInfoWidget->SetInitialDisplay(m_entity);
	}
}

void AArgusActor::UpdateUIWidgetComponentLocation()
{
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
