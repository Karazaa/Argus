// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityGetIdTest, "Argus.Entity.GetId", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityGetIdTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity(10u);
	TestEqual(TEXT("Testing ArgusEntity creation and ID storage."), entity.GetId(), 10u);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityAutoIncrementIdTest, "Argus.Entity.AutoIncrementId", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityAutoIncrementIdTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity0 = ArgusEntity();
	ArgusEntity entity1 = ArgusEntity();
	TestEqual(TEXT("Testing ArgusEntity0 Id is 0."), entity0.GetId(), 0u);
	TestEqual(TEXT("Testing ArgusEntity1 Id is 1 after auto increment."), entity1.GetId(), 1u);

	return true;
}

#endif //WITH_AUTOMATION_TESTS
