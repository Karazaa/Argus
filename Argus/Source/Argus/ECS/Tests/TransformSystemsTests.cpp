// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusSystemsManager.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/TransformSystems.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TransformSystemsGetPathingLocationAtTimeOffsetTest, "Argus.ECS.Systems.TransformSystems.GetPathingLocationAtTimeOffset", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TransformSystemsGetPathingLocationAtTimeOffsetTest::RunTest(const FString& Parameters)
{
	const FVector startLocation = FVector::ZeroVector;
	const FVector oneSecondInPast = FVector(-1.0f, 0.0f, 0.0f);
	const FVector twoSecondsInPast = FVector(-2.0f, 0.0f, 0.0f);
	const FVector oneSecondInFuture = FVector(1.0f, 0.0f, 0.0f);
	const FVector twoSecondsInFuture = FVector(2.0f, 0.0f, 0.0f);
	const FVector expectedForward = FVector(1.0f, 0.0f, 0.0f);

	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity();
	TransformSystems::TransformSystemsComponentArgs components;

	components.m_entity = entity;
	components.m_taskComponent = entity.AddComponent<TaskComponent>();
	components.m_transformComponent = entity.AddComponent<TransformComponent>();
	components.m_navigationComponent = entity.AddComponent<NavigationComponent>();
	components.m_targetingComponent = entity.AddComponent<TargetingComponent>();

	if (!components.AreComponentsValidCheck())
	{
		return false;
	}

	components.m_taskComponent->m_currentTask = ETask::MoveToLocation;
	components.m_transformComponent->m_desiredSpeedUnitsPerSecond = 1.0f;
	components.m_navigationComponent->m_navigationPoints.reserve(5);
	components.m_navigationComponent->m_navigationPoints.push_back(twoSecondsInPast);
	components.m_navigationComponent->m_navigationPoints.push_back(oneSecondInPast);
	components.m_navigationComponent->m_navigationPoints.push_back(startLocation);
	components.m_navigationComponent->m_navigationPoints.push_back(oneSecondInFuture);
	components.m_navigationComponent->m_navigationPoints.push_back(twoSecondsInFuture);
	components.m_navigationComponent->m_lastPointIndex = 2u;

	TransformSystems::GetPathingLocationAtTimeOffsetResults results;
	results.m_outputPredictedLocation = FVector::ZeroVector;
	results.m_outputPredictedForwardDirection = FVector::ZeroVector;
	results.m_navigationIndexOfPredictedLocation = 0u;
	TransformSystems::GetPathingLocationAtTimeOffset(1.0f, components, results);

#pragma region Test navigation index one second in the future
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test %s for one second in the future."), ARGUS_FUNCNAME, ARGUS_NAMEOF(index)),
		results.m_navigationIndexOfPredictedLocation,
		3u
	);
#pragma endregion

#pragma region Test location one second in the future
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} for one second in the future."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), oneSecondInFuture.X, oneSecondInFuture.Y, oneSecondInFuture.Z),
		results.m_outputPredictedLocation,
		oneSecondInFuture
	);
#pragma endregion

#pragma region Test rotation one second in the future
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s forward vector is {%f, %f, %f} for one second in the future."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedForward.X, expectedForward.Y, expectedForward.Z),
		results.m_outputPredictedForwardDirection,
		expectedForward
	);
#pragma endregion

	TransformSystems::GetPathingLocationAtTimeOffset(-1.0f, components, results);

#pragma region Test navigation index one second in the past
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test %s for one second in the past."), ARGUS_FUNCNAME, ARGUS_NAMEOF(index)),
		results.m_navigationIndexOfPredictedLocation,
		1u
	);
#pragma endregion

#pragma region Test location one second in the past
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} for one second in the past."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), oneSecondInPast.X, oneSecondInPast.Y, oneSecondInPast.Z),
		results.m_outputPredictedLocation,
		oneSecondInPast
	);
#pragma endregion

#pragma region Test rotation one second in the past
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s forward vector is {%f, %f, %f} for one second in the past."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedForward.X, expectedForward.Y, expectedForward.Z),
		results.m_outputPredictedForwardDirection,
		expectedForward
	);
#pragma endregion

	TransformSystems::GetPathingLocationAtTimeOffset(2.0f, components, results);

#pragma region Test navigation index two seconds in the future
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test %s for two seconds in the future."), ARGUS_FUNCNAME, ARGUS_NAMEOF(index)),
		results.m_navigationIndexOfPredictedLocation,
		4u
	);
#pragma endregion

