// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Detour/DetourNavMeshQuery.h"

class ArgusDetourQuery
{
public:
	static dtStatus FindWallsOverlappingShape(dtPolyRef startRef, const dtReal* verts, const int nverts,
		const dtQueryFilter* filter,
		dtPolyRef* neiRefs, int* neiCount, const int maxNei,
		dtReal* resultWalls, dtPolyRef* resultRefs, int* resultCount, const int maxResult);
};