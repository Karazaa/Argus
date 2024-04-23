// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "../ArgusEntity.h"
#include "../Systems/TargetingSystems.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TargetingSystemsTargetNearestEntityMatchingFactionMaskTest, "Argus.ECS.Systems.TargetingSystems.TargetNearestEntityMatchingFactionMask", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TargetingSystemsTargetNearestEntityMatchingFactionMaskTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	const ArgusEntity sourceEntity = ArgusEntity::CreateEntity();
	const ArgusEntity closeEntity = ArgusEntity::CreateEntity();
	const ArgusEntity fartherEntity = ArgusEntity::CreateEntity();
	TransformComponent* sourceTransformComponent = sourceEntity.AddComponent<TransformComponent>();
	TransformComponent* closeTransformComponent = closeEntity.AddComponent<TransformComponent>();
	TransformComponent* fartherTransformComponent = fartherEntity.AddComponent<TransformComponent>();
	IdentityComponent* sourceIdentityComponent = sourceEntity.AddComponent<IdentityComponent>();
	IdentityComponent* closeIdentityComponent = closeEntity.AddComponent<IdentityComponent>();
	IdentityComponent* fartherIdentityComponent = fartherEntity.AddComponent<IdentityComponent>();
	const TargetingComponent* sourceTargetingComponent = sourceEntity.AddComponent<TargetingComponent>();

	if (!sourceTransformComponent || !closeTransformComponent || !fartherTransformComponent || 
		!sourceIdentityComponent || !closeIdentityComponent || !fartherIdentityComponent || !sourceTargetingComponent)
	{
		return false;
	}

	sourceTransformComponent->m_transform = FTransform(FVector(0.0f, 0.0f, 0.0f));
	closeTransformComponent->m_transform = FTransform(FVector(50.0f, 0.0f, 0.0f));
	fartherTransformComponent->m_transform = FTransform(FVector(100.0f, 0.0f, 0.0f));
	sourceIdentityComponent->m_faction = EFaction::FactionA;
	closeIdentityComponent->m_faction = EFaction::FactionB;
	fartherIdentityComponent->m_faction = EFaction::FactionC;
	sourceIdentityComponent->AddEnemyFaction(EFaction::FactionB);
	sourceIdentityComponent->AddEnemyFaction(EFaction::FactionC);

	TestTrue(TEXT("Testing that the default targeting ID is invalid."), sourceTargetingComponent->HasNoTarget());

	TargetingSystems::TargetNearestEntityMatchingFactionMask(sourceEntity, sourceTransformComponent, sourceIdentityComponent->m_enemies);

	TestEqual(TEXT("Testing that the source entity is targeting the correct entity after running TargetNearestEntityMatchingFactionMask."), sourceTargetingComponent->m_targetEntityId, closeEntity.GetId());

	fartherTransformComponent->m_transform.SetLocation(FVector(49.0f, 0.0f, 0.0f));
	TargetingSystems::TargetNearestEntityMatchingFactionMask(sourceEntity, sourceTransformComponent, sourceIdentityComponent->m_enemies);

	TestEqual(TEXT("Testing that the source entity is targeting the correct entity after changing locations and running TargetNearestEntityMatchingFactionMask."), sourceTargetingComponent->m_targetEntityId, fartherEntity.GetId());

	fartherTransformComponent->m_transform.SetLocation(FVector(50.0f, 0.0f, 0.0f));
	TargetingSystems::TargetNearestEntityMatchingFactionMask(sourceEntity, sourceTransformComponent, sourceIdentityComponent->m_enemies);

	TestEqual(TEXT("Testing that the source entity is targeting the correct entity using tiebreaking rules after running TargetNearestEntityMatchingFactionMask"), sourceTargetingComponent->m_targetEntityId, closeEntity.GetId());

	sourceIdentityComponent->AddAllyFaction(EFaction::FactionB);
	TargetingSystems::TargetNearestEntityMatchingFactionMask(sourceEntity, sourceTransformComponent, sourceIdentityComponent->m_enemies);

	TestEqual(TEXT("Testing that the source entity is targeting the correct entity after changing enemy faction mask"), sourceTargetingComponent->m_targetEntityId, fartherEntity.GetId());

	TargetingSystems::TargetNearestEntityMatchingFactionMask(sourceEntity, sourceTransformComponent, sourceIdentityComponent->m_allies);

	TestEqual(TEXT("Testing that the source entity is targeting the correct entity after changing ally faction mask"), sourceTargetingComponent->m_targetEntityId, closeEntity.GetId());

	return true;
}

#endif //WITH_AUTOMATION_TESTS