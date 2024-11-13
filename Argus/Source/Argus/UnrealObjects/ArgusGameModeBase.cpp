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

	UE_LOG(ArgusUnrealObjectsLog, Display, TEXT("[%s] Constructing Argus game mode base."), ARGUS_FUNCNAME);
	GameStateClass = AArgusGameStateBase::StaticClass();
}

void AArgusGameModeBase::StartPlay()
{
	ArgusEntity::FlushAllEntities();
	Super::StartPlay();
	UE_LOG(ArgusUnrealObjectsLog, Display, TEXT("[%s] Argus game mode base starting play."), ARGUS_FUNCNAME);
}

void AArgusGameModeBase::Tick(float deltaTime)
{
	ARGUS_TRACE(AArgusGameModeBase::Tick)

	Super::Tick(deltaTime);

	UWorld* worldPointer = GetWorld();

	// Update camera and process player input
	for (AArgusPlayerController* argusPlayerController : TActorRange<AArgusPlayerController>(worldPointer))
	{
		if (argusPlayerController)
		{
			argusPlayerController->ProcessArgusPlayerInput(deltaTime);
		}
	}

	// Run all ECS systems.
	m_argusSystemsManager.RunSystems(worldPointer, deltaTime);

	// Take/Release ArgusActors based on ECS state.
	ManageActorStateForEntities();
}

void AArgusGameModeBase::ManageActorStateForEntities()
{
	ARGUS_TRACE(AArgusGameModeBase::ManageActorStateForEntities)

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

		if (taskComponent->m_baseState == EBaseState::SpawnedWaitingForActorTake)
		{
			SpawnActorFromEntity(entity, taskComponent);
		}
		else if (taskComponent->m_baseState == EBaseState::DestroyedWaitingForActorRelease)
		{
			// TODO JAMES: Release actor object back to the pool.
		}
	}
}

void AArgusGameModeBase::SpawnActorFromEntity(ArgusEntity entity, TaskComponent* taskComponent)
{
	if (!entity || !taskComponent)
	{
		return;
	}

	const UArgusActorRecord* actorRecord = ArgusStaticData::GetRecord<UArgusActorRecord>(taskComponent->m_spawnedFromArgusActorRecordId);
	if (!actorRecord)
	{
		return;
	}

	AArgusActor* spawnedActor = m_argusActorPool.Take(GetWorld(), actorRecord->m_argusActorClass);
	if (!spawnedActor)
	{
		return;
	}

	spawnedActor->SetEntity(entity);
}

FColor AArgusGameModeBase::GetTeamColor(ETeam team)
{
	if (!m_teamColorMap.Contains(team))
	{
		UE_LOG
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
		UE_LOG
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