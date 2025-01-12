// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusDetourQuery.h"
#include "ArgusMacros.h"
#include "Detour/DetourNode.h"
#include "Detour/DetourAssert.h"

inline unsigned int dtHashRef(dtPolyRef a)
{
	a += ~(a << 31);
	a ^= (a >> 20);
	a += (a << 6);
	a ^= (a >> 12);
	a += ~(a << 22);
	a ^= (a >> 32);
	return (unsigned int)a;
}

static bool passLinkFilter(const dtMeshTile* tile, const int polyIdx, const dtQuerySpecialLinkFilter* filter)
{
	const int linkIdx = polyIdx - tile->header->offMeshBase;

	return !(filter && polyIdx >= tile->header->offMeshBase
		&& linkIdx < tile->header->offMeshConCount
		&& tile->offMeshCons[linkIdx].userId != 0
		&& filter->isLinkAllowed(tile->offMeshCons[linkIdx].userId) == false);
}

static bool passLinkFilterByRef(const dtNavMesh* nav, const dtMeshTile* tile, const dtPolyRef ref)
{
	return passLinkFilter(tile, nav->decodePolyIdPoly(ref), nullptr);
}

static dtStatus getPortalPoints(const dtNavMesh* nav, dtPolyRef from, const dtPoly* fromPoly, const dtMeshTile* fromTile,
	dtPolyRef to, const dtPoly* toPoly, const dtMeshTile* toTile,
	dtReal* left, dtReal* right)
{
	// Find the link that points to the 'to' polygon.
	const dtLink* link = 0;
	unsigned int linkIndex = fromPoly->firstLink;
	while (linkIndex != DT_NULL_LINK)
	{
		const dtLink& testLink = nav->getLink(fromTile, linkIndex);
		linkIndex = testLink.next;

		if (testLink.ref == to)
		{
			link = &testLink;
			break;
		}
	}
	if (!link)
		return DT_FAILURE | DT_INVALID_PARAM;

	// Handle off-mesh connections.
	if (fromPoly->getType() == DT_POLYTYPE_OFFMESH_POINT)
	{
		// Find link that points to first vertex.
		unsigned int i = fromPoly->firstLink;
		while (i != DT_NULL_LINK)
		{
			const dtLink& testLink = nav->getLink(fromTile, i);
			i = testLink.next;

			if (testLink.ref == to)
			{
				const int v = testLink.edge;
				dtVcopy(left, &fromTile->verts[fromPoly->verts[v] * 3]);
				dtVcopy(right, &fromTile->verts[fromPoly->verts[v] * 3]);
				return DT_SUCCESS;
			}
		}
		return DT_FAILURE | DT_INVALID_PARAM;
	}
	//@UE BEGIN
#if WITH_NAVMESH_SEGMENT_LINKS
	else if (fromPoly->getType() == DT_POLYTYPE_OFFMESH_SEGMENT)
	{
		// Find link that points to first vertex.
		unsigned int i = fromPoly->firstLink;
		while (i != DT_NULL_LINK)
		{
			const dtLink& testLink = nav->getLink(fromTile, i);
			i = testLink.next;

			if (testLink.ref == to)
			{
				const int v = testLink.edge * 2;
				dtVcopy(left, &fromTile->verts[fromPoly->verts[v + 0] * 3]);
				dtVcopy(right, &fromTile->verts[fromPoly->verts[v + 1] * 3]);
				return DT_SUCCESS;
			}
		}

		return DT_FAILURE | DT_INVALID_PARAM;
	}
	//@UE END
#endif // WITH_NAVMESH_SEGMENT_LINKS

	if (toPoly->getType() == DT_POLYTYPE_OFFMESH_POINT)
	{
		unsigned int i = toPoly->firstLink;
		while (i != DT_NULL_LINK)
		{
			const dtLink& testLink = nav->getLink(toTile, i);
			i = testLink.next;

			if (testLink.ref == from)
			{
				const int v = testLink.edge;
				dtVcopy(left, &toTile->verts[toPoly->verts[v] * 3]);
				dtVcopy(right, &toTile->verts[toPoly->verts[v] * 3]);
				return DT_SUCCESS;
			}
		}
		return DT_FAILURE | DT_INVALID_PARAM;
	}
	//@UE BEGIN
#if WITH_NAVMESH_SEGMENT_LINKS
	else if (toPoly->getType() == DT_POLYTYPE_OFFMESH_SEGMENT)
	{
		unsigned int i = toPoly->firstLink;
		while (i != DT_NULL_LINK)
		{
			const dtLink& testLink = nav->getLink(toTile, i);
			i = testLink.next;

			if (testLink.ref == from)
			{
				const int v = testLink.edge * 2;
				dtVcopy(left, &toTile->verts[toPoly->verts[v + 0] * 3]);
				dtVcopy(right, &toTile->verts[toPoly->verts[v + 1] * 3]);
				return DT_SUCCESS;
			}
		}
		return DT_FAILURE | DT_INVALID_PARAM;
	}
#endif // WITH_NAVMESH_SEGMENT_LINKS
	//@UE END

	// Find portal vertices.
	const int v0 = fromPoly->verts[link->edge];
	const int v1 = fromPoly->verts[(link->edge + 1) % (int)fromPoly->vertCount];
	dtVcopy(left, &fromTile->verts[v0 * 3]);
	dtVcopy(right, &fromTile->verts[v1 * 3]);

	// If the link is at tile boundary, dtClamp the vertices to
	// the link width.
//@UE BEGIN
	if ((link->side & DT_CONNECTION_INTERNAL) == 0)
		//@UE END
	{
		// Unpack portal limits.
		if (link->bmin != 0 || link->bmax != 255)
		{
			const dtReal s = dtReal(1.) / 255.0f;
			const dtReal tmin = link->bmin * s;
			const dtReal tmax = link->bmax * s;
			dtVlerp(left, &fromTile->verts[v0 * 3], &fromTile->verts[v1 * 3], tmin);
			dtVlerp(right, &fromTile->verts[v0 * 3], &fromTile->verts[v1 * 3], tmax);
		}
	}

	return DT_SUCCESS;
}

