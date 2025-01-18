// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ObstaclePointKDTree.h"

void ObstaclePointKDTreeNode::Reset()
{
	m_location = FVector2D::ZeroVector;
	m_indicies.m_obstacleIndex = -1;
	m_indicies.m_obstaclePointIndex = -1;
	m_leftChild = nullptr;
	m_rightChild = nullptr;
}

void ObstaclePointKDTreeNode::Populate(const FVector& worldSpaceLocation)
{
	m_location = FVector2D(worldSpaceLocation);
}

bool ObstaclePointKDTreeNode::ShouldSkipNode() const
{
	return m_indicies.m_obstacleIndex < 0 || m_indicies.m_obstaclePointIndex < 0;
}

bool ObstaclePointKDTreeNode::ShouldSkipNode(bool valueToSkip) const
{
	return m_indicies.m_obstacleIndex < 0 || m_indicies.m_obstaclePointIndex < 0;
}

void ObstaclePointKDTree::InsertObstaclesIntoKDTree(const TArray<ObstaclePointArray>& obstacles)
{
	ARGUS_MEMORY_TRACE(ArgusKDTree);
	ARGUS_TRACE(ArgusKDTree::InsertObstaclesIntoKDTree);

	for (int32 i = 0; i < obstacles.Num(); ++i)
	{
		for (int32 j = 0; j < obstacles[i].Num(); ++j)
		{
			ObstaclePointKDTreeNode* nodeToInsert = m_nodePool.Take();
			nodeToInsert->m_location = obstacles[i][j].m_point;
			nodeToInsert->m_indicies.m_obstacleIndex = i;
			nodeToInsert->m_indicies.m_obstaclePointIndex = j;

			if (!m_rootNode)
			{
				m_rootNode = nodeToInsert;
			}
			else
			{
				InsertNodeIntoKDTreeRecursive(m_rootNode, nodeToInsert, 0u);
			}
		}
	}
}

bool ObstaclePointKDTree::FindObstacleIndiciesWithinRangeOfLocation(TArray<ObstacleIndicies>& obstacleIndicies, const FVector& location, const float range) const
{
	if (!m_rootNode)
	{
		return false;
	}

	if (range <= 0.0f)
	{
		ARGUS_LOG(ArgusUtilitiesLog, Error, TEXT("[%s] Searching range is less than or equal to 0."), ARGUS_FUNCNAME);
		return false;
	}

	TArray<const ObstaclePointKDTreeNode*> foundNodes;
	FindNodesWithinRangeOfLocationRecursive(foundNodes, m_rootNode, location, FMath::Square(range), true, 0u);
	obstacleIndicies.Reserve(foundNodes.Num());
	for (int32 i = 0; i < foundNodes.Num(); ++i)
	{
		if (!foundNodes[i])
		{
			continue;
		}

		obstacleIndicies.Add(foundNodes[i]->m_indicies);
	}

	return obstacleIndicies.Num() > 0;
}
