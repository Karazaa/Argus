// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusTesting.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityCreateEntityTest, "Argus.ECS.Entity.CreateEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityCreateEntityTest::RunTest(const FString& Parameters)
{
	const uint32 expectedEntityId = ArgusECSConstants::k_maxEntities - 1u;

	ArgusTesting::StartArgusTest();

#pragma region Test attempting to create an ArgusEntity with an invalid ID
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Attempting to retrieve an ID that is beyond %s."),
			ARGUS_NAMEOF(ArgusECSConstants::k_maxEntities)
		)
	);
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Attempting to create an %s with an invalid ID value."),
			ARGUS_NAMEOF(ArgusEntity)
		),
		EAutomationExpectedErrorFlags::Contains,
		2
	);
#pragma endregion

	ArgusEntity entity = ArgusEntity::CreateEntity(ArgusECSConstants::k_maxEntities);

	entity = ArgusEntity::CreateEntity(expectedEntityId);

#pragma region Test creating an entity with a specific ID
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing that creating an %s via %s allows for ID retrieval via %s"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntity::CreateEntity), 
			ARGUS_NAMEOF(ArgusEntity::GetId)
		),
		entity.GetId(), 
		expectedEntityId
	);
#pragma endregion

#pragma region Test creating an ArgusEntity with an already selected value just under maximum.
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Exceeded the maximum number of allowed %s."),
			ARGUS_NAMEOF(ArgusEntity)
		)
	);
#pragma endregion

	entity = ArgusEntity::CreateEntity(expectedEntityId);

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityAssignmentOperatorTest, "Argus.ECS.Entity.AssignmentOperator", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityAssignmentOperatorTest::RunTest(const FString& Parameters)
{
	const uint32 initialEntityId = 20u;
	const uint32 assignedEntityId = 30u;

	ArgusTesting::StartArgusTest();

	ArgusEntity entity0 = ArgusEntity::CreateEntity(initialEntityId);
	ArgusEntity entity1 = ArgusEntity::CreateEntity(assignedEntityId);

#pragma region Test creating an entity with a specific ID
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing creation of %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity)
		),
		entity1.GetId(), 
		assignedEntityId
	);
#pragma endregion

	entity1 = entity0;

#pragma region Test assigning one ArgusEntity to another
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing assigment of %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity)
		),
		entity1.GetId(), 
		initialEntityId
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityEqualsOperatorTest, "Argus.ECS.Entity.EqualsOperator", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityEqualsOperatorTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity0 = ArgusEntity::CreateEntity();
	ArgusEntity entity1 = ArgusEntity::CreateEntity();

#pragma region Test creating two ArgusEntities with different IDs causes them to not be ==
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Testing creation of two %s with different IDs and the equals operator returning false."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity)
		),
		entity0 == entity1
	);
#pragma endregion

	entity1 = entity0;

#pragma region Test creating two ArgusEntities with the same ID causes them to be ==
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Testing creation of two %s with the same ID and the equals operator returning true."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity)
		),
		entity0 == entity1
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityBoolOperatorTest, "Argus.ECS.Entity.BoolOperator", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityBoolOperatorTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();

#pragma region Test that the bool operator of an empty ArgusEntity returns false
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Testing bool operator of empty %s returns false."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity)
		),
		ArgusEntity::k_emptyEntity
	);
#pragma endregion

	ArgusEntity entity = ArgusEntity::CreateEntity();

#pragma region Test that the bool operator of a non empty ArgusEntity returns true
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Testing bool operator of non-empty %s returns true."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity)
		),
		entity
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityCreateInvalidEntityTest, "Argus.ECS.Entity.CreateInvalidEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityCreateInvalidEntityTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::k_emptyEntity;

#pragma region Test that the bool operator of an invalid ArgusEntity returns false
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Testing that calling %s with invalid ID makes an invalid %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::CreateEntity), 
			ARGUS_NAMEOF(ArgusEntity)
		),
		entity
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityFlushAllEntitiesTest, "Argus.ECS.Entity.FlushAllEntities", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityFlushAllEntitiesTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity();

