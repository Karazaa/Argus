// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusKDTree.h"

class ArgusEntity;

struct ArgusEntityKDTreeNode : public IArgusKDTreeNode<uint16>
{
	FVector		m_worldSpaceLocation = FVector::ZeroVector;
	uint16		m_entityId = ArgusECSConstants::k_maxEntities;
	ArgusEntityKDTreeNode* m_leftChild = nullptr;
	ArgusEntityKDTreeNode* m_rightChild = nullptr;
	bool forceFullSearch = false;

	ArgusEntityKDTreeNode() {};

	virtual void	Reset() override;
	virtual FVector GetLocation() const override { return m_worldSpaceLocation; }
	virtual void	Populate(const FVector& worldSpaceLocation) override;
	virtual bool	ShouldSkipNode() const override;
	virtual bool	ShouldSkipNode(TFunction<bool(uint16)> queryFilter) const override;
	virtual bool	PassesRangeCheck(const FVector& targetLocation, float rangeSquared) const override;
	virtual float   GetValueForDimension(uint16 dimension) const override;

	void Populate(const ArgusEntity& entityToRepresent);
};

class ArgusEntityKDTreeRangeOutput
{
public:
	ArgusEntityKDTreeRangeOutput(float rangedRange, float meleeRange);
	void Add(const ArgusEntityKDTreeNode* nodeToAdd);

private:
	TArray<uint16> m_entityIdsWithinSightRange;
	TArray<uint16> m_entityIdsWithinRangedRange;
	TArray<uint16> m_entityIdsWithinMeleeRange;
	float m_rangedRangeThresholdSquared = 0.0f;
	float m_meleeRangeThresholdSquared = 0.0f;
};

class ArgusEntityKDTree : public ArgusKDTree<ArgusEntityKDTreeNode, TArray<const ArgusEntityKDTreeNode*>, uint16>
{
public:
	static void ErrorOnInvalidArgusEntity(const WIDECHAR* functionName);
	static void ErrorOnInvalidTransformComponent(const WIDECHAR* functionName);

	void SeedTreeWithAverageEntityLocation();
	void InsertAllArgusEntitiesIntoKDTree();
	void RebuildKDTreeForAllArgusEntities();
	void InsertArgusEntityIntoKDTree(const ArgusEntity& entityToRepresent);

	uint16 FindArgusEntityIdClosestToLocation(const FVector& location) const;
	uint16 FindArgusEntityIdClosestToLocation(const FVector& location, const ArgusEntity& entityToIgnore) const;
	uint16 FindArgusEntityIdClosestToLocation(const FVector& location, const TFunction<bool(uint16)> queryFilter) const;
	uint16 FindOtherArgusEntityIdClosestToArgusEntity(const ArgusEntity& entityToSearchAround, const TFunction<bool(uint16)> queryFilterOverride = nullptr) const;

	bool FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range) const;
	bool FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const ArgusEntity& entityToIgnore) const;
	bool FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const TFunction<bool(uint16)> queryFilterOverride) const;
	bool FindOtherArgusEntityIdsWithinRangeOfArgusEntity(TArray<uint16>& outNearbyArgusEntityIds, const ArgusEntity& entityToSearchAround, const float range, const TFunction<bool(uint16)> queryFilterOverride = nullptr) const;

	bool FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector>& convexPolygonPoints) const;
	bool FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector2D>& convexPolygonPoints) const;

	bool DoesArgusEntityExistInKDTree(const ArgusEntity& entityToRepresent) const;

protected:
	bool SearchForEntityIdRecursive(const ArgusEntityKDTreeNode* node, uint16 entityId) const;
	void RebuildSubTreeForArgusEntitiesRecursive(ArgusEntityKDTreeNode*& node, bool forceReInsertChildren);
	void ClearNodeWithReInsert(ArgusEntityKDTreeNode*& node);
};