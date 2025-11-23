// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameModeBase.h"
#include "ArgusEntityTemplate.h"
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

AArgusGameModeBase::~AArgusGameModeBase()
{
	m_argusSystemsThread.Stop();
}

void AArgusGameModeBase::StartPlay()
{
	ArgusEntity::FlushAllEntities();
	UWorld* worldPointer = GetWorld();
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusUnrealObjectsLog);
	ArgusSystemsManager::Initialize(worldPointer, m_initialTeamResourceSet, m_singletonTemplate.LoadSynchronous());

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

	ArgusSystemsManager::OnStartPlay(worldPointer, m_activePlayerController->GetPlayerTeam());
	m_argusSystemsThread.Init();
	m_argusSystemsThread.StartThread();
}

void AArgusGameModeBase::Tick(float deltaTime)
{
	ARGUS_TRACE(AArgusGameModeBase::Tick);

	Super::Tick(deltaTime);

	UWorld* worldPointer = GetWorld();
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusUnrealObjectsLog);
	ARGUS_RETURN_ON_NULL(m_activePlayerController, ArgusUnrealObjectsLog);
	
	// Tick the systems thread. Logic here should be relatively resilient to mid execution ECS changes
	m_argusSystemsThread.TickThread(deltaTime);

	// Update camera and process player input
	m_activePlayerController->ProcessArgusPlayerInput(deltaTime);

	// Run all ECS systems.
	ArgusSystemsManager::RunSystems(worldPointer, deltaTime);

	// Take/Release/Update ArgusActors based on ECS state.
	ManageActorStateForEntities(worldPointer, deltaTime);

	// Now wait on m_argusSystemsThread to finish its tick if necessary to execute worker thread dependent systems.
	FPlatformProcess::ConditionalSleep([this]() -> bool
	{
		return !m_argusSystemsThread.IsTicking() || m_argusSystemsThread.IsShutdown();
	});

	ArgusSystemsManager::RunPostThreadSystems();
}

void AArgusGameModeBase::ManageActorStateForEntities(const UWorld* worldPointer, float deltaTime)
{
	ARGUS_TRACE(AArgusGameModeBase::ManageActorStateForEntities);

	ARGUS_RETURN_ON_NULL(worldPointer, ArgusUnrealObjectsLog);
	const UArgusGameInstance* gameInstance = worldPointer->GetGameInstance<UArgusGameInstance>();
	ARGUS_RETURN_ON_NULL(gameInstance, ArgusUnrealObjectsLog);
	ARGUS_RETURN_ON_NULL(m_activePlayerController, ArgusUnrealObjectsLog);

	ArgusEntity::IterateEntities([this, gameInstance, deltaTime](ArgusEntity entity) 
	{
		TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
		if (!taskComponent)
		{
			return;
		}

		if (taskComponent->m_baseState == EBaseState::SpawnedWaitingForActorTake)
		{
			SpawnActorForEntity(entity);
		}
		else if (taskComponent->m_baseState == EBaseState::DestroyedWaitingForActorRelease)
		{
			DespawnActorForEntity(entity);
		}

		if (AArgusActor* argusActor = gameInstance->GetArgusActorFromArgusEntity(entity))
		{
			argusActor->Update(deltaTime, m_activePlayerController->GetPlayerTeam());
		}
	});
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
	taskComponent->m_baseState = EBaseState::Alive;
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
	taskComponent->m_baseState = EBaseState::Alive;

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
			ARGUS_NAMEOF(team),
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