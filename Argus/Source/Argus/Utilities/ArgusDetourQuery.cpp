// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusDetourQuery.h"
#include "Detour/DetourNode.h"
#include "Detour/DetourAssert.h"

dtStatus ArgusDetourQuery::FindWallsOverlappingShape(dtPolyRef startRef, const dtReal* verts, const int nverts,
	const dtQueryFilter* filter,
	dtPolyRef* neiRefs, int* neiCount, const int maxNei,
	dtReal* resultWalls, dtPolyRef* resultRefs, int* resultCount, const int maxResult)
{
	return DT_SUCCESS;
}