// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentHealthComponentPersistenceTest, "Argus.ECS.Component.HealthComponent.Persistence", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentHealthComponentPersistenceTest::RunTest(const FString& Parameters)
{
	const uint32 expectedSetHealthValue = 500u;
	const uint32 expectedPostResetHealthValue = 1000u;

	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity();
	HealthComponent* healthComponent = entity.AddComponent<HealthComponent>();

	if (!healthComponent)
	{
		return false;
	}

	healthComponent->m_health = expectedSetHealthValue;
	healthComponent = entity.GetComponent<HealthComponent>();

#pragma region Test HealthComponent setting health value.
	TestEqual
	(
		FString::Printf(TEXT("[%s] Creating a %s, setting it to %d, then checking the value is %d on retrieval."), ARGUS_FUNCNAME, ARGUS_NAMEOF(HealthComponent), expectedSetHealthValue, expectedSetHealthValue), 
		healthComponent->m_health, 
		expectedSetHealthValue
	);
#pragma endregion

	*healthComponent = HealthComponent();

#pragma region Test that resetting a HealthComponent returns to the default value.
	TestEqual
	(
		FString::Printf(TEXT("[%s] Creating a %s, setting it to %d, resetting it, then checking the value is %d after reset."), ARGUS_FUNCNAME, ARGUS_NAMEOF(HealthComponent), expectedSetHealthValue, expectedPostResetHealthValue), 
		healthComponent->m_health,
		expectedPostResetHealthValue
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentTargetingComponentHasEntityTargetTest, "Argus.ECS.Component.TargetingComponent.HasEntityTarget", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentTargetingComponentHasEntityTargetTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	ArgusEntity entity2 = ArgusEntity::CreateEntity();
	TargetingComponent* targetingComponent = entity1.AddComponent<TargetingComponent>();

	if (!targetingComponent)
	{
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

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentTargetingComponentHasLocationTargetTest, "Argus.ECS.Component.TargetingComponent.HasLocationTarget", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentTargetingComponentHasLocationTargetTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

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

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentIdentityComponentIsInFactionMaskTest, "Argus.ECS.Component.IdentityComponent.IsInFactionMask", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentIdentityComponentIsInFactionMaskTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	IdentityComponent* identityComponent = entity1.AddComponent<IdentityComponent>();

	if (!identityComponent)
	{
		return false;
	}

	identityComponent->m_faction = EFaction::FactionA;
	uint8 factionMask = 0u;

#pragma region Test creating an IdentityComponent of faction A and confirming that it is not present in an empty faction mask.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating an %s with %s and confirming that it is not present in an empty faction mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent), ARGUS_NAMEOF(EFaction::FactionA)),
		identityComponent->IsInFactionMask(factionMask)
	);
#pragma endregion

	factionMask = 0xFF;

#pragma region Test that the IdentityComponent's faction is present in a full faction mask.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating an %s with %s and confirming that it is present in a full faction mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent), ARGUS_NAMEOF(EFaction::FactionA)),
		identityComponent->IsInFactionMask(factionMask)
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentIdentityComponentAddAllyFactionTest, "Argus.ECS.Component.IdentityComponent.AddAllyFaction", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentIdentityComponentAddAllyFactionTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	ArgusEntity entity2 = ArgusEntity::CreateEntity();
	IdentityComponent* identityComponent1 = entity1.AddComponent<IdentityComponent>();
	IdentityComponent* identityComponent2 = entity2.AddComponent<IdentityComponent>();

	if (!identityComponent1 || !identityComponent2)
	{
		return false;
	}

	identityComponent1->m_faction = EFaction::FactionA;
	identityComponent2->m_faction = EFaction::FactionB;

	identityComponent1->AddAllyFaction(EFaction::FactionB);

#pragma region Test creating an IdentityComponent of faction A, adding it as an ally of another IdentityComponent, and then validating that it is in the ally faction mask.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the ally of another, and then testing to make sure it is in the ally faction mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInFactionMask(identityComponent1->m_allies)
	);
#pragma endregion

	identityComponent1->AddEnemyFaction(EFaction::FactionB);
	identityComponent1->AddAllyFaction(EFaction::FactionB);

#pragma region Test adding enemy and then ally and making sure it is in the ally faction mask.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the enemy and then ally of another, and then testing to make sure it is in the ally faction mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInFactionMask(identityComponent1->m_allies)
	);
