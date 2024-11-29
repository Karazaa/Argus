// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusSystemsManager.h"
#include "ArgusTesting.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/TransformSystems.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(AvoidanceSystemsProcessORCAvoidanceTest, "Argus.ECS.Systems.AvoidanceSystems.ProcessORCAvoidance", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool AvoidanceSystemsProcessORCAvoidanceTest::RunTest(const FString& Parameters)
{
	const float deltaTime = 1.0f;
	const float desiredSpeed = 100.0f;
	const FVector velocity = FVector(100.0f, 0.0f, 0.0f);
	const FVector secondVelocity = FVector(-100.0f, 0.0f, 0.0f);
	const FVector secondEntityLocation = FVector(100.0f, 0.0f, 0.0f);
	const FVector targetLocation = FVector(200.0f, 0.0f, 0.0f);
	const FVector secondTargetLocation = FVector(-100.0f, 0.0f, 0.0f);

	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity();
	TransformSystems::TransformSystemsComponentArgs components;
	UWorld* dummyPointer = nullptr;

#pragma region Test that invalid components report the proper error.
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Passed in %s object has invalid component references."),
			ARGUS_NAMEOF(TransformSystemsComponentArgs)
		)
	);
#pragma endregion

	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, deltaTime, components);

	components.m_entity = entity;
	components.m_taskComponent = entity.AddComponent<TaskComponent>();
	components.m_transformComponent = entity.AddComponent<TransformComponent>();
	components.m_navigationComponent = entity.AddComponent<NavigationComponent>();
	components.m_targetingComponent = entity.AddComponent<TargetingComponent>();
	IdentityComponent* identityComponent = entity.AddComponent<IdentityComponent>();

#pragma region Test that an error is reported if there is no singleton entity.
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Could not retrieve singleton %s."),
			ARGUS_NAMEOF(ArgusEntity)
		)
	);
#pragma endregion

	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, deltaTime, components);

	ArgusEntity singletonEntity = ArgusEntity::CreateEntity(ArgusSystemsManager::k_singletonEntityId);

#pragma region Test that an error is reported if there is no SpatialPartitioningComponent on the singleton entity.
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Could not retrieve %s."),
			ARGUS_NAMEOF(SpatialPartitioningComponent*)
		)
	);
#pragma endregion

	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, deltaTime, components);

	SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.AddComponent<SpatialPartitioningComponent>();

	if (!components.AreComponentsValidCheck(true, ARGUS_FUNCNAME) || !spatialPartitioningComponent || !identityComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	components.m_transformComponent->m_desiredSpeedUnitsPerSecond = desiredSpeed;
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

	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, deltaTime, components);

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
	secondComponents.m_transformComponent = secondEntity.AddComponent<TransformComponent>();
	if (!secondComponents.m_transformComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}
	secondComponents.m_transformComponent->m_transform.SetLocation(secondEntityLocation);

	components.m_taskComponent->m_movementState = EMovementState::MoveToLocation;

#pragma region Test that an error is reported if the navigation component is malformed.
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Attempting to process ORCA, but the source %s's %s is in an invalid state."),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(NavigationComponent)
		)
	);
#pragma endregion

	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, deltaTime, components);

	components.m_navigationComponent->m_navigationPoints.push_back(components.m_transformComponent->m_transform.GetLocation());
	components.m_navigationComponent->m_navigationPoints.push_back(targetLocation);
	components.m_transformComponent->m_currentVelocity = velocity;

	spatialPartitioningComponent->m_argusKDTree.RebuildKDTreeForAllArgusEntities();
	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, deltaTime, components);

#pragma region Test that a moving entity would not maintain its velocity when about to collide with a static entity.
	TestNotEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that a moving %s would not maintain the same %s after calling %s when about to collide with a static %s."),
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

#pragma region Test that a moving entity would not stop when about to collide with a static entity.
	TestNotEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that a moving %s would not stop after calling %s when about to collide with a static %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(AvoidanceSystems::ProcessORCAvoidance),
			ARGUS_NAMEOF(ArgusEntity)
		),
		components.m_transformComponent->m_proposedAvoidanceVelocity,
		FVector::ZeroVector
	);
#pragma endregion

