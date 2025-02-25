// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ObstaclePointKDTree.h"
#include "ArgusEntity.h"
#include "ArgusMath.h"

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

bool ObstaclePointKDTreeNode::PassesRangeCheck(const FVector& targetLocation, float rangeSquared) const
{
	if (ShouldSkipNode())
	{
		return false;
	}

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		return false;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		return false;
	}

	if (FVector::DistSquared(GetLocation(), targetLocation) < rangeSquared)
	{
		return true;
	}

	const ObstaclePoint& next = spatialPartitioningComponent->m_obstacles[m_indicies.m_obstacleIndex].GetNext(m_indicies.m_obstaclePointIndex);

	const FVector2D targetLocation2D = FVector2D(targetLocation);
	const FVector2D location2D = FVector2D(m_location);

	if ((next.m_point - location2D).Dot(targetLocation2D - location2D) < 0.0f)
	{
		return false;
	}

	if ((location2D - next.m_point).Dot(targetLocation2D - next.m_point) < 0.0f)
	{
		return false;
	}

	const float amountLeftOfLine = ArgusMath::AmountLeftOf(location2D, next.m_point, targetLocation2D);
	const float distanceSquaredLeftLine = ArgusMath::SafeDivide(FMath::Square(amountLeftOfLine), FVector2D::DistSquared(location2D, next.m_point));

	const bool result = (distanceSquaredLeftLine < rangeSquared);

	return result;
}

float ObstaclePointKDTreeNode::GetValueForDimension(uint16 dimension) const
{
	FVector location = GetLocation();
	switch (dimension)
	{
		case 0:
			return location.X;
		case 1:
			return location.Y;
		case 2:
			return location.Z;
		default:
			return 0.0f;
	}
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
