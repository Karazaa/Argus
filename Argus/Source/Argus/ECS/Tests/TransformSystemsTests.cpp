// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusSystemsManager.h"
#include "ArgusTesting.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/TransformSystems.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

//IMPLEMENT_SIMPLE_AUTOMATION_TEST(TransformSystemsMoveAlongPathTest, "Argus.ECS.Systems.TransformSystems.MoveAlongPath", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
//bool TransformSystemsMoveAlongPathTest::RunTest(const FString& Parameters)
//{
//	const FVector point0				= FVector(0.0f, 0.0f, 0.0f);
//	const FVector expectedTestLocation0 = FVector(-1.0f, 0.0f, 0.0f);
//	const FVector expectedTestForward0	= FVector(-1.0f, 0.0f, 0.0f);
//	const FVector point1				= FVector(-2.0f, 0.0f, 0.0f);
//	const FVector point2				= FVector(-2.5f, 0.0f, 0.0f);
//	const FVector point3				= FVector(-2.5f, -0.5f, 0.1f);
//	const FVector expectedTestForward1	= FVector(0.0f, -1.0f, 0.0f);
//	const float navigationSpeedUnitsPerSecond = 1.0f;
//	const float deltaSecondsPerStep = 1.0f;
//	const uint32 numExpectedPathPointsAtConclusion = 0u;
//
//	ArgusTesting::StartArgusTest();
//	ArgusEntity singletonEntity = ArgusEntity::CreateEntity(ArgusECSConstants::k_singletonEntityId);
//	singletonEntity.GetOrAddComponent<SpatialPartitioningComponent>();
//
//	ArgusEntity entity = ArgusEntity::CreateEntity();
//	UWorld* dummyWorldPointer = nullptr;
//	TransformSystems::TransformSystemsComponentArgs components;
//
//#pragma region Test passing in malformed components and getting an error message.
//	AddExpectedErrorPlain
//	(
//		FString::Printf
//		(
//			TEXT("Passed in %s object has invalid component references."),
//			ARGUS_NAMEOF(TransformSystemsComponentArgs)
//		)
//	);
//#pragma endregion
//
//	TransformSystems::MoveAlongNavigationPath(dummyWorldPointer, deltaSecondsPerStep, components);
//
//	components.m_entity = entity;
//	components.m_taskComponent = entity.AddComponent<TaskComponent>();
//	components.m_transformComponent = entity.AddComponent<TransformComponent>();
//	components.m_navigationComponent = entity.AddComponent<NavigationComponent>();
//	components.m_targetingComponent = entity.AddComponent<TargetingComponent>();
//	AvoidanceGroupingComponent* avoidanceGroupingComponent = entity.AddComponent<AvoidanceGroupingComponent>();
//
//	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !avoidanceGroupingComponent)
//	{
//		ArgusTesting::EndArgusTest();
//		return false;
//	}
//
//	components.m_taskComponent->m_movementState = MovementState::MoveToLocation;
//	components.m_transformComponent->m_desiredSpeedUnitsPerSecond = navigationSpeedUnitsPerSecond;
//
//#pragma region Test having an invalid navigation point index fires the proper error.
//	AddExpectedErrorPlain
//	(
//		FString::Printf
//		(
//			TEXT("putting pathfinding in an invalid state.")
//		)
//	);
//#pragma endregion
//
//	TransformSystems::MoveAlongNavigationPath(dummyWorldPointer, deltaSecondsPerStep, components);
//
//	components.m_navigationComponent->m_navigationPoints.Reserve(4);
//	components.m_navigationComponent->m_navigationPoints.Add(point0);
//	components.m_navigationComponent->m_navigationPoints.Add(point1);
//	components.m_navigationComponent->m_navigationPoints.Add(point2);
//	components.m_navigationComponent->m_navigationPoints.Add(point3);
//	float secondCounter = 0.0f;
//
//#pragma region Test creating an initial entity at a specific location
//	TestEqual
//	(
//		FString::Printf(TEXT("[%s] Test creating initial %s and confirming %s location is {%f, %f, %f}"), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TransformComponent), FVector::ZeroVector.X, FVector::ZeroVector.Y, FVector::ZeroVector.Z),
//		components.m_transformComponent->m_location, 
//		FVector::ZeroVector
//	);
//#pragma endregion
//
//#pragma region Test moving along path with an invalid world pointer.
//	AddExpectedErrorPlain
//	(
//		FString::Printf
//		(
//			TEXT("Passed in %s was invalid."),
//			ARGUS_NAMEOF(UWorld*)
//		),
//		EAutomationExpectedMessageFlags::Contains,
//		4
//	);
//#pragma endregion
//
//	AvoidanceSystems::ProcessORCAvoidance(dummyWorldPointer, deltaSecondsPerStep, components, avoidanceGroupingComponent);
//	TransformSystems::MoveAlongNavigationPath(dummyWorldPointer, deltaSecondsPerStep, components);
//	secondCounter += deltaSecondsPerStep;
//
//#pragma region Test navigation index after moving along path for one second
//	TestEqual
//	(
//		FString::Printf(TEXT("[%s] Test %s after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(components.m_navigationComponent->m_lastPointIndex), secondCounter),
//		components.m_navigationComponent->m_lastPointIndex, 
//		0u
//	);
//#pragma endregion
//
//#pragma region Test location after moving along path for one second
//	TestEqual
//	(
//		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedTestLocation0.X, expectedTestLocation0.Y, expectedTestLocation0.Z, secondCounter),
//		components.m_transformComponent->m_location, 
//		expectedTestLocation0
//	);
//#pragma endregion
//
//	AvoidanceSystems::ProcessORCAvoidance(dummyWorldPointer, deltaSecondsPerStep, components, avoidanceGroupingComponent);
//	TransformSystems::MoveAlongNavigationPath(dummyWorldPointer, deltaSecondsPerStep, components);
//	secondCounter += deltaSecondsPerStep;
//
//#pragma region Test navigation index after moving along path for two seconds
//	TestEqual
//	(
//		FString::Printf(TEXT("[%s] Test %s after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(components.m_navigationComponent->m_lastPointIndex), secondCounter),
//		components.m_navigationComponent->m_lastPointIndex, 
//		1u
//	);
//#pragma endregion
//
//#pragma region Test location after moving along path for two seconds
//	TestEqual
//	(
//		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), point1.X, point1.Y, point1.Z, secondCounter),
//		components.m_transformComponent->m_location, 
//		point1
//	);
//#pragma endregion
//
//	AvoidanceSystems::ProcessORCAvoidance(dummyWorldPointer, deltaSecondsPerStep, components, avoidanceGroupingComponent);
//	TransformSystems::MoveAlongNavigationPath(dummyWorldPointer, deltaSecondsPerStep, components);
//	secondCounter += deltaSecondsPerStep;
//
//#pragma region Test navigation index after moving along path for three seconds
//	TestEqual
//	(
//		FString::Printf(TEXT("[%s] Test %s after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(components.m_navigationComponent->m_lastPointIndex), secondCounter),
//		components.m_navigationComponent->m_lastPointIndex, 
//		2u
//	);
//#pragma endregion
//
//	AvoidanceSystems::ProcessORCAvoidance(dummyWorldPointer, deltaSecondsPerStep, components, avoidanceGroupingComponent);
//	TransformSystems::MoveAlongNavigationPath(dummyWorldPointer, deltaSecondsPerStep, components);
//	secondCounter += deltaSecondsPerStep;
//	const int32 numPathPoints = components.m_navigationComponent->m_navigationPoints.Num();
//
//#pragma region Test navigation index after moving along path for four seconds
//	TestEqual
//	(
//		FString::Printf(TEXT("[%s] Test %s after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(components.m_navigationComponent->m_lastPointIndex), secondCounter),
//		components.m_navigationComponent->m_lastPointIndex, 
//		0u
//	);
//#pragma endregion
//	
//#pragma region Test navigation path count after moving along path for four seconds
//	TestEqual
//	(
//		FString::Printf(TEXT("[%s] Test that %s path point count is %d after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(NavigationComponent), numExpectedPathPointsAtConclusion, secondCounter),
//		numPathPoints, 
//		numExpectedPathPointsAtConclusion
//	);
//#pragma endregion
//
//#pragma region Test current task after moving along path for four seconds
//	TestEqual
//	(
//		FString::Printf(TEXT("[%s] Test that %s current task is %s after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::None), secondCounter),
//		components.m_taskComponent->m_movementState, 
//		MovementState::None
//	);
//#pragma endregion
//
//#pragma region Test location after moving along path for four seconds
//	TestEqual
//	(
//		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), point3.X, point3.Y, point3.Z, secondCounter),
//		components.m_transformComponent->m_location, 
//		point3
//	);
//#pragma endregion
//
//	ArgusTesting::EndArgusTest();
//	return true;
//}

#endif //WITH_AUTOMATION_TESTS