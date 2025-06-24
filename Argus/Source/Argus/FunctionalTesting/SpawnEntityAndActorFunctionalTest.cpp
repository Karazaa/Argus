// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpawnEntityAndActorFunctionalTest.h"
#include "ArgusActor.h"
#include "ArgusEntityTemplate.h"
#include "ArgusStaticData.h"
#include "EngineUtils.h"

ASpawnEntityAndActorFunctionalTest::ASpawnEntityAndActorFunctionalTest(const FObjectInitializer& ObjectInitializer)
{
	TestLabel = k_testName;
}

bool ASpawnEntityAndActorFunctionalTest::DidArgusFunctionalTestFail()
{
	if (m_argusActorRecord.IsNull())
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_argusActorRecord)
		);
		return true;
	}

	return false;
}

bool ASpawnEntityAndActorFunctionalTest::DidCurrentTestStepSucceed()
{
	if (DidArgusFunctionalTestFail())
	{
		return false;
	}

	switch (m_testStepIndex)
	{
		case 0u:
			return DidLoadArgusActorRecordTestStep();
		case 1u:
			return DidSpawnArgusEntityTestStep();
		case 2u:
			return DidSpawnArgusActorTestStep();
		case 3u:
			return DidDespawnArgusActorTestStep();
		default:
			return false;
	}
}

void ASpawnEntityAndActorFunctionalTest::StartNextTestStep()
{
	if (DidArgusFunctionalTestFail())
	{
		return;
	}

	switch (m_testStepIndex)
	{
		case 0u:
			StartLoadArgusActorRecordTestStep();
			break;
		case 1u:
			StartSpawnArgusEntityTestStep();
			break;
		case 2u:
			StartSpawnArgusActorTestStep();
			break;
		case 3u:
			StartDespawnArgusActorTestStep();
			break;
		default:
			break;
	}
}

void ASpawnEntityAndActorFunctionalTest::StartLoadArgusActorRecordTestStep()
{
	StartStep(TEXT("Loading argus actor record."));

	m_loadedArgusActorRecord = m_argusActorRecord.LoadSynchronous();
}

void ASpawnEntityAndActorFunctionalTest::StartSpawnArgusEntityTestStep()
{
	StartStep(TEXT("Spawn Argus Entity."));

	m_spawnerEntity = ArgusEntity::CreateEntity();
	if (!m_spawnerEntity)
	{
		return;
	}

	SpawningComponent* spawnerSpawningComponent = m_spawnerEntity.AddComponent<SpawningComponent>();
	if (!spawnerSpawningComponent)
	{
		return;
	}

	TargetingComponent* spawnerTargetingComponent = m_spawnerEntity.AddComponent<TargetingComponent>();
	if (!spawnerTargetingComponent)
	{
		return;
	}

	TaskComponent* spawnerTaskComponent = m_spawnerEntity.AddComponent<TaskComponent>();
	if (!spawnerTaskComponent)
	{
		return;
	}

	TransformComponent* spawnerTransformComponent = m_spawnerEntity.AddComponent<TransformComponent>();
	if (!spawnerTransformComponent)
	{
		return;
	}

	const UArgusEntityTemplate* loadedEntityTemplate = m_loadedArgusActorRecord->m_entityTemplate.LoadAndStorePtr();
	if (!loadedEntityTemplate)
	{
		return;
	}

	m_expectedSpawnedEntityId = static_cast<uint16>(loadedEntityTemplate->m_entityPriority);
	SpawnEntityInfo spawnInfo;
	if (UArgusActorRecord* loadedRecord = m_argusActorRecord.LoadSynchronous())
	{
		spawnInfo.m_argusActorRecordId = loadedRecord->m_id;
	}
	spawnerSpawningComponent->m_spawnQueue.Enqueue(spawnInfo);
	spawnerSpawningComponent->m_currentQueueSize++;
	spawnerTaskComponent->m_spawningState = ESpawningState::SpawningEntity;
}

void ASpawnEntityAndActorFunctionalTest::StartSpawnArgusActorTestStep()
{
	StartStep(TEXT("Spawn Argus Actor."));
	SetMinimumSecondsPerTestStep(m_secondsToWaitAfterSpawningActor);
}

void ASpawnEntityAndActorFunctionalTest::StartDespawnArgusActorTestStep()
{
	StartStep(TEXT("Despawn Argus Actor."));
	SetMinimumSecondsPerTestStep(m_secondsToWaitAfterSpawningActor);

	ArgusEntity spawnedEntity = ArgusEntity::RetrieveEntity(m_expectedSpawnedEntityId);
	if (!spawnedEntity)
	{
		return;
	}

	TaskComponent* taskComponent = spawnedEntity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return;
	}

	taskComponent->m_baseState = EBaseState::DestroyedWaitingForActorRelease;
}


bool ASpawnEntityAndActorFunctionalTest::DidLoadArgusActorRecordTestStep()
{
	if (m_loadedArgusActorRecord)
	{
		return true;
	}

	return false;
}

bool ASpawnEntityAndActorFunctionalTest::DidSpawnArgusEntityTestStep()
{
	return ArgusEntity::DoesEntityExist(m_expectedSpawnedEntityId);
}

bool ASpawnEntityAndActorFunctionalTest::DidSpawnArgusActorTestStep()
{
	const UWorld* worldPointer = GetWorld();
	if (!worldPointer)
	{
		return false;
	}

	const UArgusGameInstance* gameInstance = worldPointer->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		return false;
	}

	const AArgusActor* argusActor = gameInstance->GetArgusActorFromArgusEntity(ArgusEntity::RetrieveEntity(m_expectedSpawnedEntityId));
	if (!argusActor)
	{
		return false;
	}

	return true;
}

bool ASpawnEntityAndActorFunctionalTest::DidDespawnArgusActorTestStep()
{
	if (ArgusEntity::DoesEntityExist(m_expectedSpawnedEntityId))
	{
		return false;
	}

	const UWorld* worldPointer = GetWorld();
	if (!worldPointer)
	{
		return false;
	}

	for (const AArgusActor* foundArgusActor : TActorRange<AArgusActor>(worldPointer))
	{
		if (foundArgusActor && foundArgusActor->IsVisible())
		{
			return false;
		}
	}

	return true;
}