// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "../ArgusEntity.h"
#include "../Systems/TransformSystems.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TransformSystemsFindNearestEntityAndTargetTest, "Argus.ECS.Systems.TransformSystems.FindNearestEntityAndTargetTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TransformSystemsFindNearestEntityAndTargetTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	const ArgusEntity sourceEntity = ArgusEntity::CreateEntity();
	const ArgusEntity closeEntity = ArgusEntity::CreateEntity();
	const ArgusEntity fartherEntity = ArgusEntity::CreateEntity();
	TransformComponent* sourceTransformComponent = sourceEntity.AddComponent<TransformComponent>();
	TransformComponent* closeTransformComponent = closeEntity.AddComponent<TransformComponent>();
	TransformComponent* fartherTransformComponent = fartherEntity.AddComponent<TransformComponent>();
	const TargetingComponent* sourceTargetingComponent = sourceEntity.AddComponent<TargetingComponent>();

	if (!sourceTransformComponent || !closeTransformComponent || !fartherTransformComponent || !sourceTargetingComponent)
	{
		return false;
	}

	sourceTransformComponent->m_transform = FTransform(FVector(0.0f, 0.0f, 0.0f));
	closeTransformComponent->m_transform = FTransform(FVector(50.0f, 0.0f, 0.0f));
	fartherTransformComponent->m_transform = FTransform(FVector(100.0f, 0.0f, 0.0f));

	TestTrue(TEXT("Testing that the default targeting ID is invalid."), sourceTargetingComponent->HasNoTarget());

	TransformSystems::FindNearestEntityAndTarget(sourceEntity, sourceTransformComponent);

	TestEqual(TEXT("Testing that the source entity is targeting the correct entity after running FindNearestEntityAndTarget."), sourceTargetingComponent->m_targetEntityId, closeEntity.GetId());

	fartherTransformComponent->m_transform.SetLocation(FVector(49.0f, 0.0f, 0.0f));
	TransformSystems::FindNearestEntityAndTarget(sourceEntity, sourceTransformComponent);

	TestEqual(TEXT("Testing that the source entity is targeting the correct entity after changing locations and running FindNearestEntityAndTarget."), sourceTargetingComponent->m_targetEntityId, fartherEntity.GetId());

	fartherTransformComponent->m_transform.SetLocation(FVector(50.0f, 0.0f, 0.0f));
	TransformSystems::FindNearestEntityAndTarget(sourceEntity, sourceTransformComponent);

	TestEqual(TEXT("Testing that the source entity is targeting the correct entity using tiebreaking rules after running FindNearestEntityAndTarget"), sourceTargetingComponent->m_targetEntityId, closeEntity.GetId());

	return true;
}

#endif //WITH_AUTOMATION_TESTS