static void storeWallSegment(const dtNavMesh* nav, const dtMeshTile* tile, const dtPoly* poly, int edge, dtPolyRef ref0, dtPolyRef ref1,
	dtReal* resultWalls, dtPolyRef* resultRefs, int* resultCount, const int maxResult)
{
	if (*resultCount >= maxResult)
	{
		return;
	}

	// Skip for polys that don't have vertices (e.g. nav links)
	if (poly->vertCount == 0)
	{
		return;
	}

	const dtReal* va = &tile->verts[poly->verts[edge] * 3];
	const dtReal* vb = &tile->verts[poly->verts[(edge + 1) % poly->vertCount] * 3];

	const int32 wall0Offset = (*resultCount * 6) + 0;
	const int32 wall1Offset = (*resultCount * 6) + 3;

	dtVcopy(&resultWalls[wall0Offset], va);
	dtVcopy(&resultWalls[wall1Offset], vb);
	resultRefs[*resultCount * 2 + 0] = ref0;
	resultRefs[*resultCount * 2 + 1] = ref1;

	*resultCount += 1;

	// If neighbor is valid, find the segment that both polygons share by projecting the neighbor segment to the current segment va-vb.
	if (ref1)
	{
		const dtMeshTile* neiTile = 0;
		const dtPoly* neiPoly = 0;
		nav->getTileAndPolyByRef(ref1, &neiTile, &neiPoly);

		// Find edge of the neighbor polygon.
		int neiEdge = -1;
		unsigned int neiLinkId = neiPoly ? neiPoly->firstLink : DT_NULL_LINK;
		while (neiLinkId != DT_NULL_LINK)
		{
			const dtLink& link = nav->getLink(neiTile, neiLinkId);
			neiLinkId = link.next;
			if (link.ref == ref0)
			{
				neiEdge = link.edge;
				break;
			}
		}

		if (neiEdge != -1)
		{
			const dtReal* va2 = &neiTile->verts[neiPoly->verts[neiEdge] * 3];
			const dtReal* vb2 = &neiTile->verts[neiPoly->verts[(neiEdge + 1) % neiPoly->vertCount] * 3];

			// Project and clip segment va2-vb2 on va-vb
			dtReal seg[3], diffA[3], diffB[3], clippedA[3], clippedB[3];
			dtVsub(seg, vb, va);

			dtVsub(diffA, va2, va);
			dtVsub(diffB, vb2, va);
			const dtReal da = dtVdot(diffA, seg);
			const dtReal db = dtVdot(diffB, seg);
			const dtReal ds = dtVdot(seg, seg);
			const dtReal dmin = dtMin(da, db);
			const dtReal dmax = dtMax(da, db);

			if (dmin <= 0)
			{
				dtVcopy(clippedA, va);
			}
			else if (dmin >= ds)
			{
				dtVcopy(clippedA, vb);
			}
			else
			{
				dtVmad(clippedA, va, seg, dmin / ds);
			}

			if (dmax <= 0)
			{
				dtVcopy(clippedB, va);
			}
			else if (dmax >= ds)
			{
				dtVcopy(clippedB, vb);
			}
			else
			{
				dtVmad(clippedB, va, seg, dmax / ds);
			}

			// Store projected segment (intersection of both edges)
			dtVcopy(&resultWalls[wall0Offset], clippedA);
			dtVcopy(&resultWalls[wall1Offset], clippedB);
		}
	}
}

