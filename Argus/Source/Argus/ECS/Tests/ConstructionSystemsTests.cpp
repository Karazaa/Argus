// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusTesting.h"
#include "Systems/ConstructionSystems.h"
#include "Systems/TimerSystems.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ConstructionSystemsBeingConstructedAutomaticTest, "Argus.ECS.Systems.ConstructionSystems.BeingConstructed.Automatic", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ConstructionSystemsBeingConstructedAutomaticTest::RunTest(const FString& Parameters)
{
	const float timestep = 1.0f;
	const float constructionTimeSeconds = 2.0f;
	const float expectedElapsedTimerProportion = 0.5f;

	ArgusTesting::StartArgusTest();

	ArgusEntity entityToConstruct = ArgusEntity::CreateEntity();
	if (!entityToConstruct)
	{
		return false;
	}

	TaskComponent* constructedTaskComponent = entityToConstruct.AddComponent<TaskComponent>();
	ConstructionComponent* constructedConstructionComponent = entityToConstruct.AddComponent<ConstructionComponent>();
	entityToConstruct.AddComponent<TimerComponent>();

	if (!constructedTaskComponent || !constructedConstructionComponent)
	{
		return false;
	}

	constructedTaskComponent->m_constructionState = EConstructionState::BeingConstructed;
	constructedConstructionComponent->m_constructionType = EConstructionType::Automatic;
	constructedConstructionComponent->m_requiredWorkSeconds = constructionTimeSeconds;
	constructedConstructionComponent->m_automaticConstructionTimerHandle.StartTimer(entityToConstruct, constructionTimeSeconds);

	TimerSystems::RunSystems(timestep);
	ConstructionSystems::RunSystems(timestep);
	
#pragma region Verifying that current work seconds equals one after two timesteps of automatic construction.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Verifying that %s equals %f after two timesteps of automatic construction."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_currentWorkSeconds),
			timestep
		),
		constructedConstructionComponent->m_currentWorkSeconds,
		timestep
	);
#pragma endregion

	TimerSystems::RunSystems(timestep);
	ConstructionSystems::RunSystems(timestep);

#pragma region Verifying that construction state equals construction finished after two timesteps of automatic construction.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Verifying that %s equals %s after two timesteps of automatic construction."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_constructionState),
			ARGUS_NAMEOF(EConstructionState::ConstructionFinished)
		),
		constructedTaskComponent->m_constructionState,
		EConstructionState::ConstructionFinished
	);
#pragma endregion

#pragma region Verifying that current work seconds equals required work seconds after two timesteps of automatic construction.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Verifying that %s equals %s after three timesteps of automatic construction."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_currentWorkSeconds),
			ARGUS_NAMEOF(m_requiredWorkSeconds)
		),
		constructedConstructionComponent->m_currentWorkSeconds,
		constructedConstructionComponent->m_requiredWorkSeconds
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ConstructionSystemsBeingConstructedManualTest, "Argus.ECS.Systems.ConstructionSystems.BeingConstructed.Manual", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ConstructionSystemsBeingConstructedManualTest::RunTest(const FString& Parameters)
{
	const float timestep = 1.0f;
	const float constructionTimeSeconds = 2.0f;
	const float expectedElapsedTimerProportion = 0.5f;
	const float constructionRange = 100.0f;
	const int32 abilityRecordId = 100;
	const FVector constructedLocation = FVector(0.0f, 100.0f, 0.0f);
	const FVector initialConstructingLocation = FVector(500.0f, 500.0f, 0.0f);
	const FVector constructingLocation = FVector(50.0f, 50.0f, 0.0f);

	ArgusTesting::StartArgusTest();

	ArgusEntity entityToConstruct = ArgusEntity::CreateEntity();
	ArgusEntity constructingEntity = ArgusEntity::CreateEntity();
	if (!entityToConstruct || !constructingEntity)
	{
		return false;
	}

	TaskComponent* constructedTaskComponent = entityToConstruct.AddComponent<TaskComponent>();
	ConstructionComponent* constructedConstructionComponent = entityToConstruct.AddComponent<ConstructionComponent>();
	TransformComponent* constructedTransformComponent = entityToConstruct.AddComponent<TransformComponent>();

	AbilityComponent* constructingAbilityComponent = constructingEntity.AddComponent<AbilityComponent>();
	TaskComponent* constructingTaskComponent = constructingEntity.AddComponent<TaskComponent>();
	TargetingComponent* constructingTargetingComponent = constructingEntity.AddComponent<TargetingComponent>();
	TransformComponent* constructingTransformComponent = constructingEntity.AddComponent<TransformComponent>();

	if (!constructedTaskComponent || !constructedConstructionComponent || !constructedTransformComponent || !constructingAbilityComponent || 
		!constructingTaskComponent || !constructingTargetingComponent || !constructingTransformComponent)
	{
		return false;
	}

	constructedTaskComponent->m_constructionState = EConstructionState::BeingConstructed;
	constructedConstructionComponent->m_constructionType = EConstructionType::Manual;
	constructedConstructionComponent->m_requiredWorkSeconds = constructionTimeSeconds;
	constructedConstructionComponent->m_constructionAbilityRecordId = abilityRecordId;
	constructedTransformComponent->m_location = constructedLocation;

	constructingAbilityComponent->m_ability0Id = abilityRecordId;
	constructingTaskComponent->m_constructionState = EConstructionState::DispatchedToConstructOther;
	constructingTargetingComponent->m_targetEntityId = entityToConstruct.GetId();
	constructingTargetingComponent->m_meleeRange = constructionRange;
	constructingTransformComponent->m_location = initialConstructingLocation;

	ConstructionSystems::RunSystems(timestep);

#pragma region Verifying that current work seconds equals zero after one timestep of out of range manual construction.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Verifying that %s equals %f after one timestep of out of range manual construction."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_currentWorkSeconds),
			0.0f
		),
		constructedConstructionComponent->m_currentWorkSeconds,
		0.0f
	);
#pragma endregion

	constructingTransformComponent->m_location = constructingLocation;
	ConstructionSystems::RunSystems(timestep);

#pragma region Verifying that current work seconds equals one after one timestep of in range manual construction.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Verifying that %s equals %f after one timestep of in range manual construction."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_currentWorkSeconds),
			timestep
		),
		constructedConstructionComponent->m_currentWorkSeconds,
		timestep
	);
#pragma endregion

	ConstructionSystems::RunSystems(timestep);

#pragma region Verifying that construction state equals none after two timesteps of in range manual construction.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Verifying that %s equals %s after two timesteps of in range manual construction."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_constructionState),
			ARGUS_NAMEOF(EConstructionState::None)
		),
		constructingTaskComponent->m_constructionState,
		EConstructionState::None
	);
#pragma endregion

#pragma region Verifying that current work seconds equals required work seconds after two timesteps of in range manual construction.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Verifying that %s equals %s after two timesteps of in range manual construction."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_currentWorkSeconds),
			ARGUS_NAMEOF(m_requiredWorkSeconds)
		),
		constructedConstructionComponent->m_currentWorkSeconds,
		constructedConstructionComponent->m_requiredWorkSeconds
	);
#pragma endregion

	ConstructionSystems::RunSystems(timestep);

#pragma region Verifying that construction state equals construction finished after three timesteps of in range manual construction.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Verifying that %s equals %s after three timesteps of in range manual construction."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_constructionState),
			ARGUS_NAMEOF(EConstructionState::ConstructionFinished)
		),
		constructedTaskComponent->m_constructionState,
		EConstructionState::ConstructionFinished
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

#endif // WITH_AUTOMATION_TESTS