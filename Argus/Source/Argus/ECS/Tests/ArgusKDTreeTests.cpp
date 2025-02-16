// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusTesting.h"
#include "ComponentDependencies/ArgusEntityKDTree.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

namespace ArgusKDTreeTestConstants
{
	const FVector location0 = FVector(50.0f, 50.0f, 0.0f);
	const FVector location1 = FVector(10.0f, 50.0f, 0.0f);
	const FVector location2 = FVector(100.0f, 50.0f, 0.0f);
	const FVector location3 = FVector(10.0f, 70.0f, 0.0f);
	const FVector location4 = FVector(100.0f, 50.0f, 10.0f);
	const FVector location5 = FVector(110.0f, 40.0f, 20.0f);
	const FVector location6 = FVector(200.0f, 200.0f, 200.0f);
	const FVector location7 = FVector(15.0f, 60.0f, 0.0f);

	const uint16 id0 = 100u;
	const uint16 id1 = 101u;
	const uint16 id2 = 102u;
	const uint16 id3 = 103u;
	const uint16 id4 = 104u;
}

struct CollectionOfArgusEntities
{
	const ArgusEntity entity0 = ArgusEntity::CreateEntity(ArgusKDTreeTestConstants::id0);
	const ArgusEntity entity1 = ArgusEntity::CreateEntity(ArgusKDTreeTestConstants::id1);
	const ArgusEntity entity2 = ArgusEntity::CreateEntity(ArgusKDTreeTestConstants::id2);
	const ArgusEntity entity3 = ArgusEntity::CreateEntity(ArgusKDTreeTestConstants::id3);
	const ArgusEntity entity4 = ArgusEntity::CreateEntity(ArgusKDTreeTestConstants::id4);
};

void PopulateKDTreeForTests(ArgusEntityKDTree& tree, CollectionOfArgusEntities& entities, bool insertIntoTree)
{
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
		transformComponent0->m_transform.SetLocation(ArgusKDTreeTestConstants::location0);
		transformComponent1->m_transform.SetLocation(ArgusKDTreeTestConstants::location1);
		transformComponent2->m_transform.SetLocation(ArgusKDTreeTestConstants::location2);
		transformComponent3->m_transform.SetLocation(ArgusKDTreeTestConstants::location3);
		transformComponent4->m_transform.SetLocation(ArgusKDTreeTestConstants::location4);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusKDTreeInsertEntitiesTest, "Argus.Utilities.ArgusKDTree.InsertEntitiesTest", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusKDTreeInsertEntitiesTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntityKDTree tree;
	CollectionOfArgusEntities entities;
	PopulateKDTreeForTests(tree, entities, true);

#pragma region Test that inserted entities are present in the KD tree.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, inserting %s into it, then checking that all of the inserted %s are present via %s"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntityKDTree::DoesArgusEntityExistInKDTree)
		),
		tree.DoesArgusEntityExistInKDTree(entities.entity0) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity1) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity2) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity3) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity4)
	);
#pragma endregion

	ArgusEntity entity5 = ArgusEntity::k_emptyEntity;

#pragma region Test that an invalid ArgusEntity reports the proper error.
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Passed in %s is invalid. It cannot be added to or retrieved from %s."),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree)
		),
		EAutomationExpectedErrorFlags::Contains,
		2
	);
#pragma endregion

	tree.DoesArgusEntityExistInKDTree(entity5);
	tree.InsertArgusEntityIntoKDTree(entity5);
	entity5 = ArgusEntity::CreateEntity(105u);
	TransformComponent* entity5TransformComponent = nullptr;

#pragma region Test that an invalid TransformComponent reports the proper error.
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Retrieved %s is invalid. Its owning %s cannot be added to or retrieved from %s."),
			ARGUS_NAMEOF(TransformComponent),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree)
		)
	);
