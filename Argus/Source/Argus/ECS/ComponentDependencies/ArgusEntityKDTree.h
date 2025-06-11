// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusKDTree.h"

class ArgusEntity;

struct ArgusEntityKDTreeNode
{
	FVector		m_worldSpaceLocation = FVector::ZeroVector;
	ArgusEntityKDTreeNode* m_leftChild = nullptr;
	ArgusEntityKDTreeNode* m_rightChild = nullptr;
	uint16		m_entityId = ArgusECSConstants::k_maxEntities;
	bool forceFullSearch = false;

	ArgusEntityKDTreeNode() {};

	void	Reset();
	FVector GetLocation() const { return m_worldSpaceLocation; }
	void	Populate(const FVector& worldSpaceLocation);
	bool	ShouldSkipNode() const;
	bool	ShouldSkipNode(TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilter) const;
	bool	PassesRangeCheck(const FVector& targetLocation, float rangeSquared, float& nodeRangeSquared) const;
	float   GetValueForDimension(uint16 dimension) const;

	void Populate(const ArgusEntity& entityToRepresent);
};

struct ArgusEntityKDTreeQueryRangeThresholds
{
	ArgusEntityKDTreeQueryRangeThresholds(float rangedRangeThreshold, float meleeRangeThreshold, float avoidanceRangeThreshold) : 
		m_rangedRangeThresholdSquared(rangedRangeThreshold * rangedRangeThreshold),
		m_meleeRangeThresholdSquared(meleeRangeThreshold * meleeRangeThreshold),
		m_avoidanceRangeThresholdSquared(avoidanceRangeThreshold * avoidanceRangeThreshold){}

	float m_rangedRangeThresholdSquared = 0.0f;
	float m_meleeRangeThresholdSquared = 0.0f;
	float m_avoidanceRangeThresholdSquared = 0.0f;
};

class ArgusEntityKDTreeRangeOutput
{
public:
	void Add(const ArgusEntityKDTreeNode* nodeToAdd, const ArgusEntityKDTreeQueryRangeThresholds& thresholds, float distFromTargetSquared);
	void ConsolidateInArray(TArray<uint16>& allEntityIds);
	bool FoundAny() const;

private:
	TArray<uint16> m_entityIdsWithinSightRange;
	TArray<uint16> m_entityIdsWithinRangedRange;
	TArray<uint16> m_entityIdsWithinMeleeRange;
	TArray<uint16> m_entityIdsWithinAvoidanceRange;
};

class ArgusEntityKDTree : public ArgusKDTree<ArgusEntityKDTreeNode, ArgusEntityKDTreeRangeOutput, ArgusEntityKDTreeQueryRangeThresholds>
{
public:
	static void ErrorOnInvalidArgusEntity(const WIDECHAR* functionName);

	void SeedTreeWithAverageEntityLocation();
	void InsertAllArgusEntitiesIntoKDTree();
	void RebuildKDTreeForAllArgusEntities();
	void InsertArgusEntityIntoKDTree(const ArgusEntity& entityToRepresent);

	uint16 FindArgusEntityIdClosestToLocation(const FVector& location) const;
	uint16 FindArgusEntityIdClosestToLocation(const FVector& location, const ArgusEntity& entityToIgnore) const;
	uint16 FindArgusEntityIdClosestToLocation(const FVector& location, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilter) const;
	uint16 FindOtherArgusEntityIdClosestToArgusEntity(const ArgusEntity& entityToSearchAround, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride = nullptr) const;

	bool FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range) const;
	bool FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const ArgusEntity& entityToIgnore) const;
	bool FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride) const;
	bool FindArgusEntityIdsWithinRangeOfLocation(ArgusEntityKDTreeRangeOutput& output, const ArgusEntityKDTreeQueryRangeThresholds& thresholds, const FVector& location, const float range, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride) const;
	bool FindOtherArgusEntityIdsWithinRangeOfArgusEntity(TArray<uint16>& outNearbyArgusEntityIds, const ArgusEntity& entityToSearchAround, const float range, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride = nullptr) const;

	bool FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector>& convexPolygonPoints) const;
	bool FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector2D>& convexPolygonPoints) const;

	bool DoesArgusEntityExistInKDTree(const ArgusEntity& entityToRepresent) const;

protected:
	bool SearchForEntityIdRecursive(const ArgusEntityKDTreeNode* node, uint16 entityId) const;
	void RebuildSubTreeForArgusEntitiesRecursive(ArgusEntityKDTreeNode*& node, bool forceReInsertChildren);
	void ClearNodeWithReInsert(ArgusEntityKDTreeNode*& node);
};