// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "Systems/TransformSystems.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TransformSystemsFaceTowardsLocationXYTest, "Argus.ECS.Systems.TransformSystems.FaceTowardsLocationXY", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TransformSystemsFaceTowardsLocationXYTest::RunTest(const FString& Parameters)
{
	const FVector expectedFirstTargetLocation = FVector(-1.0f, 0.0f, 10.0f);
	const FVector expectedSecondTargetLocation = FVector(-0.5f, 0.0f, 100.0f);
	const FVector expectedForward = FVector(-1.0f, 0.0f, 0.0f);

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

	ArgusEntity entity = ArgusEntity::CreateEntity();
	TransformSystems::TransformSystemsComponentArgs components;

	components.m_taskComponent = entity.AddComponent<TaskComponent>();
	components.m_transformComponent = entity.AddComponent<TransformComponent>();
	components.m_navigationComponent = entity.AddComponent<NavigationComponent>();

	if (!components.AreComponentsValidCheck())
	{
		return false;
	}

	components.m_taskComponent->m_currentTask = ETask::MoveToLocation;
	components.m_navigationComponent->m_navigationSpeedUnitsPerSecond = 1.0f;
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

	TransformSystems::MoveAlongNavigationPath(deltaSecondsPerStep, components);
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

	TransformSystems::MoveAlongNavigationPath(deltaSecondsPerStep, components);
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

	TransformSystems::MoveAlongNavigationPath(deltaSecondsPerStep, components);
	secondCounter += deltaSecondsPerStep;

#pragma region Test navigation index after moving along path for three seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test %s after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(components.m_navigationComponent->m_lastPointIndex), secondCounter),
		components.m_navigationComponent->m_lastPointIndex, 
		2u
	);
#pragma endregion

#pragma region Test location after moving along path for three seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s location is {%f, %f, %f} after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), point2.X, point2.Y, point2.Z, secondCounter),
		components.m_transformComponent->m_transform.GetLocation(),
		point2
	);
#pragma endregion

#pragma region Test rotation after moving along path for three seconds
	TestEqual
	(
		FString::Printf(TEXT("[%s] Test that %s forward vector is {%f, %f, %f} after moving along path for %f seconds."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), expectedTestForward0.X, expectedTestForward0.Y, expectedTestForward0.Z, secondCounter),
		components.m_transformComponent->m_transform.GetRotation().GetForwardVector(), 
		expectedTestForward0
	);
#pragma endregion

	TransformSystems::MoveAlongNavigationPath(deltaSecondsPerStep, components);
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

#endif //WITH_AUTOMATION_TESTS