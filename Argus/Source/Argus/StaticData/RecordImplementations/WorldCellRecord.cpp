// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RecordDefinitions/WorldCellRecord.h"
#include "ArgusStaticData.h"
#include "RecordDefinitions/HeadlessEntityRecord.h"
#include "RecordDefinitions/ObstaclesRecord.h"

void UWorldCellRecord::OnAsyncLoaded() const
{
	ArgusStaticData::AsyncPreLoadRecord<UObstaclesRecord>(m_obstaclesRecord.GetId());
	for (int32 i = 0; i < m_headlessEntities.Num(); ++i)
	{
		ArgusStaticData::AsyncPreLoadRecord<UHeadlessEntityRecord>(m_headlessEntities[i].GetId());
	}
}