#pragma endregion

	tree.InsertArgusEntityIntoKDTree(entity5);

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusKDTreeFlushAllNodesTest, "Argus.Utilities.ArgusKDTree.FlushAllNodesTest", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusKDTreeFlushAllNodesTest::RunTest(const FString& Parameters)
{
	const FVector expectedAverageLocation = FVector(54.0f, 54.0f, 2.0f);

	ArgusTesting::StartArgusTest();
	ArgusEntityKDTree tree;
	CollectionOfArgusEntities entities;
	PopulateKDTreeForTests(tree, entities, true);

	FVector averageLocation = tree.FlushAllNodes();

#pragma region Test that inserted entities are not present in the KD tree after being flushed.
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, inserting %s into it, then checking that none of the inserted %s are present after calling %s"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntityKDTree::FlushAllNodes)
		),
		tree.DoesArgusEntityExistInKDTree(entities.entity0) || 
		tree.DoesArgusEntityExistInKDTree(entities.entity1) || 
		tree.DoesArgusEntityExistInKDTree(entities.entity2) || 
		tree.DoesArgusEntityExistInKDTree(entities.entity3) || 
		tree.DoesArgusEntityExistInKDTree(entities.entity4)
	);
#pragma endregion

#pragma region Test that inserted entities are not present in the KD tree after being flushed.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, inserting %s into it, then checking that the calculated average location is accurate after calling %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FlushAllNodes)
		),
		averageLocation,
		expectedAverageLocation
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusKDTreeRebuildKDTreeForAllArgusEntitiesTest, "Argus.Utilities.ArgusKDTree.RebuildKDTreeForAllArgusEntitiesTest", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusKDTreeRebuildKDTreeForAllArgusEntitiesTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntityKDTree tree;
	CollectionOfArgusEntities entities;
	PopulateKDTreeForTests(tree, entities, false);

	tree.InsertAllArgusEntitiesIntoKDTree();

#pragma region Test that entities are present in the KD tree after full rebuild of tree.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then calling %s and checking that all of the inserted %s are present in the %s"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntityKDTree::InsertAllArgusEntitiesIntoKDTree),
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(ArgusEntityKDTree)
		),
		tree.DoesArgusEntityExistInKDTree(entities.entity0) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity1) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity2) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity3) && 
		tree.DoesArgusEntityExistInKDTree(entities.entity4)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusKDTreeFindOtherArgusEntityIdClosestArgusEntityTest, "Argus.Utilities.ArgusKDTree.FindOtherArgusEntityIdClosestArgusEntityTest", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusKDTreeFindOtherArgusEntityIdClosestArgusEntityTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntityKDTree tree;
	CollectionOfArgusEntities entities;
	PopulateKDTreeForTests(tree, entities, true);

	uint16 nearestEntityIdToCenter = tree.FindOtherArgusEntityIdClosestArgusEntity(entities.entity0);

#pragma region Test that we can find the nearest entity to the center entity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then checking which %s is closest to center %s via %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FindOtherArgusEntityIdClosestArgusEntity)
		),
		nearestEntityIdToCenter,
		ArgusKDTreeTestConstants::id1
	);
#pragma endregion

	uint16 nearestEntityIdToLeftCenter = tree.FindOtherArgusEntityIdClosestArgusEntity(entities.entity1);

#pragma region Test that we can find the nearest entity to the center-left entity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then checking which %s is closest to center-left %s via %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FindOtherArgusEntityIdClosestArgusEntity)
		),
		nearestEntityIdToLeftCenter,
		ArgusKDTreeTestConstants::id3
	);
#pragma endregion

	uint16 nearestEntityIdToRightCenter = tree.FindOtherArgusEntityIdClosestArgusEntity(entities.entity2);

#pragma region Test that we can find the nearest entity to the center-right entity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then checking which %s is closest to center-right %s via %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FindOtherArgusEntityIdClosestArgusEntity)
		),
		nearestEntityIdToRightCenter,
		ArgusKDTreeTestConstants::id4
	);
#pragma endregion

	ArgusEntity errorEntity = ArgusEntity::k_emptyEntity;

#pragma region Test that an invalid ArgusEntity reports the proper error.
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Passed in %s is invalid. It cannot be added to or retrieved from %s."),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree)
		)
	);
#pragma endregion

	tree.FindOtherArgusEntityIdClosestArgusEntity(errorEntity);
	errorEntity = ArgusEntity::CreateEntity(105u);
	TransformComponent* errorEntityTransformComponent = nullptr;

#pragma region Test that an TransformComponent reports the proper error.
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Retrieved %s is invalid. Its owning %s cannot be added to or retrieved from %s."),
			ARGUS_NAMEOF(TransformComponent),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree)
		)
	);
