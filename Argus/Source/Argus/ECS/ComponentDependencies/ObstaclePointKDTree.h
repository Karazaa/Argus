// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusKDTree.h"
#include "ObstaclePoint.h"

struct ObstaclePointKDTreeNode : public IArgusKDTreeNode<bool>
{
	FVector2D m_location;
	int32 m_obstacleIndex = 0;
	int32 m_obstaclePointIndex = 0;
	ObstaclePointKDTreeNode* m_leftChild = nullptr;
	ObstaclePointKDTreeNode* m_rightChild = nullptr;

	ObstaclePointKDTreeNode() {};

	virtual void	Reset() override;
	virtual FVector GetLocation() const override { return FVector(m_location, 0.0f); }
	virtual void	Populate(const FVector& worldSpaceLocation) override;
	virtual bool	ShouldSkipNode() const override;
	virtual bool	ShouldSkipNode(bool valueToSkip) const override;
};

class ObstaclePointKDTree : public ArgusKDTree<ObstaclePointKDTreeNode, bool>
{

};