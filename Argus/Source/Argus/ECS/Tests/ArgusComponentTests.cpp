// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusTesting.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentHealthComponentPersistenceTest, "Argus.ECS.Component.HealthComponent.Persistence", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentHealthComponentPersistenceTest::RunTest(const FString& Parameters)
{
	const uint32 expectedSetHealthValue = 500u;
	const uint32 expectedPostResetHealthValue = 1000u;

	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity();
	HealthComponent* healthComponent = entity.AddComponent<HealthComponent>();

	if (!healthComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	healthComponent->m_currentHealth = expectedSetHealthValue;
	healthComponent = entity.GetComponent<HealthComponent>();

#pragma region Test HealthComponent setting health value.
	TestEqual
	(
		FString::Printf(TEXT("[%s] Creating a %s, setting it to %d, then checking the value is %d on retrieval."), ARGUS_FUNCNAME, ARGUS_NAMEOF(HealthComponent), expectedSetHealthValue, expectedSetHealthValue), 
		healthComponent->m_currentHealth,
		expectedSetHealthValue
	);
#pragma endregion

	*healthComponent = HealthComponent();

#pragma region Test that resetting a HealthComponent returns to the default value.
	TestEqual
	(
		FString::Printf(TEXT("[%s] Creating a %s, setting it to %d, resetting it, then checking the value is %d after reset."), ARGUS_FUNCNAME, ARGUS_NAMEOF(HealthComponent), expectedSetHealthValue, expectedPostResetHealthValue), 
		healthComponent->m_currentHealth,
		expectedPostResetHealthValue
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentSpatialPartitioningComponentPersistenceTest, "Argus.ECS.Component.SpatialPartitioningComponent.Persistence", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentSpatialPartitioningComponentPersistenceTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity();
	SpatialPartitioningComponent* spatialPartitioningComponent = entity.AddComponent<SpatialPartitioningComponent>();
	entity.AddComponent<TransformComponent>();

	if (!spatialPartitioningComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	spatialPartitioningComponent->m_argusEntityKDTree.InsertArgusEntityIntoKDTree(entity);
	spatialPartitioningComponent = entity.GetComponent<SpatialPartitioningComponent>();

#pragma region Test SpatialPartitioningComponent by inserting the source entity into the KD Tree.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating a %s, inserting an %s, then checking that it is still there after retrieval."), ARGUS_FUNCNAME, ARGUS_NAMEOF(SpatialPartitioningComponent), ARGUS_NAMEOF(ArgusEntity)),
		spatialPartitioningComponent->m_argusEntityKDTree.DoesArgusEntityExistInKDTree(entity)
	);
#pragma endregion

	*spatialPartitioningComponent = SpatialPartitioningComponent();

#pragma region Test that resetting a SpatialPartitioningComponent returns to the default value.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating a %s, inserting an %s, resetting it, then checking the inserted %s is not present."), ARGUS_FUNCNAME, ARGUS_NAMEOF(SpatialPartitioningComponent), ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity)),
		spatialPartitioningComponent->m_argusEntityKDTree.DoesArgusEntityExistInKDTree(entity)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentTargetingComponentHasEntityTargetTest, "Argus.ECS.Component.TargetingComponent.HasEntityTarget", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentTargetingComponentHasEntityTargetTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	ArgusEntity entity2 = ArgusEntity::CreateEntity();
	TargetingComponent* targetingComponent = entity1.AddComponent<TargetingComponent>();

	if (!targetingComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

#pragma region Test creating a new TargetingComponent and it not having an entity target by default.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating a %s and confirming that it does not have an entity target."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent)),
		targetingComponent->HasEntityTarget()
	);
#pragma endregion
	
	targetingComponent->m_targetEntityId = entity2.GetId();

#pragma region Test setting target entity.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating a %s, setting target to another entity, and confirming that it does have a target."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent)),
		targetingComponent->HasEntityTarget()
	);
#pragma endregion

#pragma region Test setting target entity to a different entity.
	TestEqual
	(
		FString::Printf(TEXT("[%s] Creating a %s, setting target to another entity, then checking the value is the right ID."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent)), 
		targetingComponent->m_targetEntityId, 
		entity2.GetId()
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentTargetingComponentHasLocationTargetTest, "Argus.ECS.Component.TargetingComponent.HasLocationTarget", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentTargetingComponentHasLocationTargetTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity();
	TargetingComponent* targetingComponent = entity.AddComponent<TargetingComponent>();
	const FVector targetLocation = FVector(10.0f, 10.0f, 10.0f);

#pragma region Test creating a TargetingComponent and validating that it does not have a location target by default.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating a %s and confirming that it does not have a location target."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent)), 
		targetingComponent->HasLocationTarget()
	);
#pragma endregion

	targetingComponent->m_targetLocation = targetLocation;

#pragma region Test setting location target.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating a %s, setting target to a location, and confirming that it does have a target."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent)),
		targetingComponent->HasLocationTarget()
	);
#pragma endregion

#pragma region Test setting target location to a different location.
	TestEqual
	(
		FString::Printf(TEXT("[%s] Creating a %s, setting target to a location, then checking the value is the right location."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent)), 
		targetingComponent->m_targetLocation.GetValue(),
		targetLocation
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentIdentityComponentIsInTeamMaskTest, "Argus.ECS.Component.IdentityComponent.IsInTeamMask", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentIdentityComponentIsInTeamMaskTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	IdentityComponent* identityComponent = entity1.AddComponent<IdentityComponent>();

	if (!identityComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	identityComponent->m_team = ETeam::TeamA;
	uint8 teamMask = 0u;

#pragma region Test creating an IdentityComponent of team A and confirming that it is not present in an empty Team mask.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating an %s with %s and confirming that it is not present in an empty team mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent), ARGUS_NAMEOF(ETeam::TeamA)),
		identityComponent->IsInTeamMask(teamMask)
	);
#pragma endregion

	teamMask = 0xFF;

#pragma region Test that the team is present in a full team mask.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating an %s with %s and confirming that it is present in a full team mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent), ARGUS_NAMEOF(ETeam::TeamA)),
		identityComponent->IsInTeamMask(teamMask)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentIdentityComponentAddAllyTeamTest, "Argus.ECS.Component.IdentityComponent.AddAllyTeam", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentIdentityComponentAddAllyTeamTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	ArgusEntity entity2 = ArgusEntity::CreateEntity();
	IdentityComponent* identityComponent1 = entity1.AddComponent<IdentityComponent>();
	IdentityComponent* identityComponent2 = entity2.AddComponent<IdentityComponent>();

	if (!identityComponent1 || !identityComponent2)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	identityComponent1->m_team = ETeam::TeamA;
	identityComponent2->m_team = ETeam::TeamB;

	identityComponent1->AddAllyTeam(ETeam::TeamB);

#pragma region Test creating an IdentityComponent of Team A, adding it as an ally of another IdentityComponent, and then validating that it is in the ally Team mask.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the ally of another, and then testing to make sure it is in the ally Team mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInTeamMask(identityComponent1->m_allies)
	);
#pragma endregion

	identityComponent1->AddEnemyTeam(ETeam::TeamB);
	identityComponent1->AddAllyTeam(ETeam::TeamB);

#pragma region Test adding enemy and then ally and making sure it is in the ally Team mask.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the enemy and then ally of another, and then testing to make sure it is in the ally Team mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInTeamMask(identityComponent1->m_allies)
	);
