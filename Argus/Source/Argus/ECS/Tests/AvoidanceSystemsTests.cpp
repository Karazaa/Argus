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
	IdentityComponent* identityComponent = entity.AddComponent<IdentityComponent>();

	ArgusEntity singletonEntity = ArgusEntity::CreateEntity(ArgusSystemsManager::s_singletonEntityId);
	SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.AddComponent<SpatialPartitioningComponent>();

	if (!components.AreComponentsValidCheck() || !spatialPartitioningComponent || !identityComponent)
	{
		return false;
	}

	components.m_transformComponent->m_desiredSpeedUnitsPerSecond = 100.0f;
	identityComponent->m_team = ETeam::TeamA;
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

	ArgusEntity secondEntity = ArgusEntity::CreateEntity();
	TransformSystems::TransformSystemsComponentArgs secondComponents;
	secondComponents.m_entity = secondEntity;
	secondComponents.m_taskComponent = secondEntity.AddComponent<TaskComponent>();
	secondComponents.m_transformComponent = secondEntity.AddComponent<TransformComponent>();
	secondComponents.m_navigationComponent = secondEntity.AddComponent<NavigationComponent>();
	secondComponents.m_targetingComponent = secondEntity.AddComponent<TargetingComponent>();
	IdentityComponent* secondIdentityComponent = secondEntity.AddComponent<IdentityComponent>();

	if (!secondComponents.AreComponentsValidCheck() || !secondIdentityComponent)
	{
		return false;
	}

	secondComponents.m_transformComponent->m_transform.SetLocation(FVector(100.0f, 0.0f, 0.0f));
	secondIdentityComponent->m_team = ETeam::TeamA;

	components.m_navigationComponent->m_navigationPoints.push_back(FVector::ZeroVector);
	components.m_navigationComponent->m_navigationPoints.push_back(FVector(200.0f, 0.0f, 0.0f));
	components.m_taskComponent->m_currentTask = ETask::MoveToLocation;
	components.m_transformComponent->m_currentVelocity = FVector(100.0f, 0.0f, 0.0f);

	spatialPartitioningComponent->m_argusKDTree.RebuildKDTreeForAllArgusEntities();
	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, 1.0f, components);
	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, 1.0f, secondComponents);

#pragma region Test that a moving entity would maintain its velocity when about to collide with a static but movable entity.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that a moving %s would have the same %s after calling %s when about to collide with a static, but movable, %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(m_proposedAvoidanceVelocity),
			ARGUS_NAMEOF(AvoidanceSystems::ProcessORCAvoidance),
			ARGUS_NAMEOF(ArgusEntity)
		),
		components.m_transformComponent->m_proposedAvoidanceVelocity,
		components.m_transformComponent->m_currentVelocity
	);
#pragma endregion

#pragma region Test that a static but movable entity would move to avoid a moving entity that is about to collide with it.
	TestNotEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that a static, but movable, %s would move to avoid a moving %s that is about to collide with it after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(AvoidanceSystems::ProcessORCAvoidance)
		),
		secondComponents.m_transformComponent->m_proposedAvoidanceVelocity,
		FVector::ZeroVector
	);
#pragma endregion

	return true;
}

#endif //WITH_AUTOMATION_TESTS