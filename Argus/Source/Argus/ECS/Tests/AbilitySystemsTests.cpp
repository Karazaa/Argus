// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusTesting.h"
#include "Misc/AutomationTest.h"
#include "RecordDefinitions/AbilityRecord.h"
#include "RecordDefinitions/ArgusActorRecord.h"
#include "Systems/AbilitySystems.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(AbilitySystemsCastSpawnAbilityTest, "Argus.ECS.Systems.AbilitySystems.CastSpawnAbility", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool AbilitySystemsCastSpawnAbilityTest::RunTest(const FString& Parameters)
{
	const uint32 argusActorRecordId = 5u;
	ArgusTesting::StartArgusTest();

#pragma region Test that passing in an invalid AbilitySystemsComponentArgs errors
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Passed in %s object has invalid component references."),
			ARGUS_NAMEOF(AbilitySystemsComponentArgs)
		)
	);
#pragma endregion

	AbilitySystems::AbilitySystemsComponentArgs components;
	AbilitySystems::CastAbility(nullptr, components);

	components.m_entity = ArgusEntity::CreateEntity();
	components.m_abilityComponent = components.m_entity.AddComponent<AbilityComponent>();
	components.m_taskComponent = components.m_entity.AddComponent<TaskComponent>();

#pragma region Test that passing in an invalid UAbilityRecord* errors
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Could not successfully retrieve %s from %s."),
			ARGUS_NAMEOF(UAbilityRecord*),
			ARGUS_NAMEOF(ArgusStaticData)
		)
	);
#pragma endregion

	AbilitySystems::CastAbility(nullptr, components);

#pragma region Test that casting a spawn ability without a spawning component errors
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Attempting to cast a spawn ability without having a %s."),
			ARGUS_NAMEOF(SpawningComponent)
		)
	);
#pragma endregion

	UAbilityRecord* abilityRecord = NewObject<UAbilityRecord>();
	if (!abilityRecord)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}
	abilityRecord->m_abilityType = EAbilityTypes::Spawn;
	AbilitySystems::CastAbility(abilityRecord, components);

	SpawningComponent* spawnComponent = components.m_entity.AddComponent<SpawningComponent>();
	TimerComponent* timerComponent = components.m_entity.AddComponent<TimerComponent>();

#pragma region Test that spawning from an ability without an actor record specified errors
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Could not retrieve %s from %s."),
			ARGUS_NAMEOF(UArgusActorRecord*), 
			ARGUS_NAMEOF(UAbilityRecord*)
		)
	);
#pragma endregion

	AbilitySystems::CastAbility(abilityRecord, components);

	UArgusActorRecord* argusActorRecord = NewObject<UArgusActorRecord>();
	if (!argusActorRecord)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}
	argusActorRecord->m_id = argusActorRecordId;
	abilityRecord->m_timeToSpawnSeconds = 0.0f;
	abilityRecord->m_argusActorRecordToSpawn = argusActorRecord;

	AbilitySystems::CastAbility(abilityRecord, components);

#pragma region Test that task component/spawning component have been told to spawn.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Casting a Spawn Ability and confirming that the spawning %s has the correct state."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity)
		),
		(spawnComponent->m_argusActorRecordId == argusActorRecordId) &&
		(components.m_taskComponent->m_spawningState == SpawningState::SpawningEntity)
	);
#pragma endregion

	abilityRecord->m_timeToSpawnSeconds = 1.0f;
	AbilitySystems::CastAbility(abilityRecord, components);

#pragma region Test that task component/spawning component have been told to spawn.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Casting a Spawn Ability with a cast time and confirming that the spawning %s has the correct state."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity)
		),
		(spawnComponent->m_argusActorRecordId == argusActorRecordId) &&
		(components.m_taskComponent->m_spawningState == SpawningState::WaitingToSpawnEntity) &&
		(spawnComponent->m_spawnTimerHandle.GetTimerIndex() != UINT8_MAX)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

#endif //WITH_AUTOMATION_TESTS