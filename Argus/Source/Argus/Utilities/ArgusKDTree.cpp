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

void ArgusKDTree::InsertNodeIntoKDTreeRecursive(ArgusKDTreeNode* iterationNode, ArgusKDTreeNode* nodeToInsert, uint16 depth)
{
	if (!iterationNode || !nodeToInsert)
	{
		return;
	}

	uint16 dimension = depth % 3u;

	float iterationNodeValue = 0.0f;
	float noteToInsertValue = 0.0f;
	switch (dimension)
	{
		case 0:
			iterationNodeValue = iterationNode->m_worldSpaceLocation.X;
			noteToInsertValue = nodeToInsert->m_worldSpaceLocation.X;
			break;
		case 1:
			iterationNodeValue = iterationNode->m_worldSpaceLocation.Y;
			noteToInsertValue = nodeToInsert->m_worldSpaceLocation.Y;
			break;
		case 2:
			iterationNodeValue = iterationNode->m_worldSpaceLocation.Z;
			noteToInsertValue = nodeToInsert->m_worldSpaceLocation.Z;
			break;
	}

	if (noteToInsertValue < iterationNodeValue)
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

		InsertNodeIntoKDTreeRecursive(iterationNode->m_rightChild, nodeToInsert, depth + 1u);
	}
}

bool ArgusKDTree::SearchForEntityIdRecursive(const ArgusKDTreeNode* node, uint16 entityId) const
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

const ArgusKDTree::ArgusKDTreeNode* ArgusKDTree::FindArgusEntityIdClosestToLocationRecursive(const ArgusKDTreeNode* iterationNode, const FVector& targetLocation, uint16 entityIdToIgnore, uint16 depth) const
{
	if (!iterationNode)
	{
		return nullptr;
	}

	uint16 dimension = depth % 3u;

	ArgusKDTreeNode* firstBranch = nullptr;
	ArgusKDTreeNode* secondBranch = nullptr;

	float iterationNodeValue = 0.0f;
	float targetLocationValue = 0.0f;
	switch (dimension)
	{
		case 0:
			iterationNodeValue = iterationNode->m_worldSpaceLocation.X;
			targetLocationValue = targetLocation.X;
			break;
		case 1:
			iterationNodeValue = iterationNode->m_worldSpaceLocation.Y;
			targetLocationValue = targetLocation.Y;
			break;
		case 2:
			iterationNodeValue = iterationNode->m_worldSpaceLocation.Z;
			targetLocationValue = targetLocation.Z;
			break;
	}

	if (targetLocationValue < iterationNodeValue)
	{
		firstBranch = iterationNode->m_leftChild;
		secondBranch = iterationNode->m_rightChild;
	}
	else
	{
		firstBranch = iterationNode->m_rightChild;
		secondBranch = iterationNode->m_leftChild;
	}

	const ArgusKDTreeNode* potentialNearestNeighbor = FindArgusEntityIdClosestToLocationRecursive(firstBranch, targetLocation, entityIdToIgnore, depth + 1);
	potentialNearestNeighbor = ChooseNodeCloserToTarget(iterationNode, potentialNearestNeighbor, targetLocation, entityIdToIgnore);

	if (potentialNearestNeighbor)
	{
		const float potentialDistanceSquared = FVector::DistSquared(potentialNearestNeighbor->m_worldSpaceLocation, targetLocation);
		const float distanceAlongDimensionSquared = FMath::Square(iterationNodeValue - targetLocationValue);

		if (potentialDistanceSquared > distanceAlongDimensionSquared)
		{
			const ArgusKDTreeNode* cachedPotentialNearestNeighbor = potentialNearestNeighbor;
			potentialNearestNeighbor = FindArgusEntityIdClosestToLocationRecursive(secondBranch, targetLocation, entityIdToIgnore, depth + 1);

			if (iterationNode->m_entityId == entityIdToIgnore)
			{
				potentialNearestNeighbor = ChooseNodeCloserToTarget(cachedPotentialNearestNeighbor, potentialNearestNeighbor, targetLocation, entityIdToIgnore);
			}
			else
			{
				potentialNearestNeighbor = ChooseNodeCloserToTarget(iterationNode, potentialNearestNeighbor, targetLocation, entityIdToIgnore);
			}
		}
	}
	else
	{
		potentialNearestNeighbor = FindArgusEntityIdClosestToLocationRecursive(secondBranch, targetLocation, entityIdToIgnore, depth + 1);
		potentialNearestNeighbor = ChooseNodeCloserToTarget(iterationNode, potentialNearestNeighbor, targetLocation, entityIdToIgnore);
	}

	return potentialNearestNeighbor;
}

const ArgusKDTree::ArgusKDTreeNode* ArgusKDTree::ChooseNodeCloserToTarget(const ArgusKDTreeNode* node0, const ArgusKDTreeNode* node1, const FVector& targetLocation, uint16 entityIdToIgnore) const
{
	if (!node0 || node0->m_entityId == entityIdToIgnore)
	{
		return node1;
	}
	if (!node1 || node1->m_entityId == entityIdToIgnore)
	{
		return node0;
	}

	if (FVector::DistSquared(node0->m_worldSpaceLocation, targetLocation) > FVector::DistSquared(node1->m_worldSpaceLocation, targetLocation))
	{
		return node1;
	}
	else
	{
		return node0;
	}
}

ArgusKDTree::~ArgusKDTree()
{
	FlushAllNodes();
}

void ArgusKDTree::FlushAllNodes()
{
	ARGUS_TRACE(ArgusKDTree::FlushAllNodes)

		if (m_rootNode)
		{
			ClearNodeRecursive(m_rootNode);
			delete(m_rootNode);
			m_rootNode = nullptr;
		}
}

void ArgusKDTree::InsertArgusEntityIntoKDTree(const ArgusEntity& entityToRepresent)
{
	ARGUS_TRACE(ArgusKDTree::InsertArgusEntityIntoKDTree)

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
	ARGUS_TRACE(ArgusKDTree::RebuildKDTreeForAllArgusEntities)

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

uint16 ArgusKDTree::FindArgusEntityIdClosestToLocation(const FVector& location) const
{
	return FindArgusEntityIdClosestToLocation(location, ArgusEntity::s_emptyEntity);
}

uint16 ArgusKDTree::FindArgusEntityIdClosestToLocation(const FVector& location, const ArgusEntity& entityToIgnore) const
{
	if (!m_rootNode)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	const ArgusKDTreeNode* foundNode = FindArgusEntityIdClosestToLocationRecursive(m_rootNode, location, entityToIgnore.GetId(), 0u);

	if (!foundNode)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	return foundNode->m_entityId;
}

uint16 ArgusKDTree::FindOtherArgusEntityIdClosestArgusEntity(const ArgusEntity& entityToSearchAround) const
{
	const TransformComponent* transformComponent = entityToSearchAround.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	return FindArgusEntityIdClosestToLocation(transformComponent->m_transform.GetLocation(), entityToSearchAround);
}