static bool ArgusDtIntersectSegmentPoly2D(const dtReal* p0, const dtReal* p1,
	const dtReal* verts, int nverts,
	dtReal& tmin, dtReal& tmax,
	int& segMin, int& segMax)
{
	static const dtReal EPS = 0.00000001f;

	tmin = 0;
	tmax = 1;
	segMin = -1;
	segMax = -1;

	dtReal dir[3];
	dtVsub(dir, p1, p0);

	for (int i = 0, j = nverts - 1; i < nverts; j = i++)
	{
		dtReal edge[3], diff[3];
		dtVsub(edge, &verts[i * 3], &verts[j * 3]);
		dtVsub(diff, p0, &verts[j * 3]);
		const dtReal n = dtVperp2D(edge, diff);
		const dtReal d = dtVperp2D(dir, edge);
		if (dtAbs(d) < EPS)
		{
			// S is nearly parallel to this edge
			if (n < 0)
				return false;
			else
				continue;
		}
		const dtReal t = n / d;
		if (d < 0)
		{
			// segment S is entering across this edge
			if (t > tmin)
			{
				tmin = t;
				segMin = j;
				// S enters after leaving polygon
				if (tmin > tmax)
					return false;
			}
		}
		else
		{
			// segment S is leaving across this edge
			if (t < tmax)
			{
				tmax = t;
				segMax = j;
				// S leaves before entering polygon
				if (tmax < tmin)
					return false;
			}
		}
	}

	return true;
}

static void FreePool(ArgusDtNodePool* nodePool)
{
	nodePool->~ArgusDtNodePool();
	dtFree(nodePool, DT_ALLOC_PERM_NAVQUERY);
}

