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

	ObstaclePointKDTreeQueryRangeThresholds() {};
	ObstaclePointKDTreeQueryRangeThresholds(float avoidanceRangeThreshold) { m_avoidanceRangeThresholdSquared = avoidanceRangeThreshold * avoidanceRangeThreshold; }
};

class ObstaclePointKDTreeRangeOutput
{
public:
	bool AnyObstacleInidiciesInAvoidanceRange() const { return !m_obstacleIndiciesInAvoidanceRange.IsEmpty(); }
	bool AnyObstacleIndiciesInSightRange() const { return !m_obstacleIndiciesInSightRange.IsEmpty() || !m_obstacleIndiciesInAvoidanceRange.IsEmpty(); }
	int32 GetNumObstaclesInAvoidanceRange() const { return m_obstacleIndiciesInAvoidanceRange.Num(); }
	int32 GetNumObstacleInidciesInSightRange() const { return m_obstacleIndiciesInSightRange.Num() + m_obstacleIndiciesInAvoidanceRange.Num(); }
	void Add(const ObstaclePointKDTreeNode* nodeToAdd, const ObstaclePointKDTreeQueryRangeThresholds& thresholds, float distFromTargetSquared);
	void ResetAll();
	void PopulateArrayWithObstacleIndiciesInSightRange(TArray<ObstacleIndicies, ArgusContainerAllocator<0u> >& arrayToPopulate) const;

	const TArray<ObstacleIndicies, ArgusContainerAllocator<20u> >& GetObstacleIndiciesInSightRange() const { return m_obstacleIndiciesInSightRange; }
	const TArray<ObstacleIndicies, ArgusContainerAllocator<20u> >& GetObstacleIndiciesInAvoidanceRange() const { return m_obstacleIndiciesInAvoidanceRange; }


	template<typename Function>
	void IterateObstacleIndiciesInSightRange(Function&& function) const
	{
		for (int32 i = 0; i < m_obstacleIndiciesInAvoidanceRange.Num(); ++i)
		{
			function(m_obstacleIndiciesInAvoidanceRange[i]);
		}
		for (int32 i = 0; i < m_obstacleIndiciesInSightRange.Num(); ++i)
		{
			function(m_obstacleIndiciesInSightRange[i]);
		}
	}

private:
	TArray<ObstacleIndicies, ArgusContainerAllocator<20u> > m_obstacleIndiciesInSightRange;
	TArray<ObstacleIndicies, ArgusContainerAllocator<20u> > m_obstacleIndiciesInAvoidanceRange;
};

class ObstaclePointKDTree : public ArgusKDTree<	ObstaclePointKDTreeNode, ObstaclePointKDTreeRangeOutput, 
												ObstaclePointKDTreeQueryRangeThresholds, ArgusECSConstants::k_avoidanceObstaclePreAllocatedAmount>
{
public:
	void InsertObstaclesIntoKDTree(const ObstaclesContainer& obstacles);
	bool FindObstacleIndiciesWithinRangeOfLocation(TArray<ObstacleIndicies>& obstacleIndicies, const FVector& location, const float range);
	bool FindObstacleIndiciesWithinRangeOfLocation(ObstaclePointKDTreeRangeOutput& obstacleIndicies, ObstaclePointKDTreeQueryRangeThresholds& thresholds, const FVector& location, const float range) const;

private:
	ObstaclePointKDTreeRangeOutput m_queryScratchData;
};