// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ObstaclePointKDTree.h"

void ObstaclePointKDTreeNode::Reset()
{
	m_location = FVector2D::ZeroVector;
	m_obstacleIndex = 0;
	m_obstaclePointIndex = 0;
	m_leftChild = nullptr;
	m_rightChild = nullptr;
}

void ObstaclePointKDTreeNode::Populate(const FVector& worldSpaceLocation)
{
	m_location = FVector2D(worldSpaceLocation);
}

bool ObstaclePointKDTreeNode::ShouldSkipNode() const
{
	return false;
}

bool ObstaclePointKDTreeNode::ShouldSkipNode(bool valueToSkip) const
{
	return false;
}
