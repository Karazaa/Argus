// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusKDTree.h"
#include "ObstaclePoint.h"

struct ObstacleIndicies
{
	int32 m_obstacleIndex = -1;
	int32 m_obstaclePointIndex = -1;
};

struct ObstaclePointKDTreeNode : public IArgusKDTreeNode<bool>
{
	FVector2D m_location = FVector2D::ZeroVector;
	ObstacleIndicies m_indicies;
	ObstaclePointKDTreeNode* m_leftChild = nullptr;
	ObstaclePointKDTreeNode* m_rightChild = nullptr;

	ObstaclePointKDTreeNode() {};

	virtual void	Reset() override;
	virtual FVector GetLocation() const override { return FVector(m_location, 0.0f); }
	virtual void	Populate(const FVector& worldSpaceLocation) override;
	virtual bool	ShouldSkipNode() const override;
	virtual bool	ShouldSkipNode(bool valueToSkip) const override;
	virtual bool	PassesRangeCheck(const FVector& targetLocation, float rangeSquared) const override;
};

class ObstaclePointKDTree : public ArgusKDTree<ObstaclePointKDTreeNode, bool>
{
public:
	void InsertObstaclesIntoKDTree(const TArray<ObstaclePointArray>& obstacles);
	bool FindObstacleIndiciesWithinRangeOfLocation(TArray<ObstacleIndicies>& obstacleIndicies, const FVector& location, const float range) const;
};