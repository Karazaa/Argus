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

class ArgusEntityKDTree : public ArgusKDTree<ArgusEntityKDTreeNode, uint16>
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
	uint16 FindOtherArgusEntityIdClosestArgusEntity(const ArgusEntity& entityToSearchAround) const;

	bool FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range) const;
	bool FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const ArgusEntity& entityToIgnore) const;
	bool FindOtherArgusEntityIdsWithinRangeOfArgusEntity(TArray<uint16>& outNearbyArgusEntityIds, const ArgusEntity& entityToSearchAround, const float range) const;

	bool FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector>& convexPolygonPoints) const;
	bool FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector2D>& convexPolygonPoints) const;

	bool DoesArgusEntityExistInKDTree(const ArgusEntity& entityToRepresent) const;

protected:
	bool SearchForEntityIdRecursive(const ArgusEntityKDTreeNode* node, uint16 entityId) const;
	void RebuildSubTreeForArgusEntitiesRecursive(ArgusEntityKDTreeNode*& node, bool forceReInsertChildren);
	void ClearNodeWithReInsert(ArgusEntityKDTreeNode*& node);
};