#pragma endregion

	tree.FindOtherArgusEntityIdClosestArgusEntity(errorEntity);

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusKDTreeFindArgusEntityIdClosestToLocationTest, "Argus.Utilities.ArgusKDTree.FindArgusEntityIdClosestToLocationTest", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusKDTreeFindArgusEntityIdClosestToLocationTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntityKDTree tree;
	CollectionOfArgusEntities entities;
	PopulateKDTreeForTests(tree, entities, true);

	uint16 nearestEntityIdToCenter = tree.FindArgusEntityIdClosestToLocation(ArgusKDTreeTestConstants::location0);

#pragma region Test that we can find the nearest entity to the center entity
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then checking which %s is closest to center %s via %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FindArgusEntityIdClosestToLocation)
		),
		nearestEntityIdToCenter,
		ArgusKDTreeTestConstants::id0
	);
#pragma endregion

	uint16 nearestEntityIdToZPoint = tree.FindArgusEntityIdClosestToLocation(ArgusKDTreeTestConstants::location5);

#pragma region Test that we can find the nearest entity to a non-entity z location
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then checking which %s is closest to a z point %s via %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FindArgusEntityIdClosestToLocation)
		),
		nearestEntityIdToZPoint,
		ArgusKDTreeTestConstants::id4
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusKDTreeFindArgusEntityIdsWithinRangeOfLocationTest, "Argus.Utilities.ArgusKDTree.FindArgusEntityIdsWithinRangeOfLocationTest", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusKDTreeFindArgusEntityIdsWithinRangeOfLocationTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();
	ArgusEntityKDTree tree;
	CollectionOfArgusEntities entities;
	PopulateKDTreeForTests(tree, entities, true);

	TArray<uint16> argusEntityIds;
	tree.FindArgusEntityIdsWithinRangeOfLocation(argusEntityIds, ArgusKDTreeTestConstants::location6, 1.0f);

#pragma region Test that we cannot find any entities within a small range of a far away location
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then checking which %s are in range of a far away location via %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation)
		),
		argusEntityIds.Num(),
		0
	);
#pragma endregion

	argusEntityIds.Empty();
	tree.FindArgusEntityIdsWithinRangeOfLocation(argusEntityIds, ArgusKDTreeTestConstants::location0, 1.0f);

#pragma region Test that we can find exactly one entity near the center
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then checking that exactly one %s is in range of a central location via %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation)
		),
		argusEntityIds.Num(),
		1
	);
#pragma endregion

	if (argusEntityIds.Num() < 1)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

#pragma region Test the id of the one entity near the center
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then checking that a specific %s is in range of a central location via %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation)
		),
		argusEntityIds[0],
		ArgusKDTreeTestConstants::id0
	);
#pragma endregion

	argusEntityIds.Empty();
	tree.FindArgusEntityIdsWithinRangeOfLocation(argusEntityIds, ArgusKDTreeTestConstants::location0, 200.0f);

#pragma region Test that we can find all entities within a large range
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then checking that all %s are within a large range of a central location via %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation)
		),
		argusEntityIds.Num(),
		5
	);
#pragma endregion

	argusEntityIds.Empty();
	tree.FindArgusEntityIdsWithinRangeOfLocation(argusEntityIds, ArgusKDTreeTestConstants::location7, 20.0f);

#pragma region Test that we can find exactly two entities near a left point
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then checking that exactly two %s are in range of a left location via %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation)
		),
		argusEntityIds.Num(),
		2
	);
#pragma endregion

	if (argusEntityIds.Num() < 2)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	const bool entity1Present = argusEntityIds[0] == ArgusKDTreeTestConstants::id1 || argusEntityIds[1] == ArgusKDTreeTestConstants::id1;
	const bool entity3Present = argusEntityIds[0] == ArgusKDTreeTestConstants::id3 || argusEntityIds[1] == ArgusKDTreeTestConstants::id3;

#pragma region Test the ids of the entities near the left
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, creating some %s, then checking that specific %s are in range of a left location via %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntityKDTree),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation)
		),
		entity1Present && entity3Present
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

#endif //WITH_AUTOMATION_TESTS