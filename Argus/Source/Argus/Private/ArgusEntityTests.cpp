// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityGetIdTest, "Argus.Entity.GetId", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityGetIdTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity(20u);
	TestEqual(TEXT("Testing ArgusEntity creation and ID storage."), entity.GetId(), 20u);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityAutoIncrementIdTest, "Argus.Entity.AutoIncrementId", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityAutoIncrementIdTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity();
	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	TestEqual(TEXT("Testing ArgusEntity0 Id is 0."), entity0.GetId(), 0u);
	TestEqual(TEXT("Testing ArgusEntity1 Id is 1 after auto increment."), entity1.GetId(), 1u);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityCopyEntityTest, "Argus.Entity.CopyEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityDoesEntityExistTest, "Argus.Entity.DoesEntityExist", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityDoesEntityExistTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity::CreateEntity(20u);
	TestTrue(TEXT("Testing if entity exists after creation."), ArgusEntity::DoesEntityExist(20u));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityRetrieveEntityTest, "Argus.Entity.RetrieveEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityRetrieveEntityTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity::CreateEntity(20u);
	std::optional<ArgusEntity> existingEntity = ArgusEntity::RetrieveEntity(20u);
	std::optional<ArgusEntity> fakeEntity = ArgusEntity::RetrieveEntity(30u);

	TestTrue(TEXT("Creating an entity, attempting to retrieve it, and testing that the returned optional has value."), existingEntity.has_value());
	TestEqual(TEXT("Testing if retrieved optional has correct value."), existingEntity->GetId(), 20u);
	TestFalse(TEXT("Making sure retrieving a fake entity doesn't return anything."), fakeEntity.has_value());

	return true;
}

#endif //WITH_AUTOMATION_TESTS