#pragma region Test location two seconds in the future
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} for two seconds in the future."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), twoSecondsInFuture.X, twoSecondsInFuture.Y, twoSecondsInFuture.Z),
		results.m_outputPredictedLocation,
		twoSecondsInFuture
	);
#pragma endregion

#pragma region Test rotation two seconds in the future
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s forward vector is {%f, %f, %f} for two seconds in the future."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedForward.X, expectedForward.Y, expectedForward.Z),
		results.m_outputPredictedForwardDirection,
		expectedForward
	);
#pragma endregion

	TransformSystems::GetPathingLocationAtTimeOffset(-2.0f, components, results);

#pragma region Test navigation index two seconds in the past
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test %s for two seconds in the past."), ARGUS_FUNCNAME, ARGUS_NAMEOF(index)),
		results.m_navigationIndexOfPredictedLocation,
		0u
	);
#pragma endregion

#pragma region Test location two seconds in the past
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} for two seconds in the past."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), twoSecondsInPast.X, twoSecondsInPast.Y, twoSecondsInPast.Z),
		results.m_outputPredictedLocation,
		twoSecondsInPast
	);
#pragma endregion

#pragma region Test rotation two seconds in the past
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s forward vector is {%f, %f, %f} for two seconds in the past."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedForward.X, expectedForward.Y, expectedForward.Z),
		results.m_outputPredictedForwardDirection,
		expectedForward
	);
#pragma endregion

	TransformSystems::GetPathingLocationAtTimeOffset(3.0f, components, results);

#pragma region Test navigation index three seconds in the future
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test %s for three seconds in the future."), ARGUS_FUNCNAME, ARGUS_NAMEOF(index)),
		results.m_navigationIndexOfPredictedLocation,
		4u
	);
#pragma endregion

#pragma region Test location three seconds in the future
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} for three seconds in the future."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), twoSecondsInFuture.X, twoSecondsInFuture.Y, twoSecondsInFuture.Z),
		results.m_outputPredictedLocation,
		twoSecondsInFuture
	);
#pragma endregion

#pragma region Test rotation three seconds in the future
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s forward vector is {%f, %f, %f} for three seconds in the future."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedForward.X, expectedForward.Y, expectedForward.Z),
		results.m_outputPredictedForwardDirection,
		expectedForward
	);
#pragma endregion

	TransformSystems::GetPathingLocationAtTimeOffset(-3.0f, components, results);

#pragma region Test navigation index three seconds in the past
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test %s for three seconds in the past."), ARGUS_FUNCNAME, ARGUS_NAMEOF(index)),
		results.m_navigationIndexOfPredictedLocation,
		0u
	);
#pragma endregion

#pragma region Test location three seconds in the past
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} for three seconds in the past."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), twoSecondsInPast.X, twoSecondsInPast.Y, twoSecondsInPast.Z),
		results.m_outputPredictedLocation,
		twoSecondsInPast
	);
#pragma endregion

#pragma region Test rotation three seconds in the past
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s forward vector is {%f, %f, %f} for three seconds in the past."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedForward.X, expectedForward.Y, expectedForward.Z),
		results.m_outputPredictedForwardDirection,
		expectedForward
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TransformSystemsFaceTowardsLocationXYTest, "Argus.ECS.Systems.TransformSystems.FaceTowardsLocationXY", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TransformSystemsFaceTowardsLocationXYTest::RunTest(const FString& Parameters)
{
	const FVector expectedFirstTargetLocation = FVector(-1.0f, 0.0f, 10.0f);
	const FVector expectedSecondTargetLocation = FVector(-0.5f, 0.0f, 100.0f);
	const FVector expectedThirdTargetLocation = FVector(0.5f, 0.0f, 100.0f);
	const FVector expectedForward = FVector(-1.0f, 0.0f, 0.0f);
	const FVector secondExpectedForward = FVector(1.0f, 0.0f, 0.0f);

	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity();
	TransformComponent* transformComponent = entity.AddComponent<TransformComponent>();

	if (!transformComponent)
	{
		return false;
	}

#pragma region Test creating an entity with a default forward transform.
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test creating an %s and validating that its default forward vector equals {%f, %f, %f}"), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), FVector::ForwardVector.X, FVector::ForwardVector.Y, FVector::ForwardVector.Z),
		transformComponent->m_transform.GetRotation().GetForwardVector(), 
		FVector::ForwardVector
	);
#pragma endregion

	TransformSystems::FaceTowardsLocationXY(transformComponent, expectedFirstTargetLocation);