#pragma endregion

#pragma region Test adding enemy then ally and making sure it is not in the enemy Team mask.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the enemy and then ally of another, and then testing to make sure it is not in the enemy Team mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInTeamMask(identityComponent1->m_enemies)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentIdentityComponentAddEnemyTeamTest, "Argus.ECS.Component.IdentityComponent.AddEnemyTeam", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentIdentityComponentAddEnemyTeamTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	ArgusEntity entity2 = ArgusEntity::CreateEntity();
	IdentityComponent* identityComponent1 = entity1.AddComponent<IdentityComponent>();
	IdentityComponent* identityComponent2 = entity2.AddComponent<IdentityComponent>();

	if (!identityComponent1 || !identityComponent2)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	identityComponent1->m_team = ETeam::TeamA;
	identityComponent2->m_team = ETeam::TeamB;

	identityComponent1->AddEnemyTeam(ETeam::TeamA);

#pragma region Test creating two IdentityComponents, adding one as the enemy of itself, and then validating that it is not in its own enemy mask.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the enemy of itself, and then testing to make sure it is not in the enemy mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent1->IsInTeamMask(identityComponent1->m_enemies)
	);
#pragma endregion

	identityComponent1->AddEnemyTeam(ETeam::TeamB);

#pragma region Test adding an enemy and then validating that it is in the enemy mask.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the enemy of another, and then testing to make sure it is in the enemy Team mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInTeamMask(identityComponent1->m_enemies)
	);
