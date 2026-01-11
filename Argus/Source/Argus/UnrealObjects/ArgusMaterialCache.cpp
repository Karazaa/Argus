// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusMaterialCache.h"

FArgusMaterialCache* FArgusMaterialCache::s_materialCacheInstance = nullptr;

FArgusMaterialCache::FArgusMaterialCache()
{
	s_materialCacheInstance = this;
}