#pragma region Test facing an entity towards a direction and confirming it has the proper forward vector
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test facing %s towards target and confirming it has the proper forward vector, {%f, %f, %f}."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), expectedForward.X, expectedForward.Y, expectedForward.Z),
		transformComponent->m_transform.GetRotation().GetForwardVector(), 
		expectedForward
	);
#pragma endregion

	TransformSystems::FaceTowardsLocationXY(transformComponent, expectedSecondTargetLocation);

#pragma region Test facing an entity towards a coincident vector and confirm that it does not change facing
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test facing %s towards coincident vector and confirming it does not change facing."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)),
		transformComponent->m_transform.GetRotation().GetForwardVector(), 
		expectedForward
	);
#pragma endregion

	TransformSystems::FaceTowardsLocationXY(transformComponent, FVector::ZeroVector);

#pragma region Test facing an entity towards a zero vector and confirming it does not change facing
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test facing %s towards %s and confirming it does not change facing."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(FVector::ZeroVector)),
		transformComponent->m_transform.GetRotation().GetForwardVector(), 
		expectedForward
	);
#pragma endregion

	TransformSystems::FaceTowardsLocationXY(transformComponent, expectedThirdTargetLocation);

#pragma region Test facing an entity towards a direction and confirming it has the proper forward vector
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test facing %s towards target and confirming it has the proper forward vector, {%f, %f, %f}."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), secondExpectedForward.X, secondExpectedForward.Y, secondExpectedForward.Z),
		transformComponent->m_transform.GetRotation().GetForwardVector(),
		secondExpectedForward
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TransformSystemsMoveAlongPathTest, "Argus.ECS.Systems.TransformSystems.MoveAlongPath", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TransformSystemsMoveAlongPathTest::RunTest(const FString& Parameters)
{
	const FVector point0				= FVector(0.0f, 0.0f, 0.0f);
	const FVector expectedTestLocation0 = FVector(-1.0f, 0.0f, 0.0f);
	const FVector expectedTestForward0	= FVector(-1.0f, 0.0f, 0.0f);
	const FVector point1				= FVector(-2.0f, 0.0f, 0.0f);
	const FVector point2				= FVector(-2.5f, 0.0f, 0.0f);
	const FVector point3				= FVector(-2.5f, -0.5f, 0.1f);
	const FVector expectedTestForward1	= FVector(0.0f, -1.0f, 0.0f);
	const float navigationSpeedUnitsPerSecond = 1.0f;
	const float deltaSecondsPerStep = 1.0f;
	const uint32 numExpectedPathPointsAtConclusion = 0u;

	ArgusEntity::FlushAllEntities();

	ArgusEntity singletonEntity = ArgusEntity::CreateEntity(ArgusSystemsManager::s_singletonEntityId);
	singletonEntity.AddComponent<SpatialPartitioningComponent>();

	ArgusEntity entity = ArgusEntity::CreateEntity();
	TransformSystems::TransformSystemsComponentArgs components;

	components.m_entity = entity;
	components.m_taskComponent = entity.AddComponent<TaskComponent>();
	components.m_transformComponent = entity.AddComponent<TransformComponent>();
	components.m_navigationComponent = entity.AddComponent<NavigationComponent>();
	components.m_targetingComponent = entity.AddComponent<TargetingComponent>();

	if (!components.AreComponentsValidCheck())
	{
		return false;
	}

	components.m_taskComponent->m_currentTask = ETask::MoveToLocation;
	components.m_transformComponent->m_desiredSpeedUnitsPerSecond = navigationSpeedUnitsPerSecond;
	components.m_navigationComponent->m_navigationPoints.reserve(4);
	components.m_navigationComponent->m_navigationPoints.push_back(point0);
	components.m_navigationComponent->m_navigationPoints.push_back(point1);
	components.m_navigationComponent->m_navigationPoints.push_back(point2);
	components.m_navigationComponent->m_navigationPoints.push_back(point3);
	float secondCounter = 0.0f;

#pragma region Test creating an initial entity at a specific location
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test creating initial %s and confirming %s location is {%f, %f, %f}"), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TransformComponent), FVector::ZeroVector.X, FVector::ZeroVector.Y, FVector::ZeroVector.Z),
		components.m_transformComponent->m_transform.GetLocation(), 
		FVector::ZeroVector
	);
#pragma endregion

#pragma region Test forward vector of initial entity
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test initial %s %s forward vector is {%f, %f, %f}."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TransformComponent), FVector::ForwardVector.X, FVector::ForwardVector.Y, FVector::ForwardVector.Z),
		components.m_transformComponent->m_transform.GetRotation().GetForwardVector(), 
		FVector::ForwardVector
	);
