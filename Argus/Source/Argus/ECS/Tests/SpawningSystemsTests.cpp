// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityTemplate.h"
#include "ArgusMacros.h"
#include "ArgusTesting.h"
#include "DataComponentDefinitions/TransformComponentData.h"
#include "Misc/AutomationTest.h"
#include "RecordDefinitions/ArgusActorRecord.h"
#include "Systems/SpawningSystems.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(SpawningSystemsSpawnEntityTest, "Argus.ECS.Systems.SpawningSystems.SpawnEntity", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool SpawningSystemsSpawnEntityTest::RunTest(const FString& Parameters)
{
	const float				dummyHeight = 500.0f;
	const float				dummyRadius = 700.0f;
	const float				dummySpeed = 0.1f;
	const uint32			dummyRecordID = 1234u;
	const UEntityPriority	dummyEntityPriority = UEntityPriority::HighPriority;

	ArgusTesting::StartArgusTest();
	UTransformComponentData* transformComponentData = NewObject<UTransformComponentData>();
	UArgusEntityTemplate* entityTemplate = NewObject<UArgusEntityTemplate>();
	UArgusActorRecord* argusActorRecord = NewObject<UArgusActorRecord>();
	SpawnEntityInfo spawnInfo;
	if (!transformComponentData || !entityTemplate || !argusActorRecord)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	transformComponentData->m_height = dummyHeight;
	transformComponentData->m_radius = dummyRadius;

	entityTemplate->m_entityPriority = dummyEntityPriority;
	entityTemplate->m_componentData.Add(transformComponentData);

	argusActorRecord->m_id = dummyRecordID;
	argusActorRecord->m_entityTemplateOverride = entityTemplate;

	SpawningSystemsArgs components;

#pragma region Test that invalid components report the proper error.
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Passed in %s object has invalid component references."),
			ARGUS_NAMEOF(SpawningSystemsArgs)
		),
		EAutomationExpectedErrorFlags::Contains,
		2
	);
#pragma endregion
	SpawningSystems::SpawnEntity(components, spawnInfo, argusActorRecord);

	components.m_entity = ArgusEntity::CreateEntity();
	components.m_spawningComponent = components.m_entity.AddComponent<SpawningComponent>();
	components.m_targetingComponent = components.m_entity.AddComponent<TargetingComponent>();
	components.m_taskComponent = components.m_entity.AddComponent<TaskComponent>();
	components.m_transformComponent = components.m_entity.AddComponent<TransformComponent>();
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		ArgusTesting::EndArgusTest();
		return false;
	}
	components.m_taskComponent->m_spawningState = ESpawningState::SpawningEntity;

#pragma region Test that the to-be-spawned entity does not yet exist.
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Test that the to-be-spawned %s does not yet exist."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity)
		),
		ArgusEntity::DoesEntityExist(static_cast<uint16>(dummyEntityPriority))
	);
#pragma endregion

	SpawningSystems::SpawnEntity(components, spawnInfo, argusActorRecord);

#pragma region Test that the to-be-spawned entity exists after spawning.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Test that the to-be-spawned %s exists after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(SpawningSystems::SpawnEntity)
		),
		ArgusEntity::DoesEntityExist(static_cast<uint16>(dummyEntityPriority))
	);
#pragma endregion

	ArgusEntity spawnedEntity = ArgusEntity::RetrieveEntity(static_cast<uint16>(dummyEntityPriority));
	const TaskComponent* spawnedTaskComponent = spawnedEntity.GetComponent<TaskComponent>();
	const TransformComponent* spawnedTransformComponent = spawnedEntity.GetComponent<TransformComponent>();

#pragma region Test that the spawned entity has a TaskComponent after spawning.
	TestNotNull
	(
		FString::Printf
		(
			TEXT("[%s] Test that the spawned %s has a valid %s after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(TaskComponent),
			ARGUS_NAMEOF(SpawningSystems::SpawnEntity)
		),
		spawnedTaskComponent
	);
#pragma endregion

#pragma region Test that the spawned entity has a TransformComponent after spawning.
	TestNotNull
	(
		FString::Printf
		(
			TEXT("[%s] Test that the spawned %s has a valid %s after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(TransformComponent),
			ARGUS_NAMEOF(SpawningSystems::SpawnEntity)
		),
		spawnedTransformComponent
	);
#pragma endregion

#pragma region Test that the spawned entity has a correct value for its TaskComponents base state.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that the %s of the spawned %s has the correct value for %s after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TaskComponent),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(m_baseState),
			ARGUS_NAMEOF(SpawningSystems::SpawnEntity)
		),
		spawnedTaskComponent->m_baseState,
		EBaseState::SpawnedWaitingForActorTake
	);
#pragma endregion

#pragma region Test that the spawned entity has a correct value for its TaskComponents spawned from record id.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that the %s of the spawned %s has the correct value for %s after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TaskComponent),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(m_spawnedFromArgusActorRecordId),
			ARGUS_NAMEOF(SpawningSystems::SpawnEntity)
		),
		spawnedTaskComponent->m_spawnedFromArgusActorRecordId,
		dummyRecordID
	);
#pragma endregion

#pragma region Test that the spawned entity has a correct value for its TransformComponents height.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that the %s of the spawned %s has the correct value for %s after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TransformComponent),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(m_height),
			ARGUS_NAMEOF(SpawningSystems::SpawnEntity)
		),
		spawnedTransformComponent->m_height,
		dummyHeight
	);
#pragma endregion

#pragma region Test that the spawned entity has a correct value for its TransformComponents radius.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that the %s of the spawned %s has the correct value for %s after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TransformComponent),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(m_radius),
			ARGUS_NAMEOF(SpawningSystems::SpawnEntity)
		),
		spawnedTransformComponent->m_radius,
		dummyRadius
	);
#pragma endregion

#pragma region Test that the spawning entity has the correct value for its TaskComponents spawning state.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that the %s of the spawning %s has the correct value for %s after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TaskComponent),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(m_spawningState),
			ARGUS_NAMEOF(SpawningSystems::SpawnEntity)
		),
		components.m_taskComponent->m_spawningState,
		ESpawningState::None
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

#endif //WITH_AUTOMATION_TESTS