#pragma region Test that the bool operator of an ArgusEntity is true after creation
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Testing if %s exists after creation."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity)
		),
		entity
	);
#pragma endregion

	ArgusEntity::FlushAllEntities();

#pragma region Test that flushing all entities invalidates existing ArgusEntities
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Testing that %s no longer exists after calling %s."),
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntity::FlushAllEntities)
		),
		entity
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityCopyConstructorTest, "Argus.ECS.Entity.CopyConstructor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityCopyConstructorTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity0 = ArgusEntity::CreateEntity();
	ArgusEntity entity1 = ArgusEntity(entity0);

#pragma region Test that the copy constructor copies one ArgusEntity to another
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing if copy constructor successfully copies one %s to another."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity)
		),
		entity0, 
		entity1
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityAutoIncrementIdTest, "Argus.ECS.Entity.AutoIncrementId", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityAutoIncrementIdTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity0 = ArgusEntity::CreateEntity();
	ArgusEntity entity1 = ArgusEntity::CreateEntity();

#pragma region Test that creating an ArgusEntity with none existing assigns the ID of 0
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing %s Id is 0."),
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(entity0)
		),
		entity0.GetId(), 
		0u
	);
#pragma endregion

#pragma region Test that the next ArgusEntity created has an id of 1
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing %s Id is 1 after auto increment."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(entity1)
		),
		entity1.GetId(), 
		1u
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityCopyEntityTest, "Argus.ECS.Entity.CopyEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityCopyEntityTest::RunTest(const FString& Parameters)
{
	const uint32 expectedId = 20u;

	ArgusTesting::StartArgusTest();
	ArgusEntity entity0 = ArgusEntity::CreateEntity(expectedId);
	ArgusEntity entity1 = entity0;
	ArgusEntity entity2 = ArgusEntity(entity0);

#pragma region Test that the copy operator successfully copies from one ArgusEntity to another
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing %s Id is %d after assignment."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(entity0), 
			expectedId
		),
		entity1.GetId(), 
		expectedId
	);
#pragma endregion

#pragma region Test that the copy constructor successfully copies from one ArgusEntity to another
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing %s Id is %d after copy constructor."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(entity2), 
			expectedId
		),
		entity2.GetId(),
		expectedId
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityDoesEntityExistTest, "Argus.ECS.Entity.DoesEntityExist", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityDoesEntityExistTest::RunTest(const FString& Parameters)
{
	const uint32 expectedId = 20u;
	const uint32 fakeId = 30u;

	ArgusTesting::StartArgusTest();
	ArgusEntity entity0 = ArgusEntity::CreateEntity(expectedId);

#pragma region Test that an ArgusEntity exists after being created
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Testing if %s returns true on a currently used ID."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::DoesEntityExist)
		),
		ArgusEntity::DoesEntityExist(expectedId)
	);
#pragma endregion

#pragma region Test that an ArgusEntity that does not exists is not listed as existing
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Testing if %s return false on a currently unused ID."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::DoesEntityExist)
		),
		ArgusEntity::DoesEntityExist(fakeId)
	);
#pragma endregion

#pragma region Test that an invalid entity ID is not listed as existing
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Testing if %s returns false on and ID of "), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::DoesEntityExist), 
			ARGUS_NAMEOF(ArgusECSConstants::k_maxEntities)
		),
		ArgusEntity::DoesEntityExist(ArgusECSConstants::k_maxEntities)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityRetrieveEntityTest, "Argus.ECS.Entity.RetrieveEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityRetrieveEntityTest::RunTest(const FString& Parameters)
{
	const uint32 expectedId = 20u;
	const uint32 fakeId = 30u;

	ArgusTesting::StartArgusTest();
	ArgusEntity::CreateEntity(expectedId);
	ArgusEntity existingEntity = ArgusEntity::RetrieveEntity(expectedId);
	ArgusEntity fakeEntity = ArgusEntity::RetrieveEntity(fakeId);
	ArgusEntity outOfRangeEntity = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_maxEntities);

#pragma region Test creating an ArgusEntity, retrieving it, and that its bool operator returns true
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating an %s, attempting to retrieve it, and testing that the returned %s is valid."),
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntity)
		),
		existingEntity
	);
#pragma endregion

#pragma region Test that a retrieved ArgusEntity has the correct ID
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing if retrieved %s has correct value %d."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity), 
			expectedId
		),
		existingEntity.GetId(), 
		expectedId
	);
#pragma endregion

#pragma region Test that retrieving a fake ArgusEntity does not return a valid ArgusEntity
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Making sure retrieving a fake %s doesn't return a valid %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntity)
		),
		fakeEntity
	);
#pragma endregion

#pragma region Test that attempting to retrieve an out of range entity ID does not return a valid ArgusEntity
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Making sure retrieving an ID of %s doesn't return a valid %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusECSConstants::k_maxEntities), 
			ARGUS_NAMEOF(ArgusEntity)
		),
		outOfRangeEntity
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityAddGetHealthComponentTest, "Argus.ECS.Entity.AddGetHealthComponent", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityAddGetHealthComponentTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity();
	HealthComponent* healthComponent = entity.GetComponent<HealthComponent>();

#pragma region Test creating an empty ArgusEntity and attempting to retrieve a non-existent HealthComponent
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating an %s, attempting to retrieve %s before adding it."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(HealthComponent)
		),
		healthComponent == nullptr
	);
#pragma endregion

	healthComponent = entity.AddComponent<HealthComponent>();

#pragma region Test adding a HealthComponent and then retrieving it
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating an %s, attempting to retrieve %s after adding it."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(HealthComponent)
		),
		healthComponent != nullptr
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityGetOrAddComponentTest, "Argus.ECS.Entity.GetOrAddComponent", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityGetOrAddComponentTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity();
	HealthComponent* healthComponent = entity.GetOrAddComponent<HealthComponent>();

#pragma region Test creating an ArgusEntity, getting or adding a HealthComponent and making sure a component is added
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Creating an %s, calling %s and making sure a %s is added."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntity::GetOrAddComponent), 
			ARGUS_NAMEOF(HealthComponent)
		),
		healthComponent == nullptr
	);
#pragma endregion

	healthComponent->m_health = 400u;
	healthComponent = entity.GetOrAddComponent<HealthComponent>();

#pragma region Test creating an ArgusEntity. getting or adding a HealthComponent twice, and making sure the same component is returned
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating an %s, calling %s twice and making sure the same %s is returned."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntity::GetOrAddComponent), 
			ARGUS_NAMEOF(HealthComponent)
		),
		healthComponent->m_health, 
		400u
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityGetNextLowestUntakenIdTest, "Argus.ECS.Entity.GetNextLowestUntakenId", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityGetNextLowestUntakenIdTest::RunTest(const FString& Parameters)
{
	const uint16 initialIdToQuery = 0u;

	ArgusTesting::StartArgusTest();
	uint16 nextLowestId = ArgusEntity::GetNextLowestUntakenId(initialIdToQuery);

#pragma region Test getting the next lowest ID when there are no entities
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing result of %s when there are no %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetNextLowestUntakenId), 
			ARGUS_NAMEOF(ArgusEntity)
		),
		nextLowestId,
		initialIdToQuery
	);
#pragma endregion

	nextLowestId = ArgusEntity::GetNextLowestUntakenId(initialIdToQuery);

#pragma region Test it again to validate that it does not change state.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing result of %s when there are no %s a second time."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetNextLowestUntakenId), 
			ARGUS_NAMEOF(ArgusEntity)
		),
		nextLowestId,
		initialIdToQuery
	);
#pragma endregion

	ArgusEntity::CreateEntity(initialIdToQuery);
	nextLowestId = ArgusEntity::GetNextLowestUntakenId(initialIdToQuery);

#pragma region Test getting next lowest untaken ID on a taken ID.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing result of %s when there is a %s with the target ID."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetNextLowestUntakenId), 
			ARGUS_NAMEOF(ArgusEntity)
		),
		nextLowestId,
		initialIdToQuery + 1u
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityGetTakenEntityIdRangeTest, "Argus.ECS.Entity.GetTakenEntityIdRange", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityGetTakenEntityIdRangeTest::RunTest(const FString& Parameters)
{
	const uint16 entity0Id = 500u;
	const uint16 entity1Id = 10u;

	ArgusTesting::StartArgusTest();

#pragma region Test initial state of lowest entity ID
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing initial value of %s and making sure it equals %d."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetLowestTakenEntityId), 
			ArgusECSConstants::k_maxEntities
		),
		ArgusEntity::GetLowestTakenEntityId(),
		ArgusECSConstants::k_maxEntities
	);
#pragma endregion

#pragma region Test initial state of highest entity ID
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing initial value of %s and making sure it equals %d."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetHighestTakenEntityId), 
			ArgusECSConstants::k_maxEntities
		),
		ArgusEntity::GetHighestTakenEntityId(),
		0u
	);
#pragma endregion

	ArgusEntity::CreateEntity(entity0Id);

#pragma region Test state of lowest entity ID after adding a single entity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing value of %s after adding a single %s with ID %d"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetLowestTakenEntityId), 
			ARGUS_NAMEOF(ArgusEntity), 
			entity0Id
		),
		ArgusEntity::GetLowestTakenEntityId(),
		entity0Id
	);
#pragma endregion

#pragma region Test state of highest entity ID after adding a single entity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing value of %s after adding a single %s with ID %d"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetHighestTakenEntityId), 
			ARGUS_NAMEOF(ArgusEntity), 
			entity0Id
		),
		ArgusEntity::GetHighestTakenEntityId(),
		entity0Id
	);
#pragma endregion

	ArgusEntity::CreateEntity(entity1Id);

#pragma region Test state of lowest entity ID after adding a two entities
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing value of %s after adding two %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetLowestTakenEntityId), 
			ARGUS_NAMEOF(ArgusEntity)
		),
		ArgusEntity::GetLowestTakenEntityId(),
		entity1Id
	);
#pragma endregion

#pragma region Test state of highest entity ID after adding a two entities
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing value of %s after adding two %s,"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetHighestTakenEntityId), 
			ARGUS_NAMEOF(ArgusEntity)
		),
		ArgusEntity::GetHighestTakenEntityId(),
		entity0Id
	);
#pragma endregion

	ArgusEntity::FlushAllEntities();

#pragma region Test state of lowest entity ID after flushing all entities
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing value of %s and making sure it equals %d after calling %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetLowestTakenEntityId), 
			ArgusECSConstants::k_maxEntities, 
			ARGUS_NAMEOF(ArgusEntity::FlushAllEntities)
		),
		ArgusEntity::GetLowestTakenEntityId(),
		ArgusECSConstants::k_maxEntities
	);
#pragma endregion

#pragma region Test state of highest entity ID after flusing all entities
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing value of %s and making sure it equals %d after calling %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetHighestTakenEntityId), 
			ArgusECSConstants::k_maxEntities, 
			ARGUS_NAMEOF(ArgusEntity::FlushAllEntities)
		),
		ArgusEntity::GetHighestTakenEntityId(),
		0u
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityDestroyEntityTest, "Argus.ECS.Entity.DestroyEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityDestroyEntityTest::RunTest(const FString& Parameters)
{
	const uint16 expectedLowestTakenIdAfterDestroy = ArgusECSConstants::k_maxEntities;
	const uint16 expectedHighestTakenIdAfterDestroy = 0u;
	const uint16 entity0Id = 500u;

	ArgusTesting::StartArgusTest();
	ArgusEntity entity = ArgusEntity::CreateEntity(entity0Id);

#pragma region Test the creation of a single entity
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Testing the creation of one %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity)
		),
		entity
	);
#pragma endregion

#pragma region Test state of lowest entity ID after adding a single entity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing value of %s after adding a single %s with ID %d"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetLowestTakenEntityId), 
			ARGUS_NAMEOF(ArgusEntity), 
			entity0Id
		),
		ArgusEntity::GetLowestTakenEntityId(),
		entity0Id
	);
#pragma endregion

#pragma region Test state of highest entity ID after adding a single entity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing value of %s after adding a single %s with ID %d"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity::GetHighestTakenEntityId), 
			ARGUS_NAMEOF(ArgusEntity), 
			entity0Id
		),
		ArgusEntity::GetHighestTakenEntityId(),
		entity0Id
	);
#pragma endregion

	if (!entity)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	TaskComponent* taskComponent = entity.AddComponent<TaskComponent>();
	if (!taskComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}
	taskComponent->m_attackingState = EAttackingState::Attacking;

#pragma region Test that the attacking state of the spawned task component is correct.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing that the value of %s equals %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(taskComponent->m_attackingState), 
			ARGUS_NAMEOF(EAttackingState::Attacking)
		),
		taskComponent->m_attackingState,
		EAttackingState::Attacking
	);
#pragma endregion

	ArgusEntity::DestroyEntity(entity);

#pragma region Test that the entity is cleared after calling ArgusEntity::DestroyEntity
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Testing that the %s was cleared after calling %s."),
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntity::DestroyEntity)
		),
		entity
	);
#pragma endregion

#pragma region Test that the attacking state of the spawned task component is correct after calling ArgusEntity::DestroyEntity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing that the value of %s equals %s after calling %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(taskComponent->m_attackingState), 
			ARGUS_NAMEOF(EAttackingState::None), 
			ARGUS_NAMEOF(ArgusEntity::DestroyEntity)
		),
		taskComponent->m_attackingState,
		EAttackingState::None
	);
#pragma endregion

#pragma region Test state of lowest entity ID after calling ArgusEntity::DestroyEntity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing value of %s after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity::GetLowestTakenEntityId),
			ARGUS_NAMEOF(ArgusEntity::DestroyEntity)
		),
		ArgusEntity::GetLowestTakenEntityId(),
		expectedLowestTakenIdAfterDestroy
	);
#pragma endregion

#pragma region Test state of highest entity ID after adding a single entity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing value of %s after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity::GetHighestTakenEntityId),
			ARGUS_NAMEOF(ArgusEntity::DestroyEntity)
		),
		ArgusEntity::GetHighestTakenEntityId(),
		expectedHighestTakenIdAfterDestroy
	);
#pragma endregion

	entity = ArgusEntity::CreateEntity(entity0Id);

#pragma region Test the creation of a single entity
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Testing the creation of one %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity)
		),
		ArgusEntity::DoesEntityExist(entity0Id)
	);
#pragma endregion

	ArgusEntity::DestroyEntity(entity0Id);

#pragma region Test the existence of a single entity after calling ArgusEntity::DestroyEntity
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Testing the existence of a %s after calling %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity::DestroyEntity)
		),
		ArgusEntity::DoesEntityExist(entity0Id)
	);
#pragma endregion

	entity = ArgusEntity::k_emptyEntity;

#pragma region Test attempting to destroy an invalid ArgusEntity.
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Attempting to destroy an %s that doesn't exist."),
			ARGUS_NAMEOF(ArgusEntity)
		),
		EAutomationExpectedMessageFlags::Contains,
		2
	);
#pragma endregion

	ArgusEntity::DestroyEntity(entity);
	ArgusEntity::DestroyEntity(ArgusECSConstants::k_maxEntities);

	ArgusTesting::EndArgusTest();
	return true;
}

#endif //WITH_AUTOMATION_TESTS