#pragma endregion

	identityComponent1->AddAllyTeam(ETeam::TeamB);
	identityComponent1->AddEnemyTeam(ETeam::TeamB);

#pragma region Test adding a different Team as enemy and validating presence in the enemy mask.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the ally and then enemy of another, and then testing to make sure it is in the enemy Team mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInTeamMask(identityComponent1->m_enemies)
	);
#pragma endregion

#pragma region Test adding an ally and then that as an enemy and making sure the Team is no longer present in the ally mask.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the ally and then enemy of another, and then testing to make sure it is not in the ally Team mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInTeamMask(identityComponent1->m_allies)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentTaskComponentIsExecutingMoveTaskTest, "Argus.ECS.Component.TaskComponent.IsExecutingMoveTask", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentTaskComponentIsExecutingMoveTaskTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity();
	TaskComponent* taskComponent = entity.AddComponent<TaskComponent>();

	if (!taskComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

#pragma region Test that None is not considered a move task.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, and checking if default task is a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	taskComponent->m_movementState = EMovementState::ProcessMoveToLocationCommand;

#pragma region Test that ProcessMoveToLocationCommand is not considered a move task.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, setting task to %s, and checking if that is considered a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::ProcessMoveToLocationCommand)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	taskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;

#pragma region Test that ProcessMoveToEntityCommand is not considered a move task.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, setting task to %s, and checking if that is considered a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::ProcessMoveToEntityCommand)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	taskComponent->m_movementState = EMovementState::FailedToFindPath;

#pragma region Test that FailedToFindPath is not considered a move task.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, setting task to %s, and checking if that is considered a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::FailedToFindPath)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	taskComponent->m_movementState = EMovementState::MoveToLocation;

#pragma region Test that MoveToLocation is considered a move task.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, setting task to %s, and validating that it is considered a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::MoveToLocation)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	taskComponent->m_movementState = EMovementState::MoveToEntity;

#pragma region Test that MoveToEntity is considered a move task.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, setting task to %s, and validating that it is considered a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::MoveToEntity)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	ArgusTesting::StartArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentNavigationComponentResetPathTest, "Argus.ECS.Component.NavigationComponent.ResetPath", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentNavigationComponentResetPathTest::RunTest(const FString& Parameters)
{
	const FVector	point0				= FVector(0.0f, 1.0f, 2.0f);
	const FVector	point1				= FVector(1.0f, 2.0f, 3.0f);
	const FVector	point2				= FVector(2.0f, 3.0f, 4.0f);
	const int32		numPoints			= 3;
	const int32		indexThroughPath	= 1;

	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity();
	NavigationComponent* navigationComponent = entity.AddComponent<NavigationComponent>();

	if (!navigationComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	navigationComponent->m_navigationPoints.SetNumZeroed(numPoints);
	navigationComponent->m_navigationPoints[0] = point0;
	navigationComponent->m_navigationPoints[1] = point1;
	navigationComponent->m_navigationPoints[2] = point2;
	navigationComponent->m_lastPointIndex = indexThroughPath;

#pragma region Test size of navigation path
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing that %s has a size of %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(navigationComponent->m_navigationPoints), numPoints),
		navigationComponent->m_navigationPoints.Num(),
		numPoints
	);
#pragma endregion

#pragma region Test last point index of navigation component
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing that %s has a value of %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(navigationComponent->m_lastPointIndex), indexThroughPath),
		navigationComponent->m_lastPointIndex,
		indexThroughPath
	);
#pragma endregion

#pragma region Test first point of navigation path
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing that the first point of the navigation path is {%f, %f, %f}"), ARGUS_FUNCNAME, point0.X, point0.Y, point0.Z),
		navigationComponent->m_navigationPoints[0],
		point0
	);
#pragma endregion

