// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "../ArgusEntity.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentHealthComponentPersistenceTest, "Argus.ECS.Component.HealthComponentPersistenceTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentHealthComponentPersistenceTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity = ArgusEntity::CreateEntity();
	HealthComponent* healthComponent = entity.AddComponent<HealthComponent>();

	if (!healthComponent)
	{
		return false;
	}

	healthComponent->m_health = 500u;
	healthComponent = entity.GetComponent<HealthComponent>();

	TestEqual(TEXT("Creating a HealthComponent, setting it to 500, then checking the value is 500 on retrieval."), healthComponent->m_health, 500u);
	*healthComponent = HealthComponent();
	TestEqual(TEXT("Creating a HealthComponent, setting it to 500, resetting it, then checking the value is 1000 after reset."), healthComponent->m_health, 1000u);

	return true;
}

#endif //WITH_AUTOMATION_TESTS