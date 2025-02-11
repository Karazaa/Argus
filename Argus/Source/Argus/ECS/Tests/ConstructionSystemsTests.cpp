// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
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
	TaskComponent* constructedTaskComponent = entityToConstruct.AddComponent<TaskComponent>();
	ConstructionComponent* constructedConstructionComponent = entityToConstruct.AddComponent<ConstructionComponent>();
	entityToConstruct.AddComponent<TimerComponent>();

	if (!constructedTaskComponent || !constructedConstructionComponent)
	{
		return false;
	}

	constructedTaskComponent->m_constructionState = ConstructionState::BeingConstructed;
	constructedConstructionComponent->m_constructionType = EConstructionType::Automatic;
	constructedConstructionComponent->m_requiredWorkSeconds = constructionTimeSeconds;
	constructedConstructionComponent->m_automaticConstructionTimerHandle.StartTimer(entityToConstruct, constructionTimeSeconds);

	ConstructionSystems::RunSystems(timestep);
	TimerSystems::RunSystems(timestep);

#pragma region Automatically construct and verifying correct state after one second.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating an %s to automatically construct and verifying that the %s is in the correct state after %f seconds."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ConstructionComponent),
			timestep
		),
		constructedConstructionComponent->m_automaticConstructionTimerHandle.GetTimeElapsedProportion(entityToConstruct),
		expectedElapsedTimerProportion
	);
#pragma endregion

	ConstructionSystems::RunSystems(timestep);
	TimerSystems::RunSystems(timestep);

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

#pragma region Checking that the automatic construction timer is finished after two timesteps.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Checking that the automatic construction timer of %s is finished after two timesteps."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ConstructionComponent)
		),
		constructedConstructionComponent->m_automaticConstructionTimerHandle.IsTimerComplete(entityToConstruct)
	);
#pragma endregion

	ConstructionSystems::RunSystems(timestep);

#pragma region Verifying that construction state equals construction finished after three timesteps of automatic construction.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Verifying that %s equals %s after three timesteps of automatic construction."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_constructionState),
			ARGUS_NAMEOF(ConstructionState::ConstructionFinished)
		),
		constructedTaskComponent->m_constructionState,
		ConstructionState::ConstructionFinished
	);
#pragma endregion

#pragma region Verifying that current work seconds equals required work seconds after three timesteps of automatic construction.
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
	ArgusTesting::StartArgusTest();

	ArgusTesting::EndArgusTest();
	return true;
}

#endif // WITH_AUTOMATION_TESTS