#pragma region Test that a static entity would not move when a moving entity is about to collide with it.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that a static %s would not move to avoid a moving %s that is about to collide with it after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(AvoidanceSystems::ProcessORCAvoidance)
		),
		secondComponents.m_transformComponent->m_proposedAvoidanceVelocity,
		FVector::ZeroVector
	);
#pragma endregion

	secondComponents.m_taskComponent = secondEntity.AddComponent<TaskComponent>();
	secondComponents.m_navigationComponent = secondEntity.AddComponent<NavigationComponent>();
	secondComponents.m_targetingComponent = secondEntity.AddComponent<TargetingComponent>();
	IdentityComponent* secondIdentityComponent = secondEntity.AddComponent<IdentityComponent>();

	if (!secondComponents.AreComponentsValidCheck(true, ARGUS_FUNCNAME) || !secondIdentityComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}
	secondIdentityComponent->m_team = ETeam::TeamA;

	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, deltaTime, components);
	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, deltaTime, secondComponents);

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

	secondComponents.m_navigationComponent->m_navigationPoints.push_back(secondComponents.m_transformComponent->m_transform.GetLocation());
	secondComponents.m_navigationComponent->m_navigationPoints.push_back(secondTargetLocation);
	secondComponents.m_taskComponent->m_movementState = EMovementState::MoveToLocation;
	secondComponents.m_transformComponent->m_currentVelocity = secondVelocity;
	secondComponents.m_transformComponent->m_desiredSpeedUnitsPerSecond = desiredSpeed;

	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, deltaTime, components);
	AvoidanceSystems::ProcessORCAvoidance(dummyPointer, deltaTime, secondComponents);

#pragma region Test that a moving entity would not maintain its velocity when about to collide with another moving entity.
	// TODO JAMES: Bug in avoidance. Need to better handle the case where current velocity points to the dead center of another agent.
	//TestNotEqual
	//(
	//	FString::Printf
	//	(
	//		TEXT("[%s] Test that a moving %s would not maintain the same %s after calling %s when about to collide with another moving %s."),
	//		ARGUS_FUNCNAME,
	//		ARGUS_NAMEOF(ArgusEntity),
	//		ARGUS_NAMEOF(m_proposedAvoidanceVelocity),
	//		ARGUS_NAMEOF(AvoidanceSystems::ProcessORCAvoidance),
	//		ARGUS_NAMEOF(ArgusEntity)
	//	),
	//	components.m_transformComponent->m_proposedAvoidanceVelocity,
	//	components.m_transformComponent->m_currentVelocity
	//);
#pragma endregion

#pragma region Test that a moving entity would not stop when about to collide with a moving entity.
	TestNotEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that a moving %s would not stop after calling %s when about to collide with a moving %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(AvoidanceSystems::ProcessORCAvoidance),
			ARGUS_NAMEOF(ArgusEntity)
		),
		components.m_transformComponent->m_proposedAvoidanceVelocity,
		FVector::ZeroVector
	);
#pragma endregion

#pragma region Test that the other moving entity would not maintain its velocity when about to collide with the original moving entity.
	// TODO JAMES: Bug in avoidance. Need to better handle the case where current velocity points to the dead center of another agent.
	//TestNotEqual
	//(
	//	FString::Printf
	//	(
	//		TEXT("[%s] Test that the other moving %s would not maintain the same %s after calling %s when about to collide with a the original moving %s."),
	//		ARGUS_FUNCNAME,
	//		ARGUS_NAMEOF(ArgusEntity),
	//		ARGUS_NAMEOF(m_proposedAvoidanceVelocity),
	//		ARGUS_NAMEOF(AvoidanceSystems::ProcessORCAvoidance),
	//		ARGUS_NAMEOF(ArgusEntity)
	//	),
	//	secondComponents.m_transformComponent->m_proposedAvoidanceVelocity,
	//	secondComponents.m_transformComponent->m_currentVelocity
	//);
#pragma endregion

#pragma region Test that the other moving entity would not stop when about to collide with the original moving entity.
	TestNotEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that the other moving %s would not stop after calling %s when about to collide with the other moving %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(AvoidanceSystems::ProcessORCAvoidance),
			ARGUS_NAMEOF(ArgusEntity)
		),
		secondComponents.m_transformComponent->m_proposedAvoidanceVelocity,
		FVector::ZeroVector
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

#endif //WITH_AUTOMATION_TESTS