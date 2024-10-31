// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusSystemsManager.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/TransformSystems.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(AvoidanceSystemsProcessORCAvoidanceTest, "Argus.ECS.Systems.AvoidanceSystems.ProcessORCAvoidance", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool AvoidanceSystemsProcessORCAvoidanceTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity();
	TransformSystems::TransformSystemsComponentArgs components;
	TWeakObjectPtr<UWorld> dummyPointer = nullptr;

	components.m_entity = entity;
	components.m_taskComponent = entity.AddComponent<TaskComponent>();
	components.m_transformComponent = entity.AddComponent<TransformComponent>();
	components.m_navigationComponent = entity.AddComponent<NavigationComponent>();
	components.m_targetingComponent = entity.AddComponent<TargetingComponent>();

	ArgusEntity singletonEntity = ArgusEntity::CreateEntity(ArgusSystemsManager::s_singletonEntityId);
	SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.AddComponent<SpatialPartitioningComponent>();

	if (!components.AreComponentsValidCheck() || !spatialPartitioningComponent)
	{
		return false;
	}

	components.m_transformComponent->m_desiredSpeedUnitsPerSecond = 100.0f;
	spatialPartitioningComponent->m_argusKDTree.RebuildKDTreeForAllArgusEntities();

#pragma region Test that a brand new entity has a zero proposed avoidance velocity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that a new %s has a %s of zero."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(m_proposedAvoidanceVelocity)
		),
		components.m_transformComponent->m_proposedAvoidanceVelocity,
		FVector::ZeroVector
	);
#pragma endregion

	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, 1.0f, components);

#pragma region Test that a lone entity with no velocity would have a proposed avoidance velocity of 0 after running ProcessORCAvoidance
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that a lone %s with no velocity would have a %s of zero after running %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(m_proposedAvoidanceVelocity),
			ARGUS_NAMEOF(AvoidanceSystems::ProcessORCAvoidance)
		),
		components.m_transformComponent->m_proposedAvoidanceVelocity,
		FVector::ZeroVector
	);
#pragma endregion

	return true;
}

#endif //WITH_AUTOMATION_TESTS