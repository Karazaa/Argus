// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "../ArgusEntity.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentHealthComponentPersistenceTest, "Argus.ECS.Component.HealthComponent.PersistenceTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentTargetingComponentHasNoTargetTest, "Argus.ECS.Component.TargetingComponent.HasNoTargetTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentTargetingComponentHasNoTargetTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	ArgusEntity entity1 = ArgusEntity::CreateEntity();
	ArgusEntity entity2 = ArgusEntity::CreateEntity();
	TargetingComponent* targetingComponent = entity1.AddComponent<TargetingComponent>();

	if (!targetingComponent)
	{
		return false;
	}

	TestTrue(TEXT("Creating a TargetingComponent and confirming that it does not have a target."), targetingComponent->HasNoTarget());
	
	targetingComponent->m_targetEntityId = entity2.GetId();

	TestEqual(TEXT("Creating a TargetingComponent, setting target to another entity, then checking the value is the right ID"), targetingComponent->m_targetEntityId, entity2.GetId());
	TestFalse(TEXT("Creating a TargetingComponent, setting target to another entity, and confirming that it does have a target."), targetingComponent->HasNoTarget());

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

	TestFalse(TEXT("Creating an IdentityComponent with faction A and confirming that it is not present in an empty faction mask."), identityComponent->IsInFactionMask(factionMask));

	factionMask = 0xFF;

	TestTrue(TEXT("Creating an IdentityComponent with faction A and confirming that it is present in a full faction mask."), identityComponent->IsInFactionMask(factionMask));

	return true;
}
#endif //WITH_AUTOMATION_TESTS