#pragma region Test second point of navigation path
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing that the second point of the navigation path is {%f, %f, %f}"), ARGUS_FUNCNAME, point1.X, point1.Y, point1.Z),
		navigationComponent->m_navigationPoints[1],
		point1
	);
#pragma endregion

#pragma region Test third point of navigation path
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing that the third point of the navigation path is {%f, %f, %f}"), ARGUS_FUNCNAME, point2.X, point2.Y, point2.Z),
		navigationComponent->m_navigationPoints[2],
		point2
	);
#pragma endregion

	navigationComponent->ResetPath();

#pragma region Test size of navigation path
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing that %s has a size of %d after path reset"), ARGUS_FUNCNAME, ARGUS_NAMEOF(navigationComponent->m_navigationPoints), 0),
		navigationComponent->m_navigationPoints.Num(),
		0
	);
#pragma endregion

#pragma region Test last point index of navigation component
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing that %s has a value of %d after path reset"), ARGUS_FUNCNAME, ARGUS_NAMEOF(navigationComponent->m_lastPointIndex), 0),
		navigationComponent->m_lastPointIndex,
		0
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentNavigationComponentResetQueuedWaypointsTest, "Argus.ECS.Component.NavigationComponent.ResetQueuedWaypoints", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentNavigationComponentResetQueuedWaypointsTest::RunTest(const FString& Parameters)
{
	const FVector	point0		= FVector(0.0f, 1.0f, 2.0f);
	const FVector	point1		= FVector(1.0f, 2.0f, 3.0f);
	const FVector	point2		= FVector(2.0f, 3.0f, 4.0f);
	const int32		numPoints	= 3;

	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity();
	NavigationComponent* navigationComponent = entity.AddComponent<NavigationComponent>();
	if (!navigationComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	navigationComponent->m_queuedWaypoints.PushLast(point0);
	navigationComponent->m_queuedWaypoints.PushLast(point1);
	navigationComponent->m_queuedWaypoints.PushLast(point2);

#pragma region Test first element of queued waypoints
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing that the first queued waypoint is {%f, %f, %f}"), ARGUS_FUNCNAME, point0.X, point0.Y, point0.Z),
		navigationComponent->m_queuedWaypoints.First(),
		point0
	);
#pragma endregion

	navigationComponent->ResetQueuedWaypoints();

#pragma region Test size of queued waypoints
	TestTrue
	(
		FString::Printf(TEXT("[%s] Testing that %s is empty after being reset"), ARGUS_FUNCNAME, ARGUS_NAMEOF(navigationComponent->m_queuedWaypoints), 0),
		navigationComponent->m_queuedWaypoints.IsEmpty()
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentTimerComponentGetTimerFromHandleTest, "Argus.ECS.Component.TimerComponent.GetTimerFromHandle", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentTimerComponentGetTimerFromHandleTest::RunTest(const FString& Parameters)
{
	const float expectedTimerDurationSeconds = 10.0f;
	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity();
	TimerComponent* timerComponent = entity.AddComponent<TimerComponent>();
	if (!timerComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

#pragma region Test that passing in an invalid TimerHandle* errors
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("The variable, %s, is null!"),
			ARGUS_NAMEOF(timerHandle)
		)
	);
#pragma endregion

	timerComponent->GetTimerFromHandle(nullptr);

	TimerHandle handle;
	Timer* timer = timerComponent->GetTimerFromHandle(&handle);

#pragma region Test that retrieving a timer from an unassigned timerhandle is null
	TestNull
	(
		FString::Printf
		(
			TEXT("[%s] Checking that calling %s on an unassigned %s returns nullptr."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerComponent::GetTimerFromHandle),
			ARGUS_NAMEOF(TimerHandle)
		),
		timer
	);
#pragma endregion

	handle.StartTimer(entity, expectedTimerDurationSeconds);
	timer = timerComponent->GetTimerFromHandle(&handle);

#pragma region Test that retrieving a timer from an assigned timerhandle is not null
	TestNotNull
	(
		FString::Printf
		(
			TEXT("[%s] Checking that calling %s on an assigned %s does not return nullptr."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerComponent::GetTimerFromHandle),
			ARGUS_NAMEOF(TimerHandle)
		),
		timer
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

#endif //WITH_AUTOMATION_TESTS