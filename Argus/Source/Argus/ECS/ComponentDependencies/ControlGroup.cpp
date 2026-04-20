// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ControlGroup.h"
#include "Serialization/Archive.h"

FArchive& operator<<(FArchive& archive, ControlGroup& controlGroup)
{
	controlGroup.m_entityIds.BulkSerialize(archive);
	return archive;
}