dtStatus ArgusDetourQuery::FindWallsOverlappingShape(const dtNavMesh* detourMesh, const int maxNodes, dtPolyRef startRef,
	const dtReal* verts, const int nverts, const dtQueryFilter* filter,
	dtPolyRef* neiRefs, int* neiCount, const int maxNei,
	dtReal* resultWalls, dtPolyRef* resultRefs, int* resultCount, const int maxResult)
{
	ARGUS_TRACE(ArgusDetourQuery::FindWallsOverlappingShape);
	ARGUS_MEMORY_TRACE(ArgusDetourQuery);

	ArgusDtNodePool* nodePool = new (dtAlloc(sizeof(ArgusDtNodePool), DT_ALLOC_PERM_NAVQUERY)) ArgusDtNodePool(maxNodes, dtNextPow2(maxNodes / 4));
	if (!nodePool && maxNodes > 0)
	{
		return DT_FAILURE | DT_OUT_OF_MEMORY;
	}

	*resultCount = 0;
	*neiCount = 0;

	// Validate input
	if (!startRef || !detourMesh->isValidPolyRef(startRef))
	{
		FreePool(nodePool);
		return DT_FAILURE | DT_INVALID_PARAM;
	}

	nodePool->clear();

	static const int MAX_STACK = 48;
	dtNode* stack[MAX_STACK];
	int nstack = 0;

	dtNode* startNode = nodePool->getNode(startRef);
	startNode->pidx = 0;
	startNode->id = startRef;
	startNode->flags = DT_NODE_CLOSED;
	stack[nstack++] = startNode;

	dtStatus status = DT_SUCCESS;

	int n = 0;
	if (n < maxNei)
	{
		neiRefs[n] = startNode->id;
		++n;
	}
	else
	{
		status |= DT_BUFFER_TOO_SMALL;
	}

	while (nstack)
	{
		// Pop front.
		dtNode* curNode = stack[0];
		for (int stackIndex = 0; stackIndex < nstack - 1; ++stackIndex)
			stack[stackIndex] = stack[stackIndex + 1];
		nstack--;

		// Get poly and tile.
		// The API input has been checked already, skip checking internal data.
		const dtPolyRef curRef = curNode->id;
		const dtMeshTile* curTile = 0;
		const dtPoly* curPoly = 0;
		detourMesh->getTileAndPolyByRefUnsafe(curRef, &curTile, &curPoly);

		unsigned int i = curPoly->firstLink;
		while (i != DT_NULL_LINK)
		{
			const dtLink& link = detourMesh->getLink(curTile, i);
			i = link.next;

			dtPolyRef neighbourRef = link.ref;
			// Skip invalid neighbors.
			if (!neighbourRef)
			{
				// store wall segment
				storeWallSegment(detourMesh, curTile, curPoly, link.edge, curRef, 0,
					resultWalls, resultRefs, resultCount, maxResult);
				continue;
			}

			// Skip if cannot alloca more nodes.
			dtNode* neighbourNode = nodePool->getNode(neighbourRef);
			if (!neighbourNode)
				continue;
			// Skip visited.
			if (neighbourNode->flags & DT_NODE_CLOSED)
				continue;

			// Expand to neighbor
			const dtMeshTile* neighbourTile = 0;
			const dtPoly* neighbourPoly = 0;
			detourMesh->getTileAndPolyByRefUnsafe(neighbourRef, &neighbourTile, &neighbourPoly);

			// Skip off-mesh connections.
			if (neighbourPoly->getType() != DT_POLYTYPE_GROUND)
				continue;

			// Do not advance if the polygon is excluded by the filter.
			if (!filter->passFilter(neighbourRef, neighbourTile, neighbourPoly) || !passLinkFilterByRef(detourMesh, neighbourTile, neighbourRef))
			{
				// store wall segment
				storeWallSegment(detourMesh, curTile, curPoly, link.edge, curRef, neighbourRef,
					resultWalls, resultRefs, resultCount, maxResult);

				continue;
			}

			// Find edge and calculate distance to the edge.
			dtReal va[3], vb[3];
			if (!getPortalPoints(detourMesh, curRef, curPoly, curTile, neighbourRef, neighbourPoly, neighbourTile, va, vb))
				continue;

			// If the poly is not touching the edge to the next polygon, skip the connection it.
			dtReal tmin, tmax;
			int segMin, segMax;
			if (!ArgusDtIntersectSegmentPoly2D(va, vb, verts, nverts, tmin, tmax, segMin, segMax))
				continue;
			if (tmin > 1.0f || tmax < 0.0f)
				continue;

			// Mark node visited, this is done before the overlap test so that
			// we will not visit the poly again if the test fails.
			neighbourNode->flags |= DT_NODE_CLOSED;
			neighbourNode->pidx = nodePool->getNodeIdx(curNode);

			// This poly is fine, store and advance to the poly.
			if (n < maxNei)
			{
				neiRefs[n] = neighbourRef;
				++n;
			}
			else
			{
				status |= DT_BUFFER_TOO_SMALL;
			}

			if (nstack < MAX_STACK)
			{
				stack[nstack++] = neighbourNode;
			}
		}

		// add hard edges of poly
		for (int neighbourIndex = 0; neighbourIndex < curPoly->vertCount; neighbourIndex++)
		{
			bool bStoreEdge = (curPoly->neis[neighbourIndex] == 0);
			if (curPoly->neis[neighbourIndex] & DT_EXT_LINK)
			{
				// check if external edge has valid link
				bool bConnected = false;

				unsigned int linkIdx = curPoly->firstLink;
				while (linkIdx != DT_NULL_LINK)
				{
					const dtLink& link = detourMesh->getLink(curTile, linkIdx);
					linkIdx = link.next;

					if (link.edge == neighbourIndex)
					{
						bConnected = true;
						break;
					}
				}

				bStoreEdge = !bConnected;
			}

			if (bStoreEdge)
			{
				storeWallSegment(detourMesh, curTile, curPoly, neighbourIndex, curRef, 0,
					resultWalls, resultRefs, resultCount, maxResult);
			}
		}
	}

	FreePool(nodePool);
	*neiCount = n;
	return status;
}

