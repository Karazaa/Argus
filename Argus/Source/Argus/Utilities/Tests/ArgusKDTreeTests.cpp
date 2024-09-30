// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

struct CollectionOfArgusEntities
{
	const ArgusEntity entity1 = ArgusEntity::CreateEntity();
	const ArgusEntity entity2 = ArgusEntity::CreateEntity();
	const ArgusEntity entity3 = ArgusEntity::CreateEntity();
	const ArgusEntity entity4 = ArgusEntity::CreateEntity();
	const ArgusEntity entity0 = ArgusEntity::CreateEntity();
};

void PopulateKDTreeForTests(ArgusKDTree& tree, CollectionOfArgusEntities& entities, bool insertIntoTree)
{
	const FVector location0 = FVector(50.0f, 50.0f, 0.0f);
	const FVector location1 = FVector(10.0f, 50.0f, 0.0f);
	const FVector location2 = FVector(100.0f, 50.0f, 0.0f);
	const FVector location3 = FVector(10.0f, 70.0f, 0.0f);
	const FVector location4 = FVector(100.0f, 50.0f, 10.0f);

	TransformComponent* transformComponent0 = entities.entity0.AddComponent<TransformComponent>();
	TransformComponent* transformComponent1 = entities.entity1.AddComponent<TransformComponent>();
	TransformComponent* transformComponent2 = entities.entity2.AddComponent<TransformComponent>();
	TransformComponent* transformComponent3 = entities.entity3.AddComponent<TransformComponent>();
	TransformComponent* transformComponent4 = entities.entity4.AddComponent<TransformComponent>();

	if (transformComponent0 &&
		transformComponent1 &&
		transformComponent2 &&
		transformComponent3 &&
		transformComponent4)
	{
		transformComponent0->m_transform.SetLocation(location0);
		transformComponent1->m_transform.SetLocation(location1);
		transformComponent2->m_transform.SetLocation(location2);
		transformComponent3->m_transform.SetLocation(location3);
		transformComponent4->m_transform.SetLocation(location4);
	}

	if (insertIntoTree)
	{
		tree.InsertArgusEntityIntoKDTree(entities.entity0);
		tree.InsertArgusEntityIntoKDTree(entities.entity1);
		tree.InsertArgusEntityIntoKDTree(entities.entity2);
		tree.InsertArgusEntityIntoKDTree(entities.entity3);
		tree.InsertArgusEntityIntoKDTree(entities.entity4);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusKDTreeInsertEntitiesTest, "Argus.Utilities.ArgusKDTree.InsertEntitiesTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusKDTreeInsertEntitiesTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();
	ArgusKDTree tree;
	CollectionOfArgusEntities entities;
	PopulateKDTreeForTests(tree, entities, true);

#pragma region Test that inserted entities are present in the KD tree.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, inserting %s into it, then checking that all of the inserted %s are present via %s"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusKDTree), 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusKDTree::DoesArgusEntityExistInKDTree)
		),
		tree.DoesArgusEntityExistInKDTree(entities.entity0) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity1) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity2) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity3) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity4)
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusKDTreeFlushAllNodesTest, "Argus.Utilities.ArgusKDTree.FlushAllNodesTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusKDTreeFlushAllNodesTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();
	ArgusKDTree tree;
	CollectionOfArgusEntities entities;
	PopulateKDTreeForTests(tree, entities, true);

	tree.FlushAllNodes();

#pragma region Test that inserted entities are not present in the KD tree after being flushed.
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, inserting %s into it, then checking that none of the inserted %s are present after calling %s"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusKDTree), 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusKDTree::FlushAllNodes)
		),
		tree.DoesArgusEntityExistInKDTree(entities.entity0) || 
		tree.DoesArgusEntityExistInKDTree(entities.entity1) || 
		tree.DoesArgusEntityExistInKDTree(entities.entity2) || 
		tree.DoesArgusEntityExistInKDTree(entities.entity3) || 
		tree.DoesArgusEntityExistInKDTree(entities.entity4)
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusKDTreeRebuildKDTreeForAllArgusEntitiesTest, "Argus.Utilities.ArgusKDTree.RebuildKDTreeForAllArgusEntitiesTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusKDTreeRebuildKDTreeForAllArgusEntitiesTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();
	ArgusKDTree tree;
	CollectionOfArgusEntities entities;
	PopulateKDTreeForTests(tree, entities, false);

	tree.RebuildKDTreeForAllArgusEntities();

#pragma region Test that entities are present in the KD tree after full rebuild of tree.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then calling %s and checking that all of the inserted %s are present in the %s"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusKDTree), 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusKDTree::RebuildKDTreeForAllArgusEntities), 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusKDTree)
		),
		tree.DoesArgusEntityExistInKDTree(entities.entity0) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity1) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity2) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity3) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity4)
	);
#pragma endregion

	return true;
}

#endif //WITH_AUTOMATION_TESTS