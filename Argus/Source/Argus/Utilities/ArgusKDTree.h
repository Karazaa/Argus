// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

class ArgusEntity;

class ArgusKDTree
{
private:
	struct ArgusKDTreeNode
	{
		FVector		m_worldSpaceLocation = FVector::ZeroVector;
		uint16		m_entityId = 0u;
		ArgusKDTreeNode* m_leftChild = nullptr;
		ArgusKDTreeNode* m_rightChild = nullptr;

		ArgusKDTreeNode(const ArgusEntity& entityToRepresent);
	};



	ArgusKDTreeNode* m_rootNode = nullptr;

	void ClearNodeRecursive(ArgusKDTreeNode* node);
	void FlushAllNodes();
	void InsertNodeIntoKDTreeRecursive(ArgusKDTreeNode* iterationNode, ArgusKDTreeNode* nodeToInsert, uint16 depth);

	bool SearchForEntityIdRecursive(const ArgusKDTreeNode* node, uint16 entityId) const;

	const ArgusKDTreeNode* FindArgusEntityIdClosestToLocationRecursive(const ArgusKDTreeNode* iterationNode, const FVector& targetLocation, uint16 depth) const;
	const ArgusKDTreeNode* ChooseNodeCloserToTarget(const ArgusKDTreeNode* node0, const ArgusKDTreeNode* node1, const FVector& targetLocation) const;

public:
	~ArgusKDTree();

	void InsertArgusEntityIntoKDTree(const ArgusEntity& entityToRepresent);
	void RebuildKDTreeForAllArgusEntities();

	bool DoesArgusEntityExistInKDTree(const ArgusEntity& entityToRepresent) const;

	uint16 FindArgusEntityIdClosestToLocation(const FVector& location) const;
};