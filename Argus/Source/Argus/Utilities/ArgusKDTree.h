// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusObjectPool.h"
#include "CoreMinimal.h"
#include <vector>

class ArgusEntity;

class ArgusKDTree
{
private:
	struct ArgusKDTreeNode
	{
		FVector		m_worldSpaceLocation = FVector::ZeroVector;
		uint16		m_entityId = ArgusECSConstants::k_maxEntities;
		ArgusKDTreeNode* m_leftChild = nullptr;
		ArgusKDTreeNode* m_rightChild = nullptr;

		ArgusKDTreeNode() {};
		void Populate(const FVector& worldSpaceLocation);
		void Populate(const ArgusEntity& entityToRepresent);
	};

public:
	~ArgusKDTree();

	FVector FlushAllNodes();
	void InsertArgusEntityIntoKDTree(const ArgusEntity& entityToRepresent);
	void RebuildKDTreeForAllArgusEntities();

	bool DoesArgusEntityExistInKDTree(const ArgusEntity& entityToRepresent) const;

	uint16 FindArgusEntityIdClosestToLocation(const FVector& location) const;
	uint16 FindArgusEntityIdClosestToLocation(const FVector& location, const ArgusEntity& entityToIgnore) const;
	uint16 FindOtherArgusEntityIdClosestArgusEntity(const ArgusEntity& entityToSearchAround) const;

	bool FindArgusEntityIdsWithinRangeOfLocation(std::vector<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range) const;
	bool FindArgusEntityIdsWithinRangeOfLocation(std::vector<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const ArgusEntity& entityToIgnore) const;


private:
	ArgusKDTreeNode* m_rootNode = nullptr;
	ArgusObjectPool<ArgusKDTreeNode> m_nodePool;

	void ClearNodeRecursive(ArgusKDTreeNode* node, FVector& currentAverageLocation, uint16& priorNodeCount);
	void InsertNodeIntoKDTreeRecursive(ArgusKDTreeNode* iterationNode, ArgusKDTreeNode* nodeToInsert, uint16 depth);

	bool SearchForEntityIdRecursive(const ArgusKDTreeNode* node, uint16 entityId) const;

	const ArgusKDTreeNode* FindArgusEntityIdClosestToLocationRecursive(const ArgusKDTreeNode* iterationNode, const FVector& targetLocation, uint16 entityIdToIgnore, uint16 depth) const;
	const ArgusKDTreeNode* ChooseNodeCloserToTarget(const ArgusKDTreeNode* node0, const ArgusKDTreeNode* node1, const FVector& targetLocation, uint16 entityIdToIgnore) const;

	void FindArgusEntityIdsWithinRangeOfLocationRecursive(std::vector<uint16>& outNearbyArgusEntityIds, const ArgusKDTreeNode* iterationNode, const FVector& targetLocation, const float rangeSquared, uint16 entityIdToIgnore, uint16 depth) const;
};