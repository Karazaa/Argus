// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "Systems/TransformSystems.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TransformSystemsFaceTowardsLocationXYTest, "Argus.ECS.Systems.TransformSystems.FaceTowardsLocationXY", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TransformSystemsFaceTowardsLocationXYTest::RunTest(const FString& Parameters)
{
	ArgusEntity entity = ArgusEntity::CreateEntity();
	TransformComponent* transformComponent = entity.AddComponent<TransformComponent>();

	if (!transformComponent)
	{
		return false;
	}

	TestEqual(TEXT("Creating an entity with a default forward transform."), transformComponent->m_transform.GetRotation().GetForwardVector(), FVector(1.0f, 0.0f, 0.0f));

	FVector targetLocation = FVector(-1.0f, 0.0f, 10.0f);
	const FVector expectedForward = FVector(-1.0f, 0.0f, 0.0f);
	TransformSystems::FaceTowardsLocationXY(transformComponent, targetLocation);

	TestEqual(TEXT("Facing entity towards target and confirming it has the proper forward vector."), transformComponent->m_transform.GetRotation().GetForwardVector(), expectedForward);

	targetLocation.X = -0.5f;
	targetLocation.Z = 100.0f;
	TransformSystems::FaceTowardsLocationXY(transformComponent, targetLocation);

	TestEqual(TEXT("Facing entity towards coincident vector and confirming it does not change facing."), transformComponent->m_transform.GetRotation().GetForwardVector(), expectedForward);

	TransformSystems::FaceTowardsLocationXY(transformComponent, FVector::ZeroVector);

	TestEqual(TEXT("Facing entity towards zero vector and confirming it does not change facing."), transformComponent->m_transform.GetRotation().GetForwardVector(), expectedForward);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TransformSystemsMoveAlongPathTest, "Argus.ECS.Systems.TransformSystems.MoveAlongPath", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TransformSystemsMoveAlongPathTest::RunTest(const FString& Parameters)
{
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

	FVector point0 = FVector(0.0f, 0.0f, 0.0f);
	FVector point1 = FVector(-2.0f, 0.0f, 0.0f);
	FVector point2 = FVector(-2.5f, 0.0f, 0.0f);
	FVector point3 = FVector(-2.5f, -0.5f, 0.1f);
	components.m_navigationComponent->m_navigationPoints.reserve(4);
	components.m_navigationComponent->m_navigationPoints.push_back(point0);
	components.m_navigationComponent->m_navigationPoints.push_back(point1);
	components.m_navigationComponent->m_navigationPoints.push_back(point2);
	components.m_navigationComponent->m_navigationPoints.push_back(point3);

	TestEqual(TEXT("Creating initial entity and confirming transform location."), components.m_transformComponent->m_transform.GetLocation(), FVector::ZeroVector);
	TestEqual(TEXT("Creating initial entity and confirming transform rotation."), components.m_transformComponent->m_transform.GetRotation().GetForwardVector(), FVector::ForwardVector);

	TransformSystems::MoveAlongNavigationPath(1.0f, components);

	TestEqual(TEXT("Checking navigation index after moving along path for one second."), components.m_navigationComponent->m_lastPointIndex, 0u);
	TestEqual(TEXT("Checking location after moving along path for one second."), components.m_transformComponent->m_transform.GetLocation(), FVector(-1.0f, 0.0f, 0.0f));
	TestEqual(TEXT("Checking rotation after moving along path for one second."), components.m_transformComponent->m_transform.GetRotation().GetForwardVector(), FVector(-1.0f, 0.0f, 0.0f));

	TransformSystems::MoveAlongNavigationPath(1.0f, components);

	TestEqual(TEXT("Checking navigation index after moving along path for two seconds."), components.m_navigationComponent->m_lastPointIndex, 1u);
	TestEqual(TEXT("Checking location after moving along path for two seconds."), components.m_transformComponent->m_transform.GetLocation(), point1);
	TestEqual(TEXT("Checking rotation after moving along path for two seconds."), components.m_transformComponent->m_transform.GetRotation().GetForwardVector(), FVector(-1.0f, 0.0f, 0.0f));

	TransformSystems::MoveAlongNavigationPath(1.0f, components);

	TestEqual(TEXT("Checking navigation index after moving along path for three seconds."), components.m_navigationComponent->m_lastPointIndex, 2u);
	TestEqual(TEXT("Checking location after moving along path for three seconds."), components.m_transformComponent->m_transform.GetLocation(), point2);
	TestEqual(TEXT("Checking rotation after moving along path for three seconds."), components.m_transformComponent->m_transform.GetRotation().GetForwardVector(), FVector(-1.0f, 0.0f, 0.0f));

	TransformSystems::MoveAlongNavigationPath(1.0f, components);

	TestEqual(TEXT("Checking navigation index after moving along path for four seconds."), components.m_navigationComponent->m_lastPointIndex, 0u);
	const uint16 numPathPoints = components.m_navigationComponent->m_navigationPoints.size();
	TestEqual(TEXT("Checking navigation path point count after moving along path for four seconds."), numPathPoints, 0u);
	TestEqual(TEXT("Checking current task after moving along path for four seconds."), components.m_taskComponent->m_currentTask, ETask::None);
	TestEqual(TEXT("Checking location after moving along path for four seconds."), components.m_transformComponent->m_transform.GetLocation(), point3);
	TestEqual(TEXT("Checking rotation after moving along path for three seconds."), components.m_transformComponent->m_transform.GetRotation().GetForwardVector(), FVector(0.0f, -1.0f, 0.0f));

	return true;
}

#endif //WITH_AUTOMATION_TESTS