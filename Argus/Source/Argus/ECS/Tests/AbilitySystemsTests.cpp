// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusTesting.h"
#include "Misc/AutomationTest.h"
#include "RecordDefinitions/AbilityRecord.h"
#include "RecordDefinitions/ArgusActorRecord.h"
#include "Systems/AbilitySystems.h"
#include "Systems/SpawningSystems.h"

#if WITH_AUTOMATION_TESTS
// TODO JAMES: This test got wrecked by static data dependencies. Need a way to fix.
//IMPLEMENT_SIMPLE_AUTOMATION_TEST(AbilitySystemsCastSpawnAbilityTest, "Argus.ECS.Systems.AbilitySystems.CastSpawnAbility", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
//bool AbilitySystemsCastSpawnAbilityTest::RunTest(const FString& Parameters)
//{
//	const float smallTimeStep = 0.1f;
//	const float timeToCastSeconds = 1.0f;
//	const int32 maximumSpawnQueueSize = 2;
//	const uint32 argusActorRecordId = 5u;
//	ArgusTesting::StartArgusTest();
//
//#pragma region Test that passing in an invalid AbilitySystemsArgs errors
//	AddExpectedErrorPlain
//	(
//		FString::Printf
//		(
//			TEXT("Passed in %s object has invalid component references."),
//			ARGUS_NAMEOF(AbilitySystemsArgs)
//		)
//	);
//#pragma endregion
//
//	AbilitySystemsArgs components;
//	AbilitySystems::CastAbility(nullptr, components);
//
//	ReticleComponent reticleComponent = ReticleComponent();
//
//	components.m_entity = ArgusEntity::CreateEntity();
//	components.m_abilityComponent = components.m_entity.AddComponent<AbilityComponent>();
//	components.m_taskComponent = components.m_entity.AddComponent<TaskComponent>();
//	components.m_reticleComponent = &reticleComponent;
//	components.m_entity.AddComponent<TargetingComponent>();
//	components.m_entity.AddComponent<TransformComponent>();
//	IdentityComponent* identityComponent = components.m_entity.AddComponent<IdentityComponent>();
//	if (!identityComponent)
//	{
//		return false;
//	}
//
//	identityComponent->m_team = ETeam::TeamA;
//	ArgusEntity teamEntity = ArgusEntity::CreateEntity(ArgusEntity::GetTeamEntityId(ETeam::TeamA));
//	if (!teamEntity)
//	{
//		return false;
//	}
//	teamEntity.AddComponent<ResourceComponent>();
//
//#pragma region Test that passing in an invalid UAbilityRecord* errors
//	AddExpectedErrorPlain
//	(
//		FString::Printf
//		(
//			TEXT("Could not successfully retrieve %s from %s."),
//			ARGUS_NAMEOF(UAbilityRecord*),
//			ARGUS_NAMEOF(ArgusStaticData)
//		)
//	);
//#pragma endregion
//
//	AbilitySystems::CastAbility(nullptr, components);
//
//#pragma region Test that casting a spawn ability without a spawning component errors
//	AddExpectedErrorPlain
//	(
//		FString::Printf
//		(
//			TEXT("Attempting to cast a spawn ability without having a %s."),
//			ARGUS_NAMEOF(SpawningComponent)
//		)
//	);
//#pragma endregion
//
//	UAbilityRecord* abilityRecord = NewObject<UAbilityRecord>();
//	if (!abilityRecord)
//	{
//		ArgusTesting::EndArgusTest();
//		return false;
//	}
//	abilityRecord->m_abilityType = EAbilityTypes::Spawn;
//	AbilitySystems::CastAbility(abilityRecord, components);
//
//	SpawningComponent* spawnComponent = components.m_entity.AddComponent<SpawningComponent>();
//	TimerComponent* timerComponent = components.m_entity.AddComponent<TimerComponent>();
//	if (!spawnComponent || !timerComponent)
//	{
//		return false;
//	}
//
//#pragma region Test that spawning from an ability without an actor record specified errors
//	AddExpectedErrorPlain
//	(
//		FString::Printf
//		(
//			TEXT("Could not retrieve %s from %s."),
//			ARGUS_NAMEOF(UArgusActorRecord*), 
//			ARGUS_NAMEOF(UAbilityRecord*)
//		)
//	);
//#pragma endregion
//
//	AbilitySystems::CastAbility(abilityRecord, components);
//
//	UArgusActorRecord* argusActorRecord = NewObject<UArgusActorRecord>();
//	if (!argusActorRecord)
//	{
//		ArgusTesting::EndArgusTest();
//		return false;
//	}
//	argusActorRecord->m_id = argusActorRecordId;
//	abilityRecord->m_timeToCastSeconds = 0.0f;
//	abilityRecord->m_argusActorRecordId = argusActorRecordId;
//
//	AbilitySystems::CastAbility(abilityRecord, components);
//	SpawnEntityInfo info;
//	spawnComponent->m_spawnQueue.Peek(info);
//
//#pragma region Test that task component/spawning component have been told to spawn.
//	TestTrue
//	(
//		FString::Printf
//		(
//			TEXT("[%s] Casting a Spawn Ability and confirming that the spawning %s has the correct state."),
//			ARGUS_FUNCNAME,
//			ARGUS_NAMEOF(ArgusEntity)
//		),
//		(info.m_argusActorRecordId == argusActorRecordId) &&
//		(components.m_taskComponent->m_spawningState == ESpawningState::ProcessQueuedSpawnEntity)
//	);
//#pragma endregion
//
//	spawnComponent->m_spawnQueue.Pop();
//	spawnComponent->m_currentQueueSize--;
//	abilityRecord->m_timeToCastSeconds = timeToCastSeconds;
//	AbilitySystems::CastAbility(abilityRecord, components);
//	SpawningSystems::RunSystems(smallTimeStep);
//	spawnComponent->m_spawnQueue.Peek(info);
//
//#pragma region Test that task component/spawning component have been told to spawn.
//	TestTrue
//	(
//		FString::Printf
//		(
//			TEXT("[%s] Casting a Spawn Ability with a cast time and confirming that the spawning %s has the correct state."),
//			ARGUS_FUNCNAME,
//			ARGUS_NAMEOF(ArgusEntity)
//		),
//		(info.m_argusActorRecordId == argusActorRecordId) &&
//		(components.m_taskComponent->m_spawningState == ESpawningState::WaitingToSpawnEntity) &&
//		(spawnComponent->m_spawnTimerHandle.GetTimerIndex() != UINT8_MAX)
//	);
//#pragma endregion
//
//	spawnComponent->m_spawnQueue.Empty();
//	spawnComponent->m_spawnTimerHandle.CancelTimer(components.m_entity);
//	spawnComponent->m_maximumQueueSize = maximumSpawnQueueSize;
//	spawnComponent->m_currentQueueSize = 0u;
//	components.m_taskComponent->m_spawningState = ESpawningState::None;
//	AbilitySystems::CastAbility(abilityRecord, components);
//	AbilitySystems::CastAbility(abilityRecord, components);
//
//#pragma region Test that casting a spawn ability multiple times queues the abilities.
//	TestTrue
//	(
//		FString::Printf
//		(
//			TEXT("[%s] Test that casting a Spawn Ability multiple times properly queues spawn abilities."),
//			ARGUS_FUNCNAME
//		),
//		(spawnComponent->m_currentQueueSize == maximumSpawnQueueSize) &&
//		(components.m_taskComponent->m_spawningState == ESpawningState::ProcessQueuedSpawnEntity)
//	);
//#pragma endregion
//
//	AbilitySystems::CastAbility(abilityRecord, components);
//
//#pragma region Test that casting a Spawn Ability at maximum queue size does not queue additional spawn abilities.
//	TestEqual
//	(
//		FString::Printf
//		(
//			TEXT("[%s] Test that casting a Spawn Ability at maximum queue size does not queue additional spawn abilities."),
//			ARGUS_FUNCNAME
//		),
//		spawnComponent->m_currentQueueSize,
//		maximumSpawnQueueSize
//	);
//#pragma endregion
//
//	SpawningSystems::RunSystems(smallTimeStep);
//	spawnComponent->m_spawnQueue.Peek(info);
//
//#pragma region Test that running spawn systems from a spawn queueinfo will properly set a timer for a spawn cast.
//	TestTrue
//	(
//		FString::Printf
//		(
//			TEXT("[%s] Test that running spawn systems from a spawn queue will properly set a timer for a spawn cast."),
//			ARGUS_FUNCNAME,
//			ARGUS_NAMEOF(ArgusEntity)
//		),
//		(info.m_argusActorRecordId == argusActorRecordId) &&
//		(components.m_taskComponent->m_spawningState == ESpawningState::WaitingToSpawnEntity) &&
//		(spawnComponent->m_spawnTimerHandle.GetTimerIndex() != UINT8_MAX)
//	);
//#pragma endregion
//
//	ArgusTesting::EndArgusTest();
//	return true;
//}

#endif //WITH_AUTOMATION_TESTS