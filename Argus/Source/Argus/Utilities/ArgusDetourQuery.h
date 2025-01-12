// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Detour/DetourNode.h"
#include "Detour/DetourNavMeshQuery.h"

class ArgusDtNodePool
{
public:
	ArgusDtNodePool(int maxNodes, int hashSize);
	~ArgusDtNodePool();
	inline void operator=(const ArgusDtNodePool&) {}
	void clear();
	dtNode* getNode(dtPolyRef id);
	dtNode* findNode(dtPolyRef id);

	inline unsigned int getNodeIdx(const dtNode* node) const
	{
		if (!node) return 0;
		return (unsigned int)(node - m_nodes) + 1;
	}

	inline dtNode* getNodeAtIdx(unsigned int idx)
	{
		if (!idx) return 0;
		return &m_nodes[idx - 1];
	}

	inline const dtNode* getNodeAtIdx(unsigned int idx) const
	{
		if (!idx) return 0;
		return &m_nodes[idx - 1];
	}

	inline int getMemUsed() const
	{
		return sizeof(*this) +
			sizeof(dtNode) * m_maxNodes +
			sizeof(dtNodeIndex) * m_maxNodes +
			sizeof(dtNodeIndex) * m_hashSize;
	}

	inline int getMaxNodes() const { return m_maxNodes; }
	//@UE BEGIN
	// If using a shared query instance it's possible that m_maxNodes is greater
	// than pool size requested by callee. There's no point in reallocating the
	// pool so we artificially limit the number of available nodes
	inline int getMaxRuntimeNodes() const { return m_maxRuntimeNodes; }
	//@UE END
	inline int getNodeCount() const { return m_nodeCount; }

	inline int getHashSize() const { return m_hashSize; }
	inline dtNodeIndex getFirst(int bucket) const { return m_first[bucket]; }
	inline dtNodeIndex getNext(int i) const { return m_next[i]; }

	//@UE BEGIN
	// overrides m_maxNodes for runtime purposes
	inline void setMaxRuntimeNodes(const int newMaxRuntimeNodes) { m_maxRuntimeNodes = newMaxRuntimeNodes; }
	//@UE END

private:

	dtNode* m_nodes;
	dtNodeIndex* m_first;
	dtNodeIndex* m_next;
	const int m_maxNodes;
	const int m_hashSize;
	//@UE BEGIN
	int m_maxRuntimeNodes;
	//@UE END
	int m_nodeCount;
};

/// <summary>
/// This entire class is intended as a workaround to the fact that the dtNavMeshQuery class has a lot of hardcoded restrictions.
/// In particular, we need to override how much memory is being allocated for initial avoidance obstacle wall calculation.
/// </summary>
class ArgusDetourQuery
{
public:
	static dtStatus FindWallsOverlappingShape(const dtNavMesh* detourMesh, const int maxNodes, dtPolyRef startRef,
		const dtReal* verts, const int nverts, const dtQueryFilter* filter,
		dtPolyRef* neiRefs, int* neiCount, const int maxNei,
		dtReal* resultWalls, dtPolyRef* resultRefs, int* resultCount, const int maxResult);
};