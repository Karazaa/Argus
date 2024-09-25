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

	bool SearchForEntityIdRecursive(ArgusKDTreeNode* node, uint16 entityId) const;

public:
	~ArgusKDTree();

	void InsertArgusEntityIntoKDTree(const ArgusEntity& entityToRepresent);
	void RebuildKDTreeForAllArgusEntities();

	bool DoesArgusEntityExistInKDTree(const ArgusEntity& entityToRepresent) const;
};