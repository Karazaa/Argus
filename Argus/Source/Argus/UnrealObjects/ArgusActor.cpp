// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActor.h"
#include "ArgusGameInstance.h"
#include "ArgusGameModeBase.h"
#include "ArgusStaticData.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

static TAutoConsoleVariable<bool> CVarShowAllArgusEntitiesDebug(TEXT("Argus.ArgusEntity.ShowAllArgusEntitiesDebug"), false, TEXT(""));
static TAutoConsoleVariable<bool> CVarShowSelectedArgusEntityDebug(TEXT("Argus.ArgusEntity.ShowSelectedArgusEntityDebug"), false, TEXT(""));

AArgusActor::AArgusActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_LastDemotable;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(FName("RootSceneComponent")));
}

ArgusEntity AArgusActor::GetEntity() const
{
	return m_entity;
}

void AArgusActor::SetEntity(const ArgusEntity& entity)
{
	if (!m_entity)
	{
		return;
	}

	if (TransformComponent* transformComponent = m_entity.GetComponent<TransformComponent>())
	{
		transformComponent->m_transform = GetActorTransform();
	}

	const UWorld* world = GetWorld();
	if (!world)
	{
		return;
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

	UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		return;
	}

	gameInstance->RegisterArgusEntityActor(this);
}

void AArgusActor::Reset()
{
	if (const UWorld* world = GetWorld())
	{
		if (UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>())
		{
			if (m_entity)
			{
				gameInstance->DeregisterArgusEntityActor(this);
			}
		}
	}

	m_entityTemplate = nullptr;
	m_isSelected = false;
	m_entity = ArgusEntity::s_emptyEntity;
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

void AArgusActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (!m_entityTemplate)
	{
		return;
	}

	m_entity = m_entityTemplate->MakeEntity();
	if (!m_entity)
	{
		return;
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

void AArgusActor::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (!m_entity)
	{
		return;
	}

	if (const TransformComponent* transformComponent = m_entity.GetComponent<TransformComponent>())
	{
		SetActorTransform(transformComponent->m_transform);

		if (CVarShowAllArgusEntitiesDebug.GetValueOnGameThread() || (CVarShowSelectedArgusEntityDebug.GetValueOnGameThread() && m_isSelected))
		{
			DrawDebugString(GetWorld(), transformComponent->m_transform.GetLocation(), m_entity.GetDebugString(), nullptr, FColor::Yellow, 0.0f, true, 0.75f);
		}
	}
}
