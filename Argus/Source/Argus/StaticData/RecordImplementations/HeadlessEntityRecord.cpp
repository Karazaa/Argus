// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RecordDefinitions/HeadlessEntityRecord.h"
#include "ArgusStaticData.h"

void UHeadlessEntityRecord::OnAsyncLoaded() const
{
	m_entityTemplate.AsyncPreLoadAndStorePtr();
}

void UHeadlessEntityRecord::ResetSoftPtrLoadStores()
{
	m_entityTemplate.ResetHardPtr();
}