#pragma endregion

#pragma region Test adding enemy then ally and making sure it is not in the enemy faction mask.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the enemy and then ally of another, and then testing to make sure it is not in the enemy faction mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInFactionMask(identityComponent1->m_enemies)
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentIdentityComponentAddEnemyFactionTest, "Argus.ECS.Component.IdentityComponent.AddEnemyFaction", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentIdentityComponentAddEnemyFactionTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	ArgusEntity entity2 = ArgusEntity::CreateEntity();
	IdentityComponent* identityComponent1 = entity1.AddComponent<IdentityComponent>();
	IdentityComponent* identityComponent2 = entity2.AddComponent<IdentityComponent>();

	if (!identityComponent1 || !identityComponent2)
	{
		return false;
	}

	identityComponent1->m_faction = EFaction::FactionA;
	identityComponent2->m_faction = EFaction::FactionB;

	identityComponent1->AddEnemyFaction(EFaction::FactionA);

#pragma region Test creating two IdentityComponents, adding one as the enemy of itself, and then validating that it is not in its own enemy mask.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the enemy of itself, and then testing to make sure it is not in the enemy mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent1->IsInFactionMask(identityComponent1->m_enemies)
	);
#pragma endregion

	identityComponent1->AddEnemyFaction(EFaction::FactionB);

#pragma region Test adding an enemy and then validating that it is in the enemy mask.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the enemy of another, and then testing to make sure it is in the enemy faction mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInFactionMask(identityComponent1->m_enemies)
	);
#pragma endregion

	identityComponent1->AddAllyFaction(EFaction::FactionB);
	identityComponent1->AddEnemyFaction(EFaction::FactionB);

#pragma region Test adding a different faction as enemy and validating presence in the enemy mask.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the ally and then enemy of another, and then testing to make sure it is in the enemy faction mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInFactionMask(identityComponent1->m_enemies)
	);
#pragma endregion

#pragma region Test adding an ally and then that as an enemy and making sure the faction is no longer present in the ally mask.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating two %ss, adding one as the ally and then enemy of another, and then testing to make sure it is not in the ally faction mask."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent)),
		identityComponent2->IsInFactionMask(identityComponent1->m_allies)
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentTaskComponentIsExecutingMoveTaskTest, "Argus.ECS.Component.TaskComponent.IsExecutingMoveTask", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentTaskComponentIsExecutingMoveTaskTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity();
	TaskComponent* taskComponent = entity.AddComponent<TaskComponent>();

	if (!taskComponent)
	{
		return false;
	}

#pragma region Test that None is not considered a move task.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, and checking if default task is a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	taskComponent->m_currentTask = ETask::ProcessMoveToLocationCommand;

#pragma region Test that ProcessMoveToLocationCommand is not considered a move task.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, setting task to %s, and checking if that is considered a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::ProcessMoveToLocationCommand)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	taskComponent->m_currentTask = ETask::ProcessMoveToEntityCommand;

#pragma region Test that ProcessMoveToEntityCommand is not considered a move task.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, setting task to %s, and checking if that is considered a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::ProcessMoveToEntityCommand)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	taskComponent->m_currentTask = ETask::FailedToFindPath;

#pragma region Test that FailedToFindPath is not considered a move task.
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, setting task to %s, and checking if that is considered a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::FailedToFindPath)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	taskComponent->m_currentTask = ETask::MoveToLocation;

#pragma region Test that MoveToLocation is considered a move task.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, setting task to %s, and validating that it is considered a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::MoveToLocation)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	taskComponent->m_currentTask = ETask::MoveToEntity;

#pragma region Test that MoveToEntity is considered a move task.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating an %s, giving it a %s, setting task to %s, and validating that it is considered a move task."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ETask::MoveToEntity)),
		taskComponent->IsExecutingMoveTask()
	);
#pragma endregion

	return true;
}
#endif //WITH_AUTOMATION_TESTS