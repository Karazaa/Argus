// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "ArgusKDTree.h"

class ArgusEntity;

struct ArgusEntityKDTreeNode
{
	FVector					m_worldSpaceLocation = FVector::ZeroVector;
	ArgusEntityKDTreeNode*	m_leftChild = nullptr;
	ArgusEntityKDTreeNode*	m_rightChild = nullptr;
	float					m_radius = 0.0f;
	uint16					m_entityId = ArgusECSConstants::k_maxEntities;

	ArgusEntityKDTreeNode() {};

	void	Reset();
	FVector GetLocation() const { return m_worldSpaceLocation; }
	void	Populate(const FVector& worldSpaceLocation);
	bool	ShouldSkipNode() const;
	bool	ShouldSkipNode(TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilter) const;
	bool	PassesRangeCheck(const FVector& targetLocation, float rangeSquared, float& nodeRangeSquared) const;
	float   GetValueForDimension(uint16 dimension) const;
	float	GetRadius() const { return m_radius; }

	void Populate(const ArgusEntity& entityToRepresent);
};

struct ArgusEntityKDTreeQueryRangeThresholds
{
	ArgusEntityKDTreeQueryRangeThresholds(float rangedRangeThreshold, float meleeRangeThreshold, float avoidanceRangeThreshold, float flockingRangeThreshold, uint16 seenByEntityId) :
		m_rangedRangeThresholdSquared(rangedRangeThreshold * rangedRangeThreshold),
		m_meleeRangeThresholdSquared(meleeRangeThreshold * meleeRangeThreshold),
		m_avoidanceRangeThresholdSquared(avoidanceRangeThreshold * avoidanceRangeThreshold),
		m_flockingRangeThresholdSquared(flockingRangeThreshold * flockingRangeThreshold),
		m_seenByEntityId(seenByEntityId){}

	float m_rangedRangeThresholdSquared = 0.0f;
	float m_meleeRangeThresholdSquared = 0.0f;
	float m_avoidanceRangeThresholdSquared = 0.0f;
	float m_flockingRangeThresholdSquared = 0.0f;
	uint16 m_seenByEntityId = ArgusECSConstants::k_maxEntities;
};

class ArgusEntityKDTreeRangeOutput
{
public:
	void Add(const ArgusEntityKDTreeNode* nodeToAdd, const ArgusEntityKDTreeQueryRangeThresholds& thresholds, float distFromTargetSquared);
	void ConsolidateInArray(TArray<uint16>& allEntityIds);
	void ResetAll();
	bool FoundAny() const;
	const TArray<uint16, ArgusContainerAllocator<20> >& GetEntityIdsInSightRange() const { return m_entityIdsWithinSightRange; }
	const TArray<uint16, ArgusContainerAllocator<20> >& GetEntityIdsInRangedRange() const { return m_entityIdsWithinRangedRange; }
	const TArray<uint16, ArgusContainerAllocator<10> >& GetEntityIdsInMeleeRange() const { return m_entityIdsWithinMeleeRange; }
	const TArray<uint16, ArgusContainerAllocator<10> >& GetEntityIdsInAvoidanceRange() const { return m_entityIdsWithinAvoidanceRange; }
	const TArray<uint16, ArgusContainerAllocator<10> >& GetEntityIdsInFlockingRange() const { return m_entityIdsWithinFlockingRange; }

private:
	TArray<uint16, ArgusContainerAllocator<20> > m_entityIdsWithinSightRange;
	TArray<uint16, ArgusContainerAllocator<20> > m_entityIdsWithinRangedRange;
	TArray<uint16, ArgusContainerAllocator<10> > m_entityIdsWithinMeleeRange;
	TArray<uint16, ArgusContainerAllocator<10> > m_entityIdsWithinAvoidanceRange;
	TArray<uint16, ArgusContainerAllocator<10> > m_entityIdsWithinFlockingRange;
};

class ArgusEntityKDTree : public ArgusKDTree<	ArgusEntityKDTreeNode, ArgusEntityKDTreeRangeOutput, 
												ArgusEntityKDTreeQueryRangeThresholds, ArgusECSConstants::k_maxEntities>
{
public:
	static void ErrorOnInvalidArgusEntity(const WIDECHAR* functionName);

	void SeedTreeWithAverageEntityLocation(bool forFlyingEntities);
	void InsertAllArgusEntitiesIntoKDTree(bool forFlyingEntities);
	void RebuildKDTreeForAllArgusEntities();
	void InsertArgusEntityIntoKDTree(const ArgusEntity& entityToRepresent);

	uint16 FindArgusEntityIdClosestToLocation(const FVector& location) const;
	uint16 FindArgusEntityIdClosestToLocation(const FVector& location, const ArgusEntity& entityToIgnore) const;
	uint16 FindArgusEntityIdClosestToLocation(const FVector& location, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilter) const;
	uint16 FindOtherArgusEntityIdClosestToArgusEntity(const ArgusEntity& entityToSearchAround, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride = nullptr) const;

	bool FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range);
	bool FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const ArgusEntity& entityToIgnore);	
	bool FindArgusEntityIdsWithinRangeOfLocation(ArgusEntityKDTreeRangeOutput& output, const ArgusEntityKDTreeQueryRangeThresholds& thresholds, const FVector& location, const float range, const ArgusEntity& entityToIgnore) const;
	bool FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride);
	bool FindArgusEntityIdsWithinRangeOfLocation(ArgusEntityKDTreeRangeOutput& output, const ArgusEntityKDTreeQueryRangeThresholds& thresholds, const FVector& location, const float range, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride) const;
	bool FindOtherArgusEntityIdsWithinRangeOfArgusEntity(TArray<uint16>& outNearbyArgusEntityIds, const ArgusEntity& entityToSearchAround, const float range, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride = nullptr);
	bool FindOtherArgusEntityIdsWithinRangeOfArgusEntity(ArgusEntityKDTreeRangeOutput& output, const ArgusEntityKDTreeQueryRangeThresholds& thresholds, const ArgusEntity& entityToSearchAround, const float range, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride = nullptr) const;

	bool FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector>& convexPolygonPoints);
	bool FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector2D>& convexPolygonPoints);

	bool DoesArgusEntityExistInKDTree(const ArgusEntity& entityToRepresent) const;

protected:
	bool SearchForEntityIdRecursive(const ArgusEntityKDTreeNode* node, uint16 entityId) const;
	void RebuildSubTreeForArgusEntitiesRecursive(ArgusEntityKDTreeNode*& node, bool forceReInsertChildren);
	void ClearNodeWithReInsert(ArgusEntityKDTreeNode*& node);

private:
	ArgusEntityKDTreeRangeOutput m_queryScratchData;
};