ArgusDtNodePool::ArgusDtNodePool(int maxNodes, int hashSize) :
	m_nodes(0),
	m_first(0),
	m_next(0),
	m_maxNodes(maxNodes),
	m_hashSize(hashSize),
	//@UE BEGIN
	m_maxRuntimeNodes(maxNodes),
	//@UE END
	m_nodeCount(0)
{
	dtAssert(dtNextPow2(m_hashSize) == (unsigned int)m_hashSize);
	dtAssert(m_maxNodes > 0);

	m_nodes = (dtNode*)dtAlloc(sizeof(dtNode) * m_maxNodes, DT_ALLOC_PERM_NODE_POOL);
	m_next = (dtNodeIndex*)dtAlloc(sizeof(dtNodeIndex) * m_maxNodes, DT_ALLOC_PERM_NODE_POOL);
	m_first = (dtNodeIndex*)dtAlloc(sizeof(dtNodeIndex) * hashSize, DT_ALLOC_PERM_NODE_POOL);

	dtAssert(m_nodes);
	dtAssert(m_next);
	dtAssert(m_first);

	memset(m_first, 0xff, sizeof(dtNodeIndex) * m_hashSize);
	memset(m_next, 0xff, sizeof(dtNodeIndex) * m_maxNodes);
}

ArgusDtNodePool::~ArgusDtNodePool()
{
	dtFree(m_nodes, DT_ALLOC_PERM_NODE_POOL);
	dtFree(m_next, DT_ALLOC_PERM_NODE_POOL);
	dtFree(m_first, DT_ALLOC_PERM_NODE_POOL);
}

void ArgusDtNodePool::clear()
{
	memset(m_first, 0xff, sizeof(dtNodeIndex) * m_hashSize);
	m_nodeCount = 0;
}

dtNode* ArgusDtNodePool::getNode(dtPolyRef id)
{
	unsigned int bucket = dtHashRef(id) & (m_hashSize - 1);
	dtNodeIndex i = m_first[bucket];
	dtNode* node = 0;
	while (i != DT_NULL_IDX)
	{
		if (m_nodes[i].id == id)
			return &m_nodes[i];
		i = m_next[i];
	}

	//@UE BEGIN
	if (m_nodeCount >= getMaxRuntimeNodes())
		//@UE END
		return 0;

	i = (dtNodeIndex)m_nodeCount;
	m_nodeCount++;

	// Init node
	node = &m_nodes[i];
	node->pidx = 0;
	node->cost = 0;
	node->total = 0;
	node->id = id;
	node->flags = 0;

	m_next[i] = m_first[bucket];
	m_first[bucket] = i;

	return node;
}

dtNode* ArgusDtNodePool::findNode(dtPolyRef id)
{
	unsigned int bucket = dtHashRef(id) & (m_hashSize - 1);
	dtNodeIndex i = m_first[bucket];
	while (i != DT_NULL_IDX)
	{
		if (m_nodes[i].id == id)
			return &m_nodes[i];
		i = m_next[i];
	}
	return 0;
}
