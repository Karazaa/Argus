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
	ArgusEntityKDTreeQueryRangeThresholds(float rangedRangeThreshold, float meleeRangeThreshold, float avoidanceRangeThreshold, uint16 seenByEntityId) :
		m_rangedRangeThresholdSquared(rangedRangeThreshold * rangedRangeThreshold),
		m_meleeRangeThresholdSquared(meleeRangeThreshold * meleeRangeThreshold),
		m_avoidanceRangeThresholdSquared(avoidanceRangeThreshold * avoidanceRangeThreshold),
		m_seenByEntityId(seenByEntityId){}

	float m_rangedRangeThresholdSquared = 0.0f;
	float m_meleeRangeThresholdSquared = 0.0f;
	float m_avoidanceRangeThresholdSquared = 0.0f;
	uint16 m_seenByEntityId = ArgusECSConstants::k_maxEntities;
};

class ArgusEntityKDTreeRangeOutput
{
public:
	void Add(const ArgusEntityKDTreeNode* nodeToAdd, const ArgusEntityKDTreeQueryRangeThresholds& thresholds, float distFromTargetSquared);
	void ConsolidateInArray(TArray<uint16>& allEntityIds);
	void ResetAll();
	bool FoundAny() const;
	const TArray<uint16, ArgusContainerAllocator<20u> >& GetEntityIdsInSightRange() const { return m_entityIdsWithinSightRange; }
	const TArray<uint16, ArgusContainerAllocator<20u> >& GetEntityIdsInRangedRange() const { return m_entityIdsWithinRangedRange; }
	const TArray<uint16, ArgusContainerAllocator<10u> >& GetEntityIdsInMeleeRange() const { return m_entityIdsWithinMeleeRange; }
	const TArray<uint16, ArgusContainerAllocator<10u> >& GetEntityIdsInAvoidanceRange() const { return m_entityIdsWithinAvoidanceRange; }

private:
	TArray<uint16, ArgusContainerAllocator<20u> > m_entityIdsWithinSightRange;
	TArray<uint16, ArgusContainerAllocator<20u> > m_entityIdsWithinRangedRange;
	TArray<uint16, ArgusContainerAllocator<10u> > m_entityIdsWithinMeleeRange;
	TArray<uint16, ArgusContainerAllocator<10u> > m_entityIdsWithinAvoidanceRange;
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
	bool RemoveArgusEntityFromKDTree(const ArgusEntity& entityToRemove);

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

	void RequestInsertArgusEntityIntoKDTree(const ArgusEntity& entityToInsert);
	void RequestRemoveArgusEntityIntoKDTree(const ArgusEntity& entityToRemove);
	void ProcessDeferredStateChanges();

protected:
	bool SearchForEntityIdRecursive(const ArgusEntityKDTreeNode* node, uint16 entityId) const;
	bool SearchForEntityIdRecursive(ArgusEntityKDTreeNode* node, uint16 entityId, ArgusEntityKDTreeNode*& ouputNode, ArgusEntityKDTreeNode*& ouputParentNode);
	void RebuildSubTreeForArgusEntitiesRecursive(ArgusEntityKDTreeNode*& node, bool forceReInsertChildren);
	void ClearNodeWithReInsert(ArgusEntityKDTreeNode*& node);

private:
	ArgusEntityKDTreeRangeOutput m_queryScratchData;
	TArray<uint16> m_entityIdsToInsert;
	TArray<uint16> m_entityIdsToRemove;
};