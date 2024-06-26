// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityGetIdTest, "Argus.ECS.Entity.GetId", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityGetIdTest::RunTest(const FString& Parameters)
{
	const uint32 expectedEntityId = 20u;

	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity(expectedEntityId);

#pragma region Test creating an entity with a specific ID
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing that creating an %s via %s allows for ID retrieval via %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity::CreateEntity), ARGUS_NAMEOF(ArgusEntity::GetId)),
		entity.GetId(), 
		expectedEntityId
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityAssignmentOperatorTest, "Argus.ECS.Entity.AssignmentOperator", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityAssignmentOperatorTest::RunTest(const FString& Parameters)
{
	const uint32 initialEntityId = 20u;
	const uint32 assignedEntityId = 30u;

	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity(initialEntityId);
	ArgusEntity entity1 = ArgusEntity::CreateEntity(assignedEntityId);

#pragma region Test creating an entity with a specific ID
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing creation of %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)),
		entity1.GetId(), 
		assignedEntityId
	);
#pragma endregion

	entity1 = entity0;

#pragma region Test assigning one ArgusEntity to another
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing assigment of %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)),
		entity1.GetId(), 
		initialEntityId
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityEqualsOperatorTest, "Argus.ECS.Entity.EqualsOperator", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityEqualsOperatorTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity();
	ArgusEntity entity1 = ArgusEntity::CreateEntity();

#pragma region Test creating two ArgusEntities with different IDs causes them to not be ==
	TestFalse
	(
		FString::Printf(TEXT("[%s] Testing creation of two %s with different IDs and the equals operator returning false."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)),
		entity0 == entity1
	);
#pragma endregion

	entity1 = entity0;

#pragma region Test creating two ArgusEntities with the same ID causes them to be ==
	TestTrue
	(
		FString::Printf(TEXT("[%s] Testing creation of two %s with the same ID and the equals operator returning true."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)),
		entity0 == entity1
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityBoolOperatorTest, "Argus.ECS.Entity.BoolOperator", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityBoolOperatorTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

#pragma region Test that the bool operator of an empty ArgusEntity returns false
	TestFalse
	(
		FString::Printf(TEXT("[%s] Testing bool operator of empty %s returns false."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)),
		ArgusEntity::s_emptyEntity
	);
#pragma endregion

	ArgusEntity entity = ArgusEntity::CreateEntity();

#pragma region Test that the bool operator of a non empty ArgusEntity returns true
	TestTrue
	(
		FString::Printf(TEXT("[%s] Testing bool operator of non-empty %s returns true."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)),
		entity
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityCreateInvalidEntityTest, "Argus.ECS.Entity.CreateInvalidEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityCreateInvalidEntityTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity(ArgusECSConstants::k_maxEntities);

#pragma region Test that the bool operator of an invalid ArgusEntity returns false
	TestFalse
	(
		FString::Printf(TEXT("[%s] Testing that calling %s with invalid ID makes an invalid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity::CreateEntity), ARGUS_NAMEOF(ArgusEntity)),
		entity
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityFlushAllEntitiesTest, "Argus.ECS.Entity.FlushAllEntities", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityFlushAllEntitiesTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity();

#pragma region Test that the bool operator of an ArgusEntity is true after creation
	TestTrue
	(
		FString::Printf(TEXT("[%s] Testing if %s exists after creation."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)),
		entity
	);
#pragma endregion

	ArgusEntity::FlushAllEntities();

#pragma region Test that flushing all entities invalidates existing ArgusEntities
	TestFalse
	(
		FString::Printf(TEXT("[%s] Testing that %s no longer exists after calling %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity::FlushAllEntities)),
		entity
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityCopyConstructorTest, "Argus.ECS.Entity.CopyConstructor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityCopyConstructorTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity();
	ArgusEntity entity1 = ArgusEntity(entity0);

#pragma region Test that the copy constructor copies one ArgusEntity to another
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing if copy constructor successfully copies one %s to another."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)),
		entity0, 
		entity1
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityAutoIncrementIdTest, "Argus.ECS.Entity.AutoIncrementId", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityAutoIncrementIdTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity();
	ArgusEntity entity1 = ArgusEntity::CreateEntity();

#pragma region Test that creating an ArgusEntity with none existing assigns the ID of 0
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing %s Id is 0."), ARGUS_FUNCNAME, ARGUS_NAMEOF(entity0)),
		entity0.GetId(), 
		0u
	);
#pragma endregion

#pragma region Test that the next ArgusEntity created has an id of 1
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing %s Id is 1 after auto increment."), ARGUS_FUNCNAME, ARGUS_NAMEOF(entity1)),
		entity1.GetId(), 
		1u
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityCopyEntityTest, "Argus.ECS.Entity.CopyEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityCopyEntityTest::RunTest(const FString& Parameters)
{
	const uint32 expectedId = 20u;

	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity(expectedId);
	ArgusEntity entity1 = entity0;
	ArgusEntity entity2 = ArgusEntity(entity0);

#pragma region Test that the copy operator successfully copies from one ArgusEntity to another
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing %s Id is %d after assignment."), ARGUS_FUNCNAME, ARGUS_NAMEOF(entity0), expectedId),
		entity1.GetId(), 
		expectedId
	);
#pragma endregion

#pragma region Test that the copy constructor successfully copies from one ArgusEntity to another
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing %s Id is %d after copy constructor."), ARGUS_FUNCNAME, ARGUS_NAMEOF(entity2), expectedId),
		entity2.GetId(),
		expectedId
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityDoesEntityExistTest, "Argus.ECS.Entity.DoesEntityExist", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityDoesEntityExistTest::RunTest(const FString& Parameters)
{
	const uint32 expectedId = 20u;
	const uint32 fakeId = 30u;

	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity(expectedId);

#pragma region Test that an ArgusEntity exists after being created
	TestTrue
	(
		FString::Printf(TEXT("[%s] Testing if %s returns true on a currently used ID."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity::DoesEntityExist)),
		ArgusEntity::DoesEntityExist(expectedId)
	);
#pragma endregion

#pragma region Test that an ArgusEntity that does not exists is not listed as existing
	TestFalse
	(
		FString::Printf(TEXT("[%s] Testing if %s return false on a currently unused ID."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity::DoesEntityExist)),
		ArgusEntity::DoesEntityExist(fakeId)
	);
#pragma endregion

#pragma region Test that an invalid entity ID is not listed as existing
	TestFalse
	(
		FString::Printf(TEXT("[%s] Testing if %s returns false on and ID of "), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity::DoesEntityExist), ARGUS_NAMEOF(ArgusECSConstants::k_maxEntities)),
		ArgusEntity::DoesEntityExist(ArgusECSConstants::k_maxEntities)
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityRetrieveEntityTest, "Argus.ECS.Entity.RetrieveEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityRetrieveEntityTest::RunTest(const FString& Parameters)
{
	const uint32 expectedId = 20u;
	const uint32 fakeId = 30u;

	ArgusEntity::FlushAllEntities();

	ArgusEntity::CreateEntity(expectedId);
	ArgusEntity existingEntity = ArgusEntity::RetrieveEntity(expectedId);
	ArgusEntity fakeEntity = ArgusEntity::RetrieveEntity(fakeId);
	ArgusEntity outOfRangeEntity = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_maxEntities);

#pragma region Test creating an ArgusEntity, retrieving it, and that its bool operator returns true
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating an %s, attempting to retrieve it, and testing that the returned %s is valid."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity)),
		existingEntity
	);
#pragma endregion

#pragma region Test that a retrieved ArgusEntity has the correct ID
	TestEqual
	(
		FString::Printf(TEXT("[%s] Testing if retrieved %s has correct value %d."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), expectedId),
		existingEntity.GetId(), 
		expectedId
	);
#pragma endregion

#pragma region Test that retrieving a fake ArgusEntity does not return a valid ArgusEntity
	TestFalse
	(
		FString::Printf(TEXT("[%s] Making sure retrieving a fake %s doesn't return a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity)),
		fakeEntity
	);
#pragma endregion

#pragma region Test that attempting to retrieve an out of range entity ID does not return a valid ArgusEntity
	TestFalse
	(
		FString::Printf(TEXT("[%s] Making sure retrieving an ID of %s doesn't return a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusECSConstants::k_maxEntities), ARGUS_NAMEOF(ArgusEntity)),
		outOfRangeEntity
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityAddGetHealthComponentTest, "Argus.ECS.Entity.AddGetHealthComponent", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityAddGetHealthComponentTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity();
	HealthComponent* healthComponent = entity.GetComponent<HealthComponent>();

#pragma region Test creating an empty ArgusEntity and attempting to retrieve a non-existent HealthComponent
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating an %s, attempting to retrieve %s before adding it."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(HealthComponent)),
		healthComponent == nullptr
	);
#pragma endregion

	healthComponent = entity.AddComponent<HealthComponent>();

#pragma region Test adding a HealthComponent and then retrieving it
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating an %s, attempting to retrieve %s after adding it."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(HealthComponent)),
		healthComponent != nullptr
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityGetOrAddComponentTest, "Argus.ECS.Entity.GetOrAddComponentTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityGetOrAddComponentTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity();
	HealthComponent* healthComponent = entity.GetOrAddComponent<HealthComponent>();

#pragma region Test creating an ArgusEntity, getting or adding a HealthComponent and making sure a component is added
	TestFalse
	(
		FString::Printf(TEXT("[%s] Creating an %s, calling %s and making sure a %s is added."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity::GetOrAddComponent), ARGUS_NAMEOF(HealthComponent)),
		healthComponent == nullptr
	);
#pragma endregion

	healthComponent->m_health = 400u;
	healthComponent = entity.GetOrAddComponent<HealthComponent>();

#pragma region Test crreating an ArgusEntity. getting or adding a HealthComponent twice, and making sure the same component is returned
	TestEqual
	(
		FString::Printf(TEXT("[%s] Creating an %s, calling %s twice and making sure the same %s is returned."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity::GetOrAddComponent), ARGUS_NAMEOF(HealthComponent)),
		healthComponent->m_health, 
		400u
	);
#pragma endregion

	return true;
}

#endif //WITH_AUTOMATION_TESTS
