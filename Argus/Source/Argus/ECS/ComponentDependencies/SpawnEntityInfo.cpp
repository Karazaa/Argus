// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpawnEntityInfo.h"
#include "Serialization/Archive.h"

FArchive& operator<<(FArchive& archive, SpawnEntityInfo& spawnEntityInfo)
{
	spawnEntityInfo.m_spawnLocationOverride.Serialize(archive);
	archive << spawnEntityInfo.m_argusActorRecordId;
	archive << spawnEntityInfo.m_spawningAbilityRecordId;
	archive << spawnEntityInfo.m_timeToCastSeconds;
	archive << spawnEntityInfo.m_initiatingEntityId;
	archive << spawnEntityInfo.m_needsConstruction;

	return archive;
}