#pragma endregion

	TWeakObjectPtr<UWorld> dummyWorldPointer;
	AvoidanceSystems::ProcessORCAvoidance(dummyWorldPointer, deltaSecondsPerStep, components);
	TransformSystems::MoveAlongNavigationPath(dummyWorldPointer, deltaSecondsPerStep, components);
	secondCounter += deltaSecondsPerStep;

#pragma region Test navigation index after moving along path for one second
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test %s after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(components.m_navigationComponent->m_lastPointIndex), secondCounter),
		components.m_navigationComponent->m_lastPointIndex, 
		0u
	);
#pragma endregion

#pragma region Test location after moving along path for one second
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedTestLocation0.X, expectedTestLocation0.Y, expectedTestLocation0.Z, secondCounter),
		components.m_transformComponent->m_transform.GetLocation(), 
		expectedTestLocation0
	);
#pragma endregion

#pragma region Test rotation after moving along path for one second
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s forward vector is {%f, %f, %f} after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedTestForward0.X, expectedTestForward0.Y, expectedTestForward0.Z, secondCounter),
		components.m_transformComponent->m_transform.GetRotation().GetForwardVector(), 
		expectedTestForward0
	);
#pragma endregion

	AvoidanceSystems::ProcessORCAvoidance(dummyWorldPointer, deltaSecondsPerStep, components);
	TransformSystems::MoveAlongNavigationPath(dummyWorldPointer, deltaSecondsPerStep, components);
	secondCounter += deltaSecondsPerStep;

#pragma region Test navigation index after moving along path for two seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test %s after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(components.m_navigationComponent->m_lastPointIndex), secondCounter),
		components.m_navigationComponent->m_lastPointIndex, 
		1u
	);
#pragma endregion

#pragma region Test location after moving along path for two seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), point1.X, point1.Y, point1.Z, secondCounter),
		components.m_transformComponent->m_transform.GetLocation(), 
		point1
	);
#pragma endregion

#pragma region Test rotation after moving along path for two seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s forward vector is {%f, %f, %f} after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedTestForward0.X, expectedTestForward0.Y, expectedTestForward0.Z, secondCounter),
		components.m_transformComponent->m_transform.GetRotation().GetForwardVector(), 
		expectedTestForward0
	);
#pragma endregion

	AvoidanceSystems::ProcessORCAvoidance(dummyWorldPointer, deltaSecondsPerStep, components);
	TransformSystems::MoveAlongNavigationPath(dummyWorldPointer, deltaSecondsPerStep, components);
	secondCounter += deltaSecondsPerStep;

#pragma region Test navigation index after moving along path for three seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test %s after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(components.m_navigationComponent->m_lastPointIndex), secondCounter),
		components.m_navigationComponent->m_lastPointIndex, 
		2u
	);
#pragma endregion

	AvoidanceSystems::ProcessORCAvoidance(dummyWorldPointer, deltaSecondsPerStep, components);
	TransformSystems::MoveAlongNavigationPath(dummyWorldPointer, deltaSecondsPerStep, components);
	secondCounter += deltaSecondsPerStep;
	const uint16 numPathPoints = components.m_navigationComponent->m_navigationPoints.size();

#pragma region Test navigation index after moving along path for four seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test %s after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(components.m_navigationComponent->m_lastPointIndex), secondCounter),
		components.m_navigationComponent->m_lastPointIndex, 
		0u
	);
#pragma endregion
	
#pragma region Test navigation path count after moving along path for four seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s path point count is %d after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(NavigationComponent), numExpectedPathPointsAtConclusion, secondCounter),
		numPathPoints, 
		numExpectedPathPointsAtConclusion
	);
#pragma endregion

#pragma region Test current task after moving along path for four seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s current task is %s after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::None), secondCounter),
		components.m_taskComponent->m_currentTask, 
		ETask::None
	);
#pragma endregion

#pragma region Test location after moving along path for four seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), point3.X, point3.Y, point3.Z, secondCounter),
		components.m_transformComponent->m_transform.GetLocation(), 
		point3
	);
#pragma endregion

#pragma region Test rotation after moving along path for four seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s forward vector is {%f, %f, %f} after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedTestForward1.X, expectedTestForward1.Y, expectedTestForward1.Z, secondCounter),
		components.m_transformComponent->m_transform.GetRotation().GetForwardVector(), 
		expectedTestForward1
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TransformSystemsFindEntitiesWithinXYBoundsTest, "Argus.ECS.Systems.TransformSystems.FindEntitiesWithinXYBounds", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TransformSystemsFindEntitiesWithinXYBoundsTest::RunTest(const FString& Parameters)
{
	const FVector	location0					= FVector	(0.0f, 0.0f, 0.0f);
	const FVector	location1					= FVector	(-10.0f, -10.0f, 0.0f);
	const FVector	location2					= FVector	(50.0f, 50.0f, 5000.0f);
	const FVector	location3					= FVector	(100.0f, 100.0f, 100.0f);
	const FVector2D intentionalMissMinXY		= FVector2D	(-200.0f, -200.0f);
	const FVector2D intentionalMissMaxXY		= FVector2D	(-100.0f, -100.0f);
	const FVector2D validHitMinXY				= FVector2D	(0.0f, 0.0f);
	const FVector2D validHitMaxXY				= FVector2D	(100.0f, 100.0f);
	const uint32	totalNumEntities			= 4u;
	const uint32	initialNumExpectedEntities	= 0u;
	const uint32	expectedNumHitEntities		= 3u;
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity();
	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	ArgusEntity entity2 = ArgusEntity::CreateEntity();
	ArgusEntity entity3 = ArgusEntity::CreateEntity();
	TransformComponent* transformComponent0 = entity0.AddComponent<TransformComponent>();
	TransformComponent* transformComponent1 = entity1.AddComponent<TransformComponent>();
	TransformComponent* transformComponent2 = entity2.AddComponent<TransformComponent>();
	TransformComponent* transformComponent3 = entity3.AddComponent<TransformComponent>();

	transformComponent0->m_transform.SetLocation(location0);
	transformComponent1->m_transform.SetLocation(location1);
	transformComponent2->m_transform.SetLocation(location2);
	transformComponent3->m_transform.SetLocation(location3);

	TArray<ArgusEntity> foundEntities;
	foundEntities.Reserve(totalNumEntities);

	FVector2D minXY = intentionalMissMinXY;
	FVector2D maxXY = intentionalMissMaxXY;

	TransformSystems::FindEntitiesWithinXYBounds(minXY, maxXY, foundEntities);

#pragma region Test no entities found after searching far away from spawned entities
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that %d %s are found when querying bounds {%f, %f}-{%f, %f}"), 
			ARGUS_FUNCNAME,
			initialNumExpectedEntities,
			ARGUS_NAMEOF(ArgusEntity),
			minXY.X, minXY.Y,
			maxXY.X, maxXY.Y
		),
		foundEntities.Num(),
		initialNumExpectedEntities
	);
#pragma endregion

	minXY = validHitMinXY;
	maxXY = validHitMaxXY;
	foundEntities.Empty();
	TransformSystems::FindEntitiesWithinXYBounds(minXY, maxXY, foundEntities);

#pragma region Test that Entity0 was found after searching in a bounding box that includes them
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Test that an %s with location {%f, %f, %f} is found when searching bounds {%f, %f}-{%f, %f}"), 
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			location0.X, location0.Y, location0.Z,
			minXY.X, minXY.Y,
			maxXY.X, maxXY.Y
		),
		foundEntities.Contains(entity0)
	);
#pragma endregion

#pragma region Test that Entity2 was found after searching in a bounding box that includes them
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Test that an %s with location {%f, %f, %f} is found when searching bounds {%f, %f}-{%f, %f}"), 
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			location2.X, location2.Y, location2.Z,
			minXY.X, minXY.Y,
			maxXY.X, maxXY.Y
		),
		foundEntities.Contains(entity2)
	);
#pragma endregion

#pragma region Test that Entity3 was found after searching in a bounding box that includes them
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Test that an %s with location {%f, %f, %f} is found when searching bounds {%f, %f}-{%f, %f}"), 
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			location3.X, location3.Y, location3.Z,
			minXY.X, minXY.Y,
			maxXY.X, maxXY.Y
		),
		foundEntities.Contains(entity3)
	);
#pragma endregion

#pragma region Test num positive entities found after searching in a bounding box that includes them
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test that %d %s are found when querying bounds {%f, %f}-{%f, %f}"), 
			ARGUS_FUNCNAME,
			expectedNumHitEntities,
			ARGUS_NAMEOF(ArgusEntity),
			minXY.X, minXY.Y,
			maxXY.X, maxXY.Y
		),
		foundEntities.Num(),
		expectedNumHitEntities
	);
#pragma endregion

	return true;
}

#endif //WITH_AUTOMATION_TESTS