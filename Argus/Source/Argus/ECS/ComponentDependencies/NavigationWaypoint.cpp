// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationWaypoint.h"
#include "Serialization/Archive.h"

FArchive& operator<<(FArchive& archive, NavigationWaypoint& waypoint)
{
	archive << waypoint.m_location;
	archive << waypoint.m_decalEntityId;

	return archive;
}