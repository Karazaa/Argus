// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "Systems/TargetingSystems.h"
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
	TargetingComponent* sourceTargetingComponent = sourceEntity.AddComponent<TargetingComponent>();

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

	TargetingSystems::TargetingSystemsComponentArgs components;
	components.m_targetingComponent = sourceTargetingComponent;
	components.m_transformComponent = sourceTransformComponent;

#pragma region Test no target by default
	TestFalse
	(
		FString::Printf(TEXT("[%s] Testing that a new %s would not have a target by default."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent)),
		sourceTargetingComponent->HasEntityTarget()
	);
#pragma endregion

	TargetingSystems::TargetNearestEntityMatchingFactionMask(sourceEntity.GetId(), sourceIdentityComponent->m_enemies, components);

#pragma region Test target closest enemy
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing that %s is targeting %s after running %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(sourceEntity),
			ARGUS_NAMEOF(closeEntity),
			ARGUS_NAMEOF(TargetingSystems::TargetNearestEntityMatchingFactionMask)
		),
		sourceTargetingComponent->m_targetEntityId,
		closeEntity.GetId()
	);
#pragma endregion

	fartherTransformComponent->m_transform.SetLocation(FVector(49.0f, 0.0f, 0.0f));
	TargetingSystems::TargetNearestEntityMatchingFactionMask(sourceEntity.GetId(), sourceIdentityComponent->m_enemies, components);

#pragma region Test target farther enemy after moving
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing that %s is targeting %s after changing locations and running %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(sourceTargetingComponent),
			ARGUS_NAMEOF(fartherEntity),
			ARGUS_NAMEOF(TargetingSystems::TargetNearestEntityMatchingFactionMask)
		),
		sourceTargetingComponent->m_targetEntityId,
		fartherEntity.GetId()
	);
#pragma endregion

	fartherTransformComponent->m_transform.SetLocation(FVector(50.0f, 0.0f, 0.0f));
	TargetingSystems::TargetNearestEntityMatchingFactionMask(sourceEntity.GetId(), sourceIdentityComponent->m_enemies, components);

#pragma region Test tiebreaking
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing that %s is targeting %s using tiebreaking rules after running %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(sourceTargetingComponent),
			ARGUS_NAMEOF(closeEntity),
			ARGUS_NAMEOF(TargetingSystems::TargetNearestEntityMatchingFactionMask)
		),
		sourceTargetingComponent->m_targetEntityId,
		closeEntity.GetId()
	);
#pragma endregion

	sourceIdentityComponent->AddAllyFaction(EFaction::FactionB);
	TargetingSystems::TargetNearestEntityMatchingFactionMask(sourceEntity.GetId(), sourceIdentityComponent->m_enemies, components);

#pragma region Test targeting nearest enemy after changing factions
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing that %s is targeting %s after changing enemy faction mask and running %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(sourceTargetingComponent),
			ARGUS_NAMEOF(fartherEntity),
			ARGUS_NAMEOF(TargetingSystems::TargetNearestEntityMatchingFactionMask)
		),
		sourceTargetingComponent->m_targetEntityId,
		fartherEntity.GetId()
	);
#pragma endregion

	TargetingSystems::TargetNearestEntityMatchingFactionMask(sourceEntity.GetId(), sourceIdentityComponent->m_allies, components);

#pragma region Test targeting after changing ally faction.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Testing that %s is targeting %s after changing ally faction mask and running %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(sourceTargetingComponent),
			ARGUS_NAMEOF(closeEntity),
			ARGUS_NAMEOF(TargetingSystems::TargetNearestEntityMatchingFactionMask)
		),
		sourceTargetingComponent->m_targetEntityId,
		closeEntity.GetId()
	);
#pragma endregion

	return true;
}

#endif //WITH_AUTOMATION_TESTS