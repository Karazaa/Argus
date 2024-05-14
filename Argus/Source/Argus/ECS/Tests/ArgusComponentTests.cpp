// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentHealthComponentPersistenceTest, "Argus.ECS.Component.HealthComponent.Persistence", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
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

	TestFalse(TEXT("Creating a TargetingComponent and confirming that it does not have a target."), targetingComponent->HasEntityTarget());
	
	targetingComponent->m_targetEntityId = entity2.GetId();

	TestEqual(TEXT("Creating a TargetingComponent, setting target to another entity, then checking the value is the right ID"), targetingComponent->m_targetEntityId, entity2.GetId());
	TestTrue(TEXT("Creating a TargetingComponent, setting target to another entity, and confirming that it does have a target."), targetingComponent->HasEntityTarget());

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

	TestTrue(TEXT("Creating two Identity Components, adding one as the ally of another, and then testing to make sure it is in the ally faction mask."), identityComponent2->IsInFactionMask(identityComponent1->m_allies));

	identityComponent1->AddEnemyFaction(EFaction::FactionB);
	identityComponent1->AddAllyFaction(EFaction::FactionB);

	TestTrue(TEXT("Creating two Identity Components, adding one as the enemy and then ally of another, and then testing to make sure it is in the ally faction mask."), identityComponent2->IsInFactionMask(identityComponent1->m_allies));
	TestFalse(TEXT("Creating two Identity Components, adding one as the enemy and then ally of another, and then testing to make sure it is not in the enemy faction mask."), identityComponent2->IsInFactionMask(identityComponent1->m_enemies));

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

	TestFalse(TEXT("Creating two Identity Components, adding one as the enemy of itself, and then testing to make sure it is not in the enemy mask."), identityComponent1->IsInFactionMask(identityComponent1->m_enemies));

	identityComponent1->AddEnemyFaction(EFaction::FactionB);

	TestTrue(TEXT("Creating two Identity Components, adding one as the enemy of another, and then testing to make sure it is in the enemy faction mask."), identityComponent2->IsInFactionMask(identityComponent1->m_enemies));

	identityComponent1->AddAllyFaction(EFaction::FactionB);
	identityComponent1->AddEnemyFaction(EFaction::FactionB);

	TestTrue(TEXT("Creating two Identity Components, adding one as the ally and then enemy of another, and then testing to make sure it is in the enemy faction mask."), identityComponent2->IsInFactionMask(identityComponent1->m_enemies));
	TestFalse(TEXT("Creating two Identity Components, adding one as the ally and then enemy of another, and then testing to make sure it is not in the ally faction mask."), identityComponent2->IsInFactionMask(identityComponent1->m_allies));
	return true;
}
#endif //WITH_AUTOMATION_TESTS