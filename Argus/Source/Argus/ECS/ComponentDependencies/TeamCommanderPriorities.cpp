// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderPriorities.h"
#include "Serialization/Archive.h"

FArchive& operator<<(FArchive& archive, ResourceSourceExtractionData& resourceSourceExtractionData)
{
	archive << resourceSourceExtractionData.m_resourceSourceEntityId;
	archive << resourceSourceExtractionData.m_resourceSinkEntityId;
	archive << resourceSourceExtractionData.m_resourceExtractorEntityId;
	archive << resourceSourceExtractionData.m_resourceSinkConstructorEntityId;
	return archive;
}