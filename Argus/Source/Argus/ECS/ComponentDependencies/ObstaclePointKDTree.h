// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "ArgusKDTree.h"
#include "ObstaclePoint.h"

struct ObstacleIndicies
{
	int32 m_obstacleIndex = -1;
	int32 m_obstaclePointIndex = -1;
};

struct ObstaclePointKDTreeNode
{
	FVector2D					m_location = FVector2D::ZeroVector;
	ObstacleIndicies			m_indicies;
	ObstaclePointKDTreeNode*	m_leftChild = nullptr;
	ObstaclePointKDTreeNode*	m_rightChild = nullptr;

	ObstaclePointKDTreeNode() {};

	void	Reset();
	FVector GetLocation() const { return FVector(m_location, 0.0f); }
	void	Populate(const FVector& worldSpaceLocation);
	bool	ShouldSkipNode() const;
	bool	ShouldSkipNode(TFunction<bool(const ObstaclePointKDTreeNode*)> queryFilter) const;
	bool	PassesRangeCheck(const FVector& targetLocation, float rangeSquared, float& nodeRangeSquared) const;
	float   GetValueForDimension(uint16 dimension) const;
	float	GetRadius() const { return 0.0f; }
};

struct ObstaclePointKDTreeQueryRangeThresholds
{
	float m_avoidanceRangeThresholdSquared = 0.0f;
};

class ObstaclePointKDTreeRangeOutput
{
public:
	void Add(const ObstaclePointKDTreeNode* nodeToAdd, const ObstaclePointKDTreeQueryRangeThresholds& thresholds, float distFromTargetSquared);
	void ResetAll();
	const TArray<ObstacleIndicies, ArgusContainerAllocator<20u> >& GetInRangeObstacleIndicies() const { return m_inRangeObstacleIndicies; }

private:
	TArray<ObstacleIndicies, ArgusContainerAllocator<20u> > m_inRangeObstacleIndicies;
};

class ObstaclePointKDTree : public ArgusKDTree<	ObstaclePointKDTreeNode, ObstaclePointKDTreeRangeOutput, 
												ObstaclePointKDTreeQueryRangeThresholds, ArgusECSConstants::k_avoidanceObstaclePreAllocatedAmount>
{
public:
	void InsertObstaclesIntoKDTree(const ObstaclesContainer& obstacles);
	bool FindObstacleIndiciesWithinRangeOfLocation(TArray<ObstacleIndicies>& obstacleIndicies, const FVector& location, const float range);
	bool FindObstacleIndiciesWithinRangeOfLocation(ObstaclePointKDTreeRangeOutput& obstacleIndicies, const FVector& location, const float range) const;

private:
	ObstaclePointKDTreeRangeOutput m_queryScratchData;
};