// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusKDTree.h"
#include "ArgusEntity.h"
#include "ArgusMacros.h"
#include "ComponentDefinitions/TransformComponent.h"

ArgusKDTree::ArgusKDTreeNode::ArgusKDTreeNode(const ArgusEntity& entityToRepresent)
{
	if (!entityToRepresent)
	{
		return;
	}

	const TransformComponent* transformComponent = entityToRepresent.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return;
	}

	m_worldSpaceLocation = transformComponent->m_transform.GetLocation();
	m_entityId = entityToRepresent.GetId();
}

void ArgusKDTree::ClearNodeRecursive(ArgusKDTreeNode* node)
{
	if (!node)
	{
		return;
	}

	if (node->m_leftChild)
	{
		ClearNodeRecursive(node->m_leftChild);
		delete(node->m_leftChild);
		node->m_leftChild = nullptr;
	}

	if (node->m_rightChild)
	{
		ClearNodeRecursive(node->m_rightChild);
		delete(node->m_rightChild);
		node->m_rightChild = nullptr;
	}
}

void ArgusKDTree::FlushAllNodes()
{
	ARGUS_TRACE(KDTree::FlushAllNodes)

	if (m_rootNode)
	{
		ClearNodeRecursive(m_rootNode);
		delete(m_rootNode);
		m_rootNode = nullptr;
	}
}

void ArgusKDTree::InsertNodeIntoKDTreeRecursive(ArgusKDTreeNode* iterationNode, ArgusKDTreeNode* nodeToInsert, uint16 depth)
{
	if (!iterationNode || !nodeToInsert)
	{
		return;
	}

	uint16 dimension = depth % 3u;

	float interationNodeValue = 0.0f;
	float noteToInsertValue = 0.0f;
	switch (dimension)
	{
		case 0:
			interationNodeValue = iterationNode->m_worldSpaceLocation.X;
			noteToInsertValue = nodeToInsert->m_worldSpaceLocation.X;
			break;
		case 1:
			interationNodeValue = iterationNode->m_worldSpaceLocation.Y;
			noteToInsertValue = nodeToInsert->m_worldSpaceLocation.Y;
			break;
		case 2:
			interationNodeValue = iterationNode->m_worldSpaceLocation.Z;
			noteToInsertValue = nodeToInsert->m_worldSpaceLocation.Z;
			break;
	}

	if (noteToInsertValue < interationNodeValue)
	{
		if (!iterationNode->m_leftChild)
		{
			iterationNode->m_leftChild = nodeToInsert;
			return;
		}

		InsertNodeIntoKDTreeRecursive(iterationNode->m_leftChild, nodeToInsert, depth + 1u);
	}
	else
	{
		if (!iterationNode->m_rightChild)
		{
			iterationNode->m_rightChild = nodeToInsert;
			return;
		}

		InsertNodeIntoKDTreeRecursive(iterationNode->m_leftChild, nodeToInsert, depth + 1u);
	}
}

bool ArgusKDTree::SearchForEntityIdRecursive(ArgusKDTreeNode* node, uint16 entityId) const
{
	if (!node)
	{
		return false;
	}

	if (node->m_entityId == entityId)
	{
		return true;
	}

	if (node->m_leftChild && SearchForEntityIdRecursive(node->m_leftChild, entityId))
	{
		return true;
	}

	if (node->m_rightChild && SearchForEntityIdRecursive(node->m_rightChild, entityId))
	{
		return true;
	}

	return false;
}

ArgusKDTree::~ArgusKDTree()
{
	FlushAllNodes();
}

void ArgusKDTree::InsertArgusEntityIntoKDTree(const ArgusEntity& entityToRepresent)
{
	ARGUS_TRACE(KDTree::InsertArgusEntityIntoKDTree)

	if (!entityToRepresent)
	{
		return;
	}

	const TransformComponent* transformComponent = entityToRepresent.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return;
	}

	ArgusKDTreeNode* nodeToInsert = new ArgusKDTreeNode(entityToRepresent);
	if (!m_rootNode)
	{
		m_rootNode = nodeToInsert;
		return;
	}

	InsertNodeIntoKDTreeRecursive(m_rootNode, nodeToInsert, 0u);
}

void ArgusKDTree::RebuildKDTreeForAllArgusEntities()
{
	ARGUS_TRACE(KDTree::RebuildKDTreeForAllArgusEntities)

	FlushAllNodes();
	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		InsertArgusEntityIntoKDTree(ArgusEntity::RetrieveEntity(i));
	}
}

bool ArgusKDTree::DoesArgusEntityExistInKDTree(const ArgusEntity& entityToRepresent) const
{
	if (!entityToRepresent || !m_rootNode)
	{
		return false;
	}

	return SearchForEntityIdRecursive(m_rootNode, entityToRepresent.GetId());
}