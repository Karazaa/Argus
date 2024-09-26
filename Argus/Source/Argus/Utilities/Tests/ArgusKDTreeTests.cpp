// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusKDTreeInsertEntitiesTest, "Argus.Utilities.ArgusKDTree.InsertEntitiesTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusKDTreeInsertEntitiesTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	const ArgusEntity entity1 = ArgusEntity::CreateEntity();
	const ArgusEntity entity2 = ArgusEntity::CreateEntity();
	const ArgusEntity entity3 = ArgusEntity::CreateEntity();
	const ArgusEntity entity4 = ArgusEntity::CreateEntity();
	const ArgusEntity entity0 = ArgusEntity::CreateEntity();
	const TransformComponent* transformComponent0 = entity0.AddComponent<TransformComponent>();
	const TransformComponent* transformComponent1 = entity1.AddComponent<TransformComponent>();
	const TransformComponent* transformComponent2 = entity2.AddComponent<TransformComponent>();
	const TransformComponent* transformComponent3 = entity3.AddComponent<TransformComponent>();
	const TransformComponent* transformComponent4 = entity4.AddComponent<TransformComponent>();

	ArgusKDTree tree;
	tree.InsertArgusEntityIntoKDTree(entity0);
	tree.InsertArgusEntityIntoKDTree(entity1);
	tree.InsertArgusEntityIntoKDTree(entity2);
	tree.InsertArgusEntityIntoKDTree(entity3);
	tree.InsertArgusEntityIntoKDTree(entity4);

#pragma region Test that inserted entities are present in the KD tree.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Creating a %s, inserting %s into it, then checking that all of the inserted %s are present via %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusKDTree), ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusKDTree::DoesArgusEntityExistInKDTree)),
		tree.DoesArgusEntityExistInKDTree(entity0) && tree.DoesArgusEntityExistInKDTree(entity1) && tree.DoesArgusEntityExistInKDTree(entity2) && tree.DoesArgusEntityExistInKDTree(entity3) && tree.DoesArgusEntityExistInKDTree(entity4)
	);
#pragma endregion

	return true;
}

#endif //WITH_AUTOMATION_TESTS