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
		FString::Printf(TEXT("[%s] Testing %s creation and ID storage."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)), 
		entity.GetId(), 
		expectedEntityID
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
		TEXT("Testing assigment of ArgusEntity."), 
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

	ArgusEntity entity0 = ArgusEntity::CreateEntity(20u);
	ArgusEntity entity1 = ArgusEntity::CreateEntity(30u);

	TestFalse(TEXT("Testing creation of two ArgusEntities with different IDs and the equals operator returning false."), entity0 == entity1);

	entity1 = entity0;

	TestTrue(TEXT("Testing creation of two ArgusEntities with the same ID and the equals operator returning true."), entity0 == entity1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityBoolOperatorTest, "Argus.ECS.Entity.BoolOperator", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityBoolOperatorTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	TestFalse(TEXT("Testing bool operator of empty entity returns false."), ArgusEntity::s_emptyEntity);

	ArgusEntity entity = ArgusEntity::CreateEntity(20u);

	TestTrue(TEXT("Testing bool operator of non-empty entity returns true."), entity);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityCreateInvalidEntityTest, "Argus.ECS.Entity.CreateInvalidEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityCreateInvalidEntityTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity(ArgusECSConstants::k_maxEntities);
	TestFalse(TEXT("Testing that creating an entity with invalid ID makes an invalid entity."), entity);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityFlushAllEntitiesTest, "Argus.ECS.Entity.FlushAllEntities", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityFlushAllEntitiesTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity(20u);
	TestTrue(TEXT("Testing if entity exists after creation."), entity);

	ArgusEntity::FlushAllEntities();

	TestFalse(TEXT("Testing that entity no longer exists after flushing all entities."), entity);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityCopyConstructorTest, "Argus.ECS.Entity.CopyConstructor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityCopyConstructorTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity(20u);
	ArgusEntity entity1 = ArgusEntity(entity0);

	TestEqual(TEXT("Testing if copy constructor successfully copies one ArgusEntity to another."), entity0, entity1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityAutoIncrementIdTest, "Argus.ECS.Entity.AutoIncrementId", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityAutoIncrementIdTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity();
	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	TestEqual(TEXT("Testing ArgusEntity0 Id is 0."), entity0.GetId(), 0u);
	TestEqual(TEXT("Testing ArgusEntity1 Id is 1 after auto increment."), entity1.GetId(), 1u);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityCopyEntityTest, "Argus.ECS.Entity.CopyEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityCopyEntityTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity(20u);
	ArgusEntity entity1 = entity0;
	ArgusEntity entity2 = ArgusEntity(entity0);
	TestEqual(TEXT("Testing ArgusEntity1 Id is 20 after assignment."), entity1.GetId(), 20u);
	TestEqual(TEXT("Testing ArgusEntity2 Id is 20 after copy constructor."), entity1.GetId(), 20u);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityDoesEntityExistTest, "Argus.ECS.Entity.DoesEntityExist", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityDoesEntityExistTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity(20u);
	TestTrue(TEXT("Testing if entity exists after creation."), ArgusEntity::DoesEntityExist(20u));
	TestFalse(TEXT("Testing that entity that doesn't exist isn't falsely listed as existing"), ArgusEntity::DoesEntityExist(30u));
	TestFalse(TEXT("Testing that invalid entity ID isn't falsely listed as existing"), ArgusEntity::DoesEntityExist(ArgusECSConstants::k_maxEntities));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityRetrieveEntityTest, "Argus.ECS.Entity.RetrieveEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityRetrieveEntityTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity::CreateEntity(20u);
	ArgusEntity existingEntity = ArgusEntity::RetrieveEntity(20u);
	ArgusEntity fakeEntity = ArgusEntity::RetrieveEntity(30u);
	ArgusEntity outOfRangeEntity = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_maxEntities);

	TestTrue(TEXT("Creating an entity, attempting to retrieve it, and testing that the returned entity is valid."), existingEntity);
	TestEqual(TEXT("Testing if retrieved entity has correct value."), existingEntity.GetId(), 20u);
	TestFalse(TEXT("Making sure retrieving a fake entity doesn't return a valid entity."), fakeEntity);
	TestFalse(TEXT("Making sure retrieving an out of range ID doesn't return a valid entity."), fakeEntity);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityAddGetHealthComponentTest, "Argus.ECS.Entity.AddGetHealthComponent", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityAddGetHealthComponentTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity();
	HealthComponent* healthComponent = entity.GetComponent<HealthComponent>();

	TestTrue(TEXT("Creating an entity, attempting to retrieve health component before adding it."), healthComponent == nullptr);

	healthComponent = entity.AddComponent<HealthComponent>();

	TestTrue(TEXT("Creating an entity, attempting to retrieve health component after adding it."), healthComponent != nullptr);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityGetOrAddComponentTest, "Argus.ECS.Entity.GetOrAddComponentTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityGetOrAddComponentTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity();
	HealthComponent* healthComponent = entity.GetOrAddComponent<HealthComponent>();

	TestFalse(TEXT("Creating an entity, calling get or add health component and making sure a component is added."), healthComponent == nullptr);

	healthComponent->m_health = 400u;
	healthComponent = entity.GetOrAddComponent<HealthComponent>();

	TestEqual(TEXT("Creating an entity, calling get or add health component twice and making sure the same component is returned."), healthComponent->m_health, 400u);

	return true;
}

#endif //WITH_AUTOMATION_TESTS
