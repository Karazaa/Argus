// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameModeBase.h"
#include "ArgusEntity.h"
#include "ArgusGameStateBase.h"
#include "ArgusPlayerController.h"
#include "ArgusStaticData.h"
#include "EngineUtils.h"

AArgusGameModeBase::AArgusGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PrePhysics;

	ARGUS_LOG(ArgusUnrealObjectsLog, Display, TEXT("[%s] Constructing Argus game mode base."), ARGUS_FUNCNAME);
	GameStateClass = AArgusGameStateBase::StaticClass();
}

void AArgusGameModeBase::StartPlay()
{
	ArgusEntity::FlushAllEntities();
	Super::StartPlay();
	ARGUS_LOG(ArgusUnrealObjectsLog, Display, TEXT("[%s] Argus game mode base starting play."), ARGUS_FUNCNAME);

	for (AArgusPlayerController* argusPlayerController : TActorRange<AArgusPlayerController>(GetWorld()))
	{
		m_activePlayerController = argusPlayerController;
		break;
	}

	if (m_activePlayerController)
	{
		m_activePlayerController->InitializeUIWidgets();
	}

	UWorld* worldPointer = GetWorld();
	if (!worldPointer)
	{
		ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld*));
		return;
	}

	ArgusSystemsManager::Initialize(worldPointer, m_initialTeamResourceSet);
}

void AArgusGameModeBase::Tick(float deltaTime)
{
	ARGUS_TRACE(AArgusGameModeBase::Tick);

	Super::Tick(deltaTime);

	UWorld* worldPointer = GetWorld();
	if (!worldPointer)
	{
		ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld*));
		return;
	}

	if (!m_activePlayerController)
	{
		ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(AArgusPlayerController*));
		return;
	}
	
	// Update camera and process player input
	m_activePlayerController->ProcessArgusPlayerInput(deltaTime);

	// Run all ECS systems.
	ArgusSystemsManager::RunSystems(worldPointer, deltaTime);

	// Take/Release ArgusActors based on ECS state.
	ManageActorStateForEntities();
}

void AArgusGameModeBase::ManageActorStateForEntities()
{
	ARGUS_TRACE(AArgusGameModeBase::ManageActorStateForEntities);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
		if (!entity)
		{
			continue;
		}

		TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
		if (!taskComponent)
		{
			continue;
		}

		if (taskComponent->m_baseState == BaseState::SpawnedWaitingForActorTake)
		{
			SpawnActorForEntity(entity);
		}
		else if (taskComponent->m_baseState == BaseState::DestroyedWaitingForActorRelease)
		{
			DespawnActorForEntity(entity);
		}
	}
}

void AArgusGameModeBase::SpawnActorForEntity(ArgusEntity spawnedEntity)
{
	UWorld* worldPointer = GetWorld();
	if (!worldPointer || !spawnedEntity)
	{
		return;
	}

	TaskComponent* taskComponent = spawnedEntity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return;
	}

	const UArgusActorRecord* actorRecord = ArgusStaticData::GetRecord<UArgusActorRecord>(taskComponent->m_spawnedFromArgusActorRecordId);
	if (!actorRecord)
	{
		return;
	}

	AArgusActor* spawnedActor = m_argusActorPool.Take(worldPointer, actorRecord->m_argusActorClass);
	if (!spawnedActor)
	{
		return;
	}

	spawnedActor->SetEntity(spawnedEntity);
	taskComponent->m_baseState = BaseState::None;
}

void AArgusGameModeBase::DespawnActorForEntity(ArgusEntity spawnedEntity)
{
	UWorld* worldPointer = GetWorld();
	if (!worldPointer || !spawnedEntity)
	{
		return;
	}
	
	const UArgusGameInstance* gameInstance = worldPointer->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		return;
	}

	AArgusActor* argusActor = gameInstance->GetArgusActorFromArgusEntity(spawnedEntity);
	if (!argusActor)
	{
		return;
	}

	m_argusActorPool.Release(argusActor);

	TaskComponent* taskComponent = spawnedEntity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return;
	}
	taskComponent->m_baseState = BaseState::None;

	ArgusEntity::DestroyEntity(spawnedEntity);
}

FColor AArgusGameModeBase::GetTeamColor(ETeam team)
{
	if (!m_teamColorMap.Contains(team))
	{
		ARGUS_LOG
		(
			ArgusUnrealObjectsLog, 
			Error, 
			TEXT("[%s] Trying to retrieve color for %s, but the %s does not have an entry in %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ETeam), 
			ARGUS_NAMEOF(m_teamColorMap)
		);
		return FColor::Red;
	}

	TObjectPtr<UTeamColorRecord>* pointerToTeamColorRecordObjectPointer = &m_teamColorMap[team];
	if (!(*pointerToTeamColorRecordObjectPointer))
	{
		ARGUS_LOG
		(
			ArgusUnrealObjectsLog, 
			Error, 
			TEXT("[%s] Trying to retrieve color for %s, but the entry in %s does not have a %s defined."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ETeam), 
			ARGUS_NAMEOF(m_teamColorMap), 
			ARGUS_NAMEOF(UTeamColorRecord)
		);
		return FColor::Red;
	}
	
	return (*pointerToTeamColorRecordObjectPointer